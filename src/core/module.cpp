#include "module.h"
#include "plugin.h"
#include "wizard/module.h"


using namespace wizard;

Module::Module(std::string name, fs::path filePath, LanguageModuleDescriptor descriptor) : IModule(*this), _name{std::move(name)}, _filePath{std::move(filePath)}, _descriptor{std::move(descriptor)} {
}

Module::~Module() {
    Terminate();
}

bool Module::Initialize(std::weak_ptr<IWizardProvider> provider) {
    // TODO: assert(IsInitialized());

    if (!fs::exists(_filePath) || !fs::is_regular_file(_filePath)) {
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
    if (ErrorData* data = std::get_if<ErrorData>(&result)) {
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
}

void Module::LoadPlugin(const std::shared_ptr<Plugin>& plugin) {
    if (_state != ModuleState::Loaded)
        return;

    if (!fs::exists(_filePath) || !fs::is_regular_file(_filePath)) {
        plugin->SetError(std::format("Plugin assembly '{}' not exist!.", _filePath.string()));
        return;
    }

    auto result = GetLanguageModule().OnPluginLoad(*plugin);
    if (auto data = std::get_if<ErrorData>(&result)) {
        plugin->SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin->GetName(), data->error, _filePath.string()));
        return;
    }

    if (auto data = std::get_if<LoadResultData>(&result)) {
        plugin->SetMethods(std::move(data->methods));
    }

    plugin->SetLoaded();

    _loadedPlugins.emplace_back(plugin);
}

void Module::MethodExport(const std::shared_ptr<Plugin>& plugin) {
    if (_state != ModuleState::Loaded)
        return;

    GetLanguageModule().OnMethodExport(*plugin);
}

void Module::StartPlugin(const std::shared_ptr<Plugin>& plugin) {
    if (_state != ModuleState::Loaded)
        return;

    GetLanguageModule().OnPluginStart(*plugin);

    plugin->SetRunning();
}

void Module::EndPlugin(const std::shared_ptr<Plugin>& plugin) {
    if (_state != ModuleState::Loaded)
        return;

    GetLanguageModule().OnPluginEnd(*plugin);

    plugin->SetTerminating();
}

void Module::SetError(std::string error) {
    _error = std::move(error);
    _state = ModuleState::Error;
    WZ_LOG_ERROR("Module '{}': {}", _name, _error);
}