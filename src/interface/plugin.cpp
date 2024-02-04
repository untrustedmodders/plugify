#include <plugify/plugin.h>
#include <core/plugin.h>

using namespace plugify;

IPlugin::IPlugin(Plugin& impl) : _impl{impl} {
}

IPlugin::~IPlugin() = default;

UniqueId IPlugin::GetId() const {
    return _impl.GetId();
}

const std::string& IPlugin::GetName() const {
    return _impl.GetName();
}

const std::string& IPlugin::GetFriendlyName() const {
    return _impl.GetFriendlyName();
}

const fs::path& IPlugin::GetBaseDir() const {
    return _impl.GetBaseDir();
}

const PluginDescriptor& IPlugin::GetDescriptor() const {
    return _impl.GetDescriptor();
}

PluginState IPlugin::GetState() const {
    return _impl.GetState();
}

const std::string& IPlugin::GetError() const {
    return _impl.GetError();
}

const std::vector<MethodData>& IPlugin::GetMethods() const {
    return _impl.GetMethods();
}