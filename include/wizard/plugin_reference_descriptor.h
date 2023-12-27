#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace wizard {
    struct PluginReferenceDescriptor {
        std::string name;
        bool optional{ false };
        std::string description;
        std::string downloadURL;
        std::vector<std::string> supportedPlatforms;
        std::optional<std::int32_t> requestedVersion;
    };
}