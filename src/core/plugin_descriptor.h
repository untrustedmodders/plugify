#pragma once

#include "plugin_reference_descriptor.h"
#include "language_module_descriptor.h"

namespace wizard {
    /**
     * Descriptor for plugins. Contains all the information present in an wplugin file.
     */
    struct PluginDescriptor
    {
        /** Descriptor version number */
        int32_t fileVersion{ 0 };

        /** Version number for the plugin. The version number must increment with each version, allowing the system to determine version relationships.
            This version number is not displayed; use the VersionName for that purpose. */
        int32_t version{ 0 };

        /** Name of the version for this plugin. This is the public-facing part of the version number and should be updated with version increments. */
        std::string versionName;

        /** Friendly name of the plugin */
        std::string friendlyName;

        /** Description of the plugin */
        std::string description;

        /** Creator of this plugin (optional) */
        std::string createdBy;

        /** Hyperlink URL for the company or individual who created this plugin (optional) */
        std::string createdByURL;

        /** Documentation URL */
        std::string docsURL;

        /** Download URL for this plugin. Used for automatic downloads and redirection if the user hasn't installed it. */
        std::string downloadURL;

        /** Support URL/email for this plugin */
        std::string supportURL;

        /** Controls platforms that can use this plugin and stages the .wplugin file and content files accordingly. */
        std::vector<std::string> supportedPlatforms;

        /** Language module associated with this plugin */
        LanguageModuleInfo languageModule;

        /**  An optional list of the other plugins that **must be** installed and enabled for this plugin to function properly. */
        std::vector<PluginReferenceDescriptor> dependencies;

        /** Constructor */
        PluginDescriptor() = default;

        /** Determines whether the plugin supports the given platform */
        bool IsSupportsPlatform(const std::string& platform) const;
    };
}