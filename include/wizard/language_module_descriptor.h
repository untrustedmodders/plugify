#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace wizard {
    struct LanguageModuleDescriptor {
        std::int32_t fileVersion{ 0 };
        std::int32_t version{ 0 };
        std::string versionName;
        std::string friendlyName;
        std::string language;
        std::string description;
        std::string createdBy;
        std::string createdByURL;
        std::string docsURL;
        std::string downloadURL;
        std::string supportURL;
        std::vector<std::string> supportedPlatforms;
        bool forceLoad{ false };
    };

    struct LanguageModuleInfo {
        std::string name;
    };
}