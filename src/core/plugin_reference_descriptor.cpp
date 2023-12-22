#include "plugin_reference_descriptor.h"

using namespace wizard;

PluginReferenceDescriptor::PluginReferenceDescriptor(std::string pluginName) : name{std::move(pluginName)} {
}

bool PluginReferenceDescriptor::IsSupportsPlatform(const std::string& platform) const {
    return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), platform) != supportedPlatforms.end();
}
