#include "plugin.h"
#include "module.h"

using namespace wizard;

Plugin::Plugin(uint64_t id, std::string name, fs::path filePath, PluginDescriptor descriptor) : _id{id}, _name{std::move(name)}, _filePath{std::move(filePath)}, _descriptor{std::move(descriptor)} {

}

void Plugin::SetError(std::string error) {
    _error = std::move(error);
    _state = PluginState::Error;
    WIZARD_LOG(_error, ErrorLevel::ERROR);
}
