#include "module.h"
#include "plugin.h"
#include <plugify/module.h>
#include <plugify/package.h>
#include <utils/library_search_dirs.h>

using namespace plugify;

Module::Module(UniqueId id, const LocalPackage& package) : IModule(*this), _id{id}, _name{package.name}, _lang{package.type}, _descriptor{std::static_pointer_cast<LanguageModuleDescriptor>(package.descriptor)} {
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
	if (!fs::exists(_filePath, ec) || !fs::is_regular_file(_filePath, ec)) {
		SetError(std::format("Module binary '{}' not exist!.", _filePath.string()));
		return false;
	}

	std::vector<fs::path> libraryDirectories;
	if (const auto& libraryDirectoriesSettings = GetDescriptor().libraryDirectories) {
		for (const std::string& rawPath : *libraryDirectoriesSettings) {
			fs::path libraryDirectory = fs::absolute(_baseDir / rawPath);
			if (!fs::exists(libraryDirectory, ec) || !fs::is_directory(libraryDirectory, ec)) {
				SetError(std::format("Library directory '{}' not exists", libraryDirectory.string()));
				return false;
			}
			libraryDirectory.make_preferred();
			libraryDirectories.emplace_back(std::move(libraryDirectory));
		}
	}

	auto scopedDirs = LibrarySearchDirs::Add(libraryDirectories);

	_library = Library::LoadFromPath(fs::absolute(_filePath));

	if (!_library) {
		SetError(std::format("Failed to load library: '{}' at: '{}' - {}", _name, _filePath.string(), Library::GetError()));
		return false;
	}

	using GetLanguageModuleFuncT = ILanguageModule*(*)();
	auto GetLanguageModuleFunc = _library->GetFunction<GetLanguageModuleFuncT>("GetLanguageModule");
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

	InitResult result = languageModulePtr->Initialize(std::move(provider), *this);
	if (auto* data = std::get_if<ErrorData>(&result)) {
		SetError(std::format("Failed to initialize module: '{}' error: '{}' at: '{}'", _name, data->error, _filePath.string()));
		Terminate();
		return false;
	}

	_languageModule = std::ref(*languageModulePtr);
	SetLoaded();
	return true;
}

void Module::Terminate() {
	if (_languageModule.has_value()) {
		GetLanguageModule().Shutdown();
	}
	_languageModule.reset();
	_library.reset();
	SetUnloaded();
}

void Module::LoadPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	auto result = GetLanguageModule().OnPluginLoad(plugin);
	if (auto* data = std::get_if<ErrorData>(&result)) {
		plugin.SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin.GetName(), data->error, plugin.GetBaseDir().string()));
		return;
	}

	plugin.SetMethods(std::move(std::get<LoadResultData>(result).methods));
	plugin.SetLoaded();

	//_loadedPlugins.emplace_back(plugin);
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

void Module::SetError(std::string error) {
	_error = std::move(error);
	_state = ModuleState::Error;
	PL_LOG_ERROR("Module '{}': {}", _name, _error);
}