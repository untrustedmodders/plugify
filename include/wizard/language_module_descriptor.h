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

#if WIZARD_BUILD_MAIN_LIB
        LanguageModuleDescriptor() = default;

        bool IsSupportsPlatform(const std::string& platform) const;

        bool Load(const fs::path& filePath);
        bool Read(const utils::json::Value& object);

        static inline const char* const kFileExtension = ".wmodule";
#endif
    };

    struct LanguageModuleInfo {
        std::string name;

#if WIZARD_BUILD_MAIN_LIB
        bool Read(const utils::json::Value& object);
#endif
    };
}