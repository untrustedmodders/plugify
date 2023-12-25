#include "plugin.h"
#include "module.h"

using namespace wizard;

Plugin::Plugin(uint64_t id, std::string name, fs::path filePath, PluginDescriptor descriptor) : _id{id}, _name{std::move(name)}, _filePath{std::move(filePath)}, _descriptor{std::move(descriptor)} {

}

void Plugin::Load() {
    if (_state != PluginState::NotLoaded)
        return;

    _module->GetLanguageModule().OnPluginLoad(*this);

    // TODO: Implement loading

    SetLoaded();
}

void Plugin::Start() {
    if (_state != PluginState::Loaded)
        return;

    _module->GetLanguageModule().OnPluginStart(*this);

    // TODO: Implement plugin start

    SetRunning();
}

void Plugin::End() {
    if (_state != PluginState::Running)
        return;

    _module->GetLanguageModule().OnPluginEnd(*this);

    // TODO: Implement plugin end

    SetTerminating();
}