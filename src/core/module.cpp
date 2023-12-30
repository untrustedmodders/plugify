#include "module.h"
#include "plugin.h"
#include "wizard/module.h"


using namespace wizard;

Module::Module(fs::path filePath, LanguageModuleDescriptor descriptor) : IModule(*this), _filePath{std::move(filePath)}, _descriptor{std::move(descriptor)} {
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

    _library = std::make_unique<Library>(_filePath);
    if (!_library) {
        SetError(std::format("Failed to load library: '{}' at: '{}'", _name, _filePath.string()));
        return false;
    }

    void* GetLanguageModulePtr = _library->GetFunction("GetLanguageModule");
    if (!GetLanguageModulePtr) {
        SetError(std::format("Function 'GetLanguageModule' not exist inside '{}' library", _filePath.string()));
        Terminate();
        return false;
    }

    using GetLanguageModuleFuncT = ILanguageModule*(*)();
    auto GetLanguageModuleFunc = reinterpret_cast<GetLanguageModuleFuncT>(GetLanguageModulePtr);
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
    if (!fs::exists(_filePath) || !fs::is_regular_file(_filePath)) {
        plugin->SetError(std::format("Plugin assembly '{}' not exist!.", _filePath.string()));
        return;
    }

    auto result = GetLanguageModule().OnPluginLoad(*plugin);
    if (ErrorData* data = std::get_if<ErrorData>(&result)) {
        plugin->SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin->GetName(), data->error, _filePath.string()));
        return;
    }

    plugin->SetLoaded();

    _loadedPlugins.emplace_back(plugin);
}

void Module::StartPlugin(const std::shared_ptr<Plugin>& plugin) {
    GetLanguageModule().OnPluginStart(*plugin);

    plugin->SetRunning();
}

void Module::EndPlugin(const std::shared_ptr<Plugin>& plugin) {
    GetLanguageModule().OnPluginEnd(*plugin);

    plugin->SetTerminating();
}

void Module::SetError(std::string error) {
    _error = std::move(error);
    _state = ModuleState::Error;
    WZ_LOG_ERROR("Module: {}", _error);
}