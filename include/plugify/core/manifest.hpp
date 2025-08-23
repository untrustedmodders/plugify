#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>

#include "plugify/core/conflict.hpp"
#include "plugify/core/dependency.hpp"
#include "plugify/core/method.hpp"

namespace plugify {
	struct PackageManifest {
		std::string name;
		PackageType type{};
		Version version;
		std::optional<std::string> description;
		std::optional<std::string> author;
		std::optional<std::string> website;
		std::optional<std::string> license;
		std::filesystem::path path;

		// Dependencies and conflicts
		std::optional<std::vector<std::string>> platforms;
		std::optional<std::vector<Dependency>> dependencies;
		std::optional<std::vector<Conflict>> conflicts;
		std::optional<std::vector<Obsolete>> obsoletes;

		// Metadata
		//std::optional<std::unordered_map<std::string, std::string>> metadata;
	};

	struct PluginManifest : PackageManifest {
		std::string language;
		std::string entry;
		//std::optional<std::vector<std::string>> capabilities;
		//std::optional<std::vector<Method>> methods;
	};

	struct ModuleManifest : PackageManifest {
		std::string language;
		//std::optional<std::filesystem::path> runtime;
		//std::optional<std::vector<std::string>> directories;
		//std::optional<bool> forceLoad;
	};

	using ManifestPtr = std::shared_ptr<PackageManifest>;
}