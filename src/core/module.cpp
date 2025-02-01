#include "module.hpp"
#include "plugin.hpp"
#include <plugify/mem_protector.hpp>
#include <plugify/module.hpp>
#include <plugify/package.hpp>
#include <plugify/plugify_provider.hpp>

#undef FindResource

using namespace plugify;

Module::Module(UniqueId id, const LocalPackage& package) : _id{id}, _name{package.name}, _lang{package.type}, _descriptor{std::static_pointer_cast<LanguageModuleDescriptor>(package.descriptor)} {
	PL_ASSERT(package.type != "plugin", "Invalid package type for module ctor");
	PL_ASSERT(package.path.has_parent_path(), "Package path doesn't contain parent path");
	// Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.
	_baseDir = package.path.parent_path();
	_filePath = _baseDir / "bin" / std::format(PLUGIFY_LIBRARY_PREFIX "{}" PLUGIFY_LIBRARY_SUFFIX, package.name);
}

Module::~Module() {
	Terminate();
}

bool Module::Initialize(const std::shared_ptr<IPlugifyProvider>& provider) {
	PL_ASSERT(GetState() != ModuleState::Loaded, "Module already was initialized");

	std::error_code ec;
	auto is_regular_file = [&](const fs::path& path) {
		return fs::exists(path, ec) && fs::is_regular_file(path, ec);
	};

	if (!is_regular_file(_filePath)) {
		SetError(std::format("Module binary '{}' not exist!.", _filePath.string()));
		return false;
	}

	fs::path baseDir = provider->GetBaseDir();

	if (const auto& resourceDirectoriesSettings = _descriptor->resourceDirectories) {
		for (const auto& rawPath : *resourceDirectoriesSettings) {
			fs::path resourceDirectory = fs::absolute(_baseDir / rawPath, ec);
			for (const auto& entry : fs::recursive_directory_iterator(resourceDirectory, ec)) {
				if (entry.is_regular_file(ec)) {
					fs::path relPath = fs::relative(entry.path(), _baseDir, ec);
					fs::path absPath = baseDir / relPath;

					if (!is_regular_file(absPath)) {
						absPath = entry.path();
					}

					_resources.try_emplace(std::move(relPath), std::move(absPath));
				}
			}
		}
	}

	auto is_directory = [&](const fs::path& path) {
		return fs::exists(path, ec) && fs::is_directory(path, ec);
	};

	std::vector<fs::path> libraryDirectories;
	if (const auto& libraryDirectoriesSettings = _descriptor->libraryDirectories) {
		for (const auto& rawPath : *libraryDirectoriesSettings) {
			fs::path libraryDirectory = fs::absolute(_baseDir / rawPath, ec);
			if (!is_directory(libraryDirectory)) {
				SetError(std::format("Library directory '{}' not exists", libraryDirectory.string()));
				return false;
			}
			if (fs::is_symlink(libraryDirectory, ec)) {
				libraryDirectory = fs::read_symlink(libraryDirectory, ec);
			}
			libraryDirectory.make_preferred();
			libraryDirectories.emplace_back(std::move(libraryDirectory));
		}
	}

	LoadFlag flags = LoadFlag::Lazy | LoadFlag::Global | /**/ LoadFlag::SearchUserDirs | LoadFlag::SearchSystem32 | LoadFlag::SearchDllLoadDir;
	if (provider->IsPreferOwnSymbols()) {
		flags |= LoadFlag::Deepbind;
	}

	auto assembly = std::make_unique<Assembly>(fs::absolute(_filePath, ec), flags, libraryDirectories);
	if (!assembly->IsValid()) {
		SetError(std::format("Failed to load library: '{}' at: '{}' - {}", _name, _filePath.string(), assembly->GetError()));
		return false;
	}

	auto GetLanguageModuleFunc = assembly->GetFunctionByName("GetLanguageModule").RCast<ILanguageModule*(*)()>();
	if (!GetLanguageModuleFunc) {
		SetError(std::format("Function 'GetLanguageModule' not exist inside '{}' library", _filePath.string()));
		Terminate();
		return false;
	}

	ILanguageModule* languageModule = GetLanguageModuleFunc();
	if (!languageModule) {
		SetError(std::format("Function 'GetLanguageModule' inside '{}' library. Not returned valid address of 'ILanguageModule' implementation!",  _filePath.string()));
		Terminate();
		return false;
	}

#if PLUGIFY_PLATFORM_WINDOWS
	constexpr bool plugifyBuildType = PLUGIFY_IS_DEBUG;
	bool moduleBuildType = languageModule->IsDebugBuild();
	if (moduleBuildType != plugifyBuildType) {
		SetError(std::format("Mismatch between plugify ({}) build type and module ({}) build type.", (plugifyBuildType ? "debug" : "release"), (moduleBuildType ? "debug" : "release")));
		Terminate();
		return false;
	}
#endif // PLUGIFY_PLATFORM_WINDOWS

	InitResult result = languageModule->Initialize(provider, *this);
	if (auto* data = std::get_if<ErrorData>(&result)) {
		SetError(std::format("Failed to initialize module: '{}' error: '{}' at: '{}'", _name, data->error.data(), _filePath.string()));
		Terminate();
		return false;
	}

	_assembly = std::move(assembly);
	_languageModule = languageModule;
	_table = std::get<InitResultData>(result).table;

	SetLoaded();
	return true;
}

void Module::Terminate() {
	if (_languageModule) {
		_languageModule->Shutdown();
		_languageModule = nullptr;
	}
	_assembly.reset();
	
	SetUnloaded();
}

void Module::Update(DateTime dt) {
	if (_languageModule && _table.hasUpdate) {
		_languageModule->OnUpdate(dt);
	}
}

bool Module::LoadPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return false;

	auto result = _languageModule->OnPluginLoad(plugin);
	if (auto* data =  std::get_if<ErrorData>(&result)) {
		plugin.SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin.GetName(), data->error.data(), plugin.GetBaseDir().string()));
		return false;
	}

	auto& [methods, data, table] = std::get<LoadResultData>(result);

	if (const auto& exportedMethods = plugin.GetDescriptor().exportedMethods) {
		if (methods.size() != exportedMethods->size()) {
			plugin.SetError(std::format("Mismatch in methods count, expected: {} but provided: {}", exportedMethods->size(), methods.size()));
			return false;
		}

		std::vector<std::string_view> errors;

		for (size_t i = 0; i < methods.size(); ++i) {
			const auto& [method, addr] = methods[i];
			const auto& exportedMethod = (*exportedMethods)[i];

			if (method != exportedMethod || !addr) {
				errors.emplace_back(exportedMethod.name);
			}
		}

		if (!errors.empty()) {
			std::string error(errors[0]);
			for (auto it = std::next(errors.begin()); it != errors.end(); ++it) {
				std::format_to(std::back_inserter(error), ", {}", *it);
			}
			plugin.SetError(std::format("Found invalid {} method(s)", error));
			return false;
		}

		plugin.SetMethods(std::move(methods));
	}

	plugin.SetTable(table);
	plugin.SetData(data);

	plugin.SetLoaded();

	//_loadedPlugins.emplace_back(plugin);
	
	return true;
}

void Module::MethodExport(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasExport()) {
		_languageModule->OnMethodExport(plugin);
	}
}

void Module::StartPlugin(Plugin& plugin) const  {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasStart()) {
		_languageModule->OnPluginStart(plugin);
	}

	plugin.SetRunning();
}

void Module::UpdatePlugin(Plugin& plugin, DateTime dt) const  {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasUpdate()) {
		_languageModule->OnPluginUpdate(plugin, dt);
	}
}

void Module::EndPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasEnd()) {
		_languageModule->OnPluginEnd(plugin);
	}

	plugin.SetTerminating();
}

std::optional<fs::path_view> Module::FindResource(const fs::path& path) const {
	auto it = _resources.find(path);
	if (it != _resources.end())
		return std::get<fs::path>(*it).native();

	return std::nullopt;
}

void Module::SetError(std::string error) {
	_error = std::move(error);
	_state = ModuleState::Error;
	PL_LOG_ERROR("Module '{}': {}", _name, _error);
}
