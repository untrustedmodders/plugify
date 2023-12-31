#include <wizard/plugin.h>
#include <core/plugin.h>

using namespace wizard;

IPlugin::IPlugin(Plugin& impl) : _impl{impl} {
}

IPlugin::~IPlugin() = default;

uint64_t IPlugin::GetId() const {
    return _impl.GetId();
}

const std::string& IPlugin::GetName() const {
    return _impl.GetName();
}

const std::string& IPlugin::GetFriendlyName() const {
    return _impl.GetFriendlyName();
}

const std::filesystem::path& IPlugin::GetFilePath() const {
    return _impl.GetFilePath();
}

std::filesystem::path IPlugin::GetBaseDir() const {
    return _impl.GetBaseDir();
}

std::filesystem::path IPlugin::GetContentDir() const {
    return _impl.GetContentDir();
}

std::filesystem::path IPlugin::GetMountedAssetPath() const {
    return _impl.GetMountedAssetPath();
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