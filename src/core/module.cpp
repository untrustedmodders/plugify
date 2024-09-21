#include "module.h"
#include "plugin.h"
#include <plugify/module.h>
#include <plugify/package.h>
#include <plugify/plugify_provider.h>
#include <plugify/mem_protector.h>
#include <utils/library_search_dirs.h>

#undef FindResource

using namespace plugify;

Module::Module(UniqueId id, const LocalPackage& package) : _id{id}, _name{package.name}, _lang{package.type}, _descriptor{std::static_pointer_cast<LanguageModuleDescriptor>(package.descriptor)} {
	PL_ASSERT(package.type != "plugin", "Invalid package type for module ctor");
	PL_ASSERT(package.path.has_parent_path(), "Package path doesn't contain parent path");
	// Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.
	_baseDir = package.path.parent_path();
	_filePath = _baseDir / "bin" / std::format(PLUGIFY_MODULE_PREFIX "{}" PLUGIFY_MODULE_SUFFIX, package.name);
}

Module::~Module() {
	Terminate();
}

bool Module::Initialize(std::weak_ptr<IPlugifyProvider> provider) {
	PL_ASSERT(GetState() != ModuleState::Loaded, "Module already was initialized");

	std::error_code ec;
	if (!fs::exists(_filePath, ec) || !fs::is_regular_file(_filePath, ec) || fs::is_symlink(_filePath, ec)) {
		SetError(std::format("Module binary '{}' not exist!.", _filePath.string()));
		return false;
	}

	auto plugifyProvider = provider.lock();

	const fs::path& baseDir = plugifyProvider->GetBaseDir();

	if (const auto& resourceDirectoriesSettings = GetDescriptor().resourceDirectories) {
		for (const auto& rawPath : *resourceDirectoriesSettings) {
			fs::path resourceDirectory = fs::absolute(_baseDir / rawPath, ec);
			for (const auto& entry : fs::recursive_directory_iterator(resourceDirectory, ec)) {
				if (entry.is_regular_file(ec) && !entry.is_symlink(ec)) {
					fs::path relPath = fs::relative(entry.path(), _baseDir, ec);
					fs::path absPath = baseDir / relPath;

					if (!fs::exists(absPath, ec) || !fs::is_regular_file(absPath, ec) || fs::is_symlink(absPath, ec)) {
						absPath = entry.path();
					}

					_resources.emplace(std::move(relPath), std::move(absPath));
				}
			}
		}
	}

	std::vector<fs::path> libraryDirectories;
	if (const auto& libraryDirectoriesSettings = GetDescriptor().libraryDirectories) {
		for (const auto& rawPath : *libraryDirectoriesSettings) {
			fs::path libraryDirectory = fs::absolute(_baseDir / rawPath, ec);
			if (!fs::exists(libraryDirectory, ec) || !fs::is_directory(libraryDirectory, ec)) {
				SetError(std::format("Library directory '{}' not exists", libraryDirectory.string()));
				return false;
			}
			libraryDirectory.make_preferred();
			libraryDirectories.emplace_back(std::move(libraryDirectory));
		}
	}

	auto scopedDirs = LibrarySearchDirs::Add(libraryDirectories);

	LoadFlag flags = LoadFlag::Lazy | LoadFlag::Global | /**/ LoadFlag::SearchUserDirs | LoadFlag::SearchSystem32 | LoadFlag::SearchDllLoadDir;
	if (plugifyProvider->IsPreferOwnSymbols()) {
		flags |= LoadFlag::Deepbind;
	}

	auto assembly = std::make_unique<Assembly>(fs::absolute(_filePath, ec), flags);
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

	ILanguageModule* languageModulePtr = GetLanguageModuleFunc();
	if (!languageModulePtr) {
		SetError(std::format("Function 'GetLanguageModule' inside '{}' library. Not returned valid address of 'ILanguageModule' implementation!",  _filePath.string()));
		Terminate();
		return false;
	}

#if PLUGIFY_PLATFORM_WINDOWS
	constexpr bool plugifyBuildType = PLUGIFY_IS_DEBUG;
	bool moduleBuildType = languageModulePtr->IsDebugBuild();
	if (moduleBuildType != plugifyBuildType) {
		SetError(std::format("Mismatch between plugify ({}) build type and module ({}) build type.", (plugifyBuildType ? "debug" : "release"), (moduleBuildType ? "debug" : "release")));
		Terminate();
		return false;
	}
#endif

	InitResult result = languageModulePtr->Initialize(std::move(provider), *this);
	if (auto* data = std::get_if<ErrorData>(&result)) {
		SetError(std::format("Failed to initialize module: '{}' error: '{}' at: '{}'", _name, data->error.data(), _filePath.string()));
		Terminate();
		return false;
	}

	_assembly = std::move(assembly);
	_languageModule = std::ref(*languageModulePtr);
	SetLoaded();
	return true;
}

void Module::Terminate() {
	if (_languageModule.has_value()) {
		GetLanguageModule().Shutdown();
	}
	_languageModule.reset();
	_assembly.reset();
	
	SetUnloaded();
}

bool Module::LoadPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return false;

	const auto& exportedMethods = plugin.GetDescriptor().exportedMethods;

	auto result = GetLanguageModule().OnPluginLoad(plugin);
	if (auto* data =  std::get_if<ErrorData>(&result)) {
		plugin.SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin.GetName(), data->error.data(), plugin.GetBaseDir().string()));
		return false;
	}

	auto& methods = std::get<LoadResultData>(result).methods;

	if (methods.size() != exportedMethods.size()) {
		plugin.SetError(std::format("Mismatch in methods count, expected: {} but provided: {}", exportedMethods.size(), methods.size()));
		return false;
	}

	std::vector<std::string_view> errors;

	for (size_t i = 0; i < methods.size(); ++i) {
		const auto& [method, addr] = methods[i];
		const auto& exportedMethod = exportedMethods[i];

		if (method != MethodRef(exportedMethod) || !addr) {
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
	plugin.SetLoaded();

	//_loadedPlugins.emplace_back(plugin);
	
	return true;
}

void Module::MethodExport(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	GetLanguageModule().OnMethodExport(plugin);
}

void Module::StartPlugin(Plugin& plugin) const  {
	if (_state != ModuleState::Loaded)
		return;

	GetLanguageModule().OnPluginStart(plugin);

	plugin.SetRunning();
}

void Module::EndPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	GetLanguageModule().OnPluginEnd(plugin);

	plugin.SetTerminating();
}

std::optional<fs::path_view> Module::FindResource(const fs::path& path) const {
	auto it = _resources.find(path);
	if (it != _resources.end())
		return std::get<fs::path>(*it).c_str();
	return {};
}

void Module::SetError(std::string error) {
	_error = std::make_unique<std::string>(std::move(error));
	_state = ModuleState::Error;
	PL_LOG_ERROR("Module '{}': {}", _name, *_error);
}