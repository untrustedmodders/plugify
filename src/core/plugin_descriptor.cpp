#include "plugin_descriptor.h"

using namespace wizard;

bool PluginDescriptor::IsSupportsPlatform(const std::string& platform) const {
    return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), platform) != supportedPlatforms.end();
}
