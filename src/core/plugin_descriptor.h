#pragma once

#include "plugin_reference_descriptor.h"
#include "language_module_descriptor.h"

namespace wizard {
    struct PluginDescriptor {
        int32_t fileVersion{ 0 };
        int32_t version{ 0 };
        std::string versionName;
        std::string friendlyName;
        std::string description;
        std::string createdBy;
        std::string createdByURL;
        std::string docsURL;
        std::string downloadURL;
        std::string supportURL;
        std::vector<std::string> supportedPlatforms;
        LanguageModuleInfo languageModule;
        std::vector<PluginReferenceDescriptor> dependencies;

        PluginDescriptor() = default;

        bool IsSupportsPlatform(const std::string& platform) const;

	    bool Load(const fs::path& filePath);
        bool Read(const utils::json::Value& object);

        static inline const char* const kFileExtension = ".wplugin";
    };
}