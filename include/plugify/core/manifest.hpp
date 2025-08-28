#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "plugify/core/conflict.hpp"
#include "plugify/core/dependency.hpp"
#include "plugify/core/method.hpp"

namespace plugify {
	/*struct PackageManifest {
        using Ptr = std::shared_ptr<PackageManifest>;

		std::string name;
		PackageType type{};
		Version version;
		std::optional<std::string> description;
		std::optional<std::string> author;
		std::optional<std::string> website;
		std::optional<std::string> license;

		std::filesystem::path path;
		std::filesystem::path root;

		// Dependencies and conflicts
		std::optional<std::vector<std::string>> platforms;
		std::optional<std::vector<Dependency>> dependencies;
		std::optional<std::vector<Conflict>> conflicts;
		std::optional<std::vector<Obsolete>> obsoletes;

		// Metadata
		//std::optional<std::unordered_map<std::string, std::string>> metadata;
		std::string language;
	};

	struct PluginManifest final : PackageManifest {
        using Ptr = std::shared_ptr<PluginManifest>;

		std::string entry;
		std::optional<std::vector<Method>> methods;
	};

    struct ModuleManifest final : PackageManifest {
        using Ptr = std::shared_ptr<ModuleManifest>;

		std::optional<std::filesystem::path> runtime;
		std::optional<std::vector<std::filesystem::path>> directories;
	};*/


    // Unified Manifest (combines all fields)
    struct PackageManifest {
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