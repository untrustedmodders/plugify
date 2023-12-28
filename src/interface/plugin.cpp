#include <wizard/plugin.h>
#include <core/plugin.h>

using namespace wizard;

IPlugin::IPlugin(Plugin& impl) : impl{impl} {
}

IPlugin::~IPlugin() = default;

uint64_t IPlugin::GetId() const {
    return impl.GetId();
}

const std::string& IPlugin::GetName() const {
    return impl.GetName();
}

const std::string& IPlugin::GetFriendlyName() const {
    return impl.GetFriendlyName();
}

const std::filesystem::path& IPlugin::GetFilePath() const {
    return impl.GetFilePath();
}

std::filesystem::path IPlugin::GetBaseDir() const {
    return impl.GetBaseDir();
}

std::filesystem::path IPlugin::GetContentDir() const {
    return impl.GetContentDir();
}

std::filesystem::path IPlugin::GetMountedAssetPath() const {
    return impl.GetMountedAssetPath();
}

const PluginDescriptor& IPlugin::GetDescriptor() const {
    return impl.GetDescriptor();
}
