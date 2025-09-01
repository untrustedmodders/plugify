#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "plugify/conflict.hpp"
#include "plugify/dependency.hpp"
#include "plugify/method.hpp"

namespace plugify {
    // Unified Manifest (combines all fields)
    struct Manifest {
        // Common fields
        std::string name;
        Version version;
        std::string language;
        std::optional<std::string> description;
        std::optional<std::string> author;
        std::optional<std::string> website;
        std::optional<std::string> license;

        // Dependencies and conflicts
        std::optional<std::vector<std::string>> platforms;
        std::optional<std::vector<Dependency>> dependencies;
        std::optional<std::vector<Conflict>> conflicts;
        std::optional<std::vector<Obsolete>> obsoletes;

        // Plugin-specific fields (only used when type == Plugin)
        std::optional<std::string> entry;
        std::optional<std::vector<Method>> methods;

        // Module-specific fields (only used when type == Module)
        std::optional<std::filesystem::path> runtime;
        std::optional<std::vector<std::filesystem::path>> directories;
    };

}