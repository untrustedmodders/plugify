#pragma once

namespace wizard {
    struct PluginReferenceDescriptor {
        std::string name;
        bool optional{ false };
        std::string description;
        std::string downloadURL;
        std::vector<std::string> supportedPlatforms;
        std::optional<int32_t> requestedVersion;

        explicit PluginReferenceDescriptor(std::string pluginName = "");

        bool IsSupportsPlatform(const std::string& platform) const;

        bool Read(const rapidjson::Value& object);
    };
}