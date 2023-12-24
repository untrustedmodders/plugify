#pragma once

namespace wizard {
    struct LanguageModuleDescriptor {
        int32_t fileVersion{ 0 };
        int32_t version{ 0 };
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

        LanguageModuleDescriptor() = default;

        bool IsSupportsPlatform(const std::string& platform) const;

        bool Load(const fs::path& filePath);
        bool Read(const utils::json::Value& object);

        static inline const char* const FileExtension = ".wmodule";
    };

    struct LanguageModuleInfo {
        std::string name;

        bool Read(const utils::json::Value& object);
    };
}