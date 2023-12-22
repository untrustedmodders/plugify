#pragma once

namespace wizard {
    /**
     * A plugin reference descriptor that includes the necessary information to download and activate a plugin on a specific platform.
     */
    struct PluginReferenceDescriptor {
        /** Specifies the name of the plugin. */
        std::string name;

        /** Indicates whether this plugin is optional, and the core should quietly ignore its absence. */
        bool optional{ false };

        /** Provides a description of the plugin for users who do not have it installed. */
        std::string description;

        /** Contains the download URL for this plugin. This URL will be used for automatic plugin downloads, redirecting to the site if a user hasn't installed it. */
        std::string downloadURL;

        /** Lists the supported platforms for this plugin, supplementing the user's allowed/denied platforms based on the plugin descriptor. */
        std::vector<std::string> supportedPlatforms;

        /** If set, specifies a particular version of the plugin that this reference corresponds to. */
        int32_t requestedVersion{ 0 };

        /** Constructor */
        explicit PluginReferenceDescriptor(std::string pluginName = "");

        /** Determines whether the referenced plugin is supported for the given platform. */
        bool IsSupportsPlatform(const std::string& platform) const;
    };
}