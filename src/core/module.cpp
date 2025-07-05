#include "module.hpp"
#include "plugin.hpp"
#include <plugify/mem_protector.hpp>
#include <plugify/module.hpp>
#include <plugify/package.hpp>
#include <plugify/plugify_provider.hpp>

#undef FindResource

using namespace plugify;

Module::Module(UniqueId id, const LocalPackage& package)
	: _id{id}
	, _name{package.name}
	, _lang{package.type}
	, _descriptor{std::static_pointer_cast<LanguageModuleDescriptor>(package.descriptor)} {
	PL_ASSERT(package.type != PackageType::Plugin && "Invalid package type for module ctor");
	PL_ASSERT(package.path.has_parent_path() && "Package path doesn't contain parent path");
	// Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.
	_baseDir = package.path.parent_path();
	_filePath = _baseDir / "bin" / std::format(PLUGIFY_LIBRARY_PREFIX "{}" PLUGIFY_LIBRARY_SUFFIX, package.name);
}

Module::Module(Module&& module) noexcept {
	*this = std::move(module);
}

bool Module::Initialize(const std::shared_ptr<IPlugifyProvider>& provider) {
	PL_ASSERT(GetState() != ModuleState::Loaded && "Module already was initialized");

	std::error_code ec;

	if (!fs::is_regular_file(_filePath, ec)) {
		SetError(std::format("Module binary '{}' not exist!.", _filePath.string()));
		return false;
	}

	fs::path baseDir = provider->GetBaseDir();

	if (const auto& resourceDirectoriesSettings = _descriptor->resourceDirectories) {
		for (const auto& rawPath : *resourceDirectoriesSettings) {
			fs::path resourceDirectory = _baseDir / rawPath;
			if (!fs::is_directory(resourceDirectory, ec)) {
				SetError(std::format("Resource directory '{}' not exists", resourceDirectory.string()));
				return false;
			}
			for (const auto& entry : fs::recursive_directory_iterator(resourceDirectory, ec)) {
				if (entry.is_regular_file(ec)) {
					fs::path relPath = fs::relative(entry.path(), _baseDir, ec);
					fs::path absPath = baseDir / relPath;

					if (!fs::is_regular_file(absPath, ec)) {
						absPath = entry.path();
					}

					_resources.try_emplace(std::move(relPath), std::move(absPath));
				}
			}
		}
	}

	std::vector<fs::path> libraryDirectories;
	if (const auto& libraryDirectoriesSettings = _descriptor->libraryDirectories) {
		for (const auto& rawPath : *libraryDirectoriesSettings) {
			fs::path libraryDirectory = _baseDir / rawPath;
			if (!fs::is_directory(libraryDirectory, ec)) {
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

	const fs::path moduleBasePath = fs::absolute(_filePath, ec);
	if (ec) {
		SetError(std::format("Failed to get module directory path '{}' - {}", _filePath.string(), ec.message()));
		return false;
	}

	auto assembly = std::make_unique<Assembly>(moduleBasePath, flags, libraryDirectories);
	if (!assembly->IsValid()) {
		SetError(std::format("Failed to load library: '{}' at: '{}' - {}", _name, _filePath.string(), assembly->GetError()));
		return false;
	}

	auto GetLanguageModuleFunc = assembly->GetFunctionByName(kGetLanguageModuleFn).RCast<ILanguageModule*(*)()>();
	if (!GetLanguageModuleFunc) {
		SetError(std::format("Function '{}' not exist inside '{}' library", kGetLanguageModuleFn, _filePath.string()));
		Terminate();
		return false;
	}

	ILanguageModule* languageModule = GetLanguageModuleFunc();
	if (!languageModule) {
		SetError(std::format("Function '{}' inside '{}' library. Not returned valid address of 'ILanguageModule' implementation!", kGetLanguageModuleFn, _filePath.string()));
		Terminate();
		return false;
	}

#if PLUGIFY_PLATFORM_WINDOWS
	constexpr bool plugifyBuildType = PLUGIFY_IS_DEBUG;
	bool moduleBuildType = languageModule->IsDebugBuild();
	if (moduleBuildType != plugifyBuildType) {
		SetError(std::format("Mismatch between plugify ({}) build type and module ({}) build type.", (plugifyBuildType ? "debug" : "release"), (moduleBuildType ? "debug" : "release"))); //-V547
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
			plugin.SetError(std::format("Found invalid {} method(s)", plg::join(errors, ", ")));
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
	_error = std::make_unique<std::string>(std::move(error));
	_state = ModuleState::Error;
	PL_LOG_ERROR("Module '{}': {}", _name, *_error);
}
