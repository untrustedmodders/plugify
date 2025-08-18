#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>

#include <plugify/api/constraint.hpp>
#include <plugify/core/dependency.hpp>
#include <plugify/core/conflict.hpp>
#include <plugify/core/method.hpp>

namespace plugify {
	struct PackageManifest {
		PackageId id;
		std::string name;
		ManifestType type{};
		Version version;
		std::string description;
		std::string author;
		std::string website;
		std::string license;
		std::filesystem::path location;

		// Dependencies and conflicts
		std::optional<std::vector<std::string>> platforms;
		std::optional<std::vector<std::unique_ptr<Dependency>>> dependencies;
		std::optional<std::vector<std::unique_ptr<Conflict>>> conflicts;

		// Metadata
		std::unordered_map<std::string, std::string> metadata;
	};

	struct PluginManifest : PackageManifest {
		Dependency language;
		std::string entryPoint;
		std::vector<std::string> capabilities;
		std::optional<std::vector<std::unique_ptr<Method>>> methods;
	};

	struct ModuleManifest : PackageManifest {
		std::string language;
		std::filesystem::path runtimePath;
		std::vector<std::string> supportedVersions;
	};
}