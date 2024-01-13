#include <wizard/plugin.h>
#include <core/plugin.h>

using namespace wizard;

IPlugin::IPlugin(Plugin& impl) : _impl{impl} {
}

IPlugin::~IPlugin() = default;

UniqueId IPlugin::GetId() const {
    return _impl.GetId_();
}

const std::string& IPlugin::GetName() const {
    return _impl.GetName_();
}

const std::string& IPlugin::GetFriendlyName() const {
    return _impl.GetFriendlyName_();
}

const fs::path& IPlugin::GetFilePath() const {
    return _impl.GetFilePath_();
}

const fs::path& IPlugin::GetBaseDir() const {
    return _impl.GetBaseDir_();
}

const fs::path& IPlugin::GetContentDir() const {
    return _impl.GetContentDir_();
}

const PluginDescriptor& IPlugin::GetDescriptor() const {
    return _impl.GetDescriptor_();
}

PluginState IPlugin::GetState() const {
    return _impl.GetState_();
}

const std::string& IPlugin::GetError() const {
    return _impl.GetError_();
}

const std::vector<MethodData>& IPlugin::GetMethods() const {
    return _impl.GetMethods_();
}