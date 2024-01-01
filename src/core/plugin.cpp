#include "plugin.h"
#include "module.h"
#include "wizard/plugin.h"

using namespace wizard;

Plugin::Plugin(uint64_t id, std::string name, fs::path filePath, PluginDescriptor descriptor) : IPlugin(*this), _id{id}, _name{std::move(name)}, _filePath{std::move(filePath)}, _descriptor{std::move(descriptor)} {

}

void Plugin::SetError(std::string error) {
    _error = std::move(error);
    _state = PluginState::Error;
    WZ_LOG_ERROR("Plugin '{}': {}", _name, _error);
}