#include "module.h"
#include "plugin.h"
#include "wizard/module.h"


using namespace wizard;

Module::Module(uint64_t id, std::string name, std::string lang, fs::path filePath, LanguageModuleDescriptor descriptor) : IModule(*this), _id{id}, _name{std::move(name)}, _lang{std::move(lang)}, _filePath{std::move(filePath)}, _descriptor{std::move(descriptor)} {
	_binaryDir = _filePath.parent_path();
	_baseDir = _binaryDir.parent_path();
}

Module::~Module() {
	Terminate();
}

bool Module::Initialize(std::weak_ptr<IWizardProvider> provider) {
	WZ_ASSERT(GetState() == ModuleState::Loaded, "Module already was initialized");

	std::error_code ec;
	if (!fs::exists(_filePath, ec) || !fs::is_regular_file(_filePath, ec)) {
		SetError(std::format("Module binary '{}' not exist!.", _filePath.string()));
		return false;
	}

	_library = Library::LoadFromPath(_filePath);
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
	if (auto data = std::get_if<ErrorData>(&result)) {
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
	if (auto data = std::get_if<ErrorData>(&result)) {
		plugin.SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin.GetName(), data->error, plugin.GetFilePath().string()));
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
	WZ_LOG_ERROR("Module '{}': {}", _name, _error);
}