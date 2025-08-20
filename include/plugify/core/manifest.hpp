#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>

#include "conflict.hpp"
#include "constraint.hpp"
#include "plugify/core/dependency.hpp"
#include "plugify/core/method.hpp"

namespace plugify {
	struct PackageManifest {
		PackageId id; // name@semver
		std::string name;
		PackageType type{}; // LanguageModule or Plugin
		Version version;
		std::string description;
		std::string author;
		std::string website;
		std::string license;
		std::filesystem::path location; //

		// Dependencies and conflicts
		std::optional<std::vector<std::string>> platforms;
		std::optional<std::vector<Dependency>> dependencies;
		std::optional<std::vector<Conflict>> conflicts;

		// Metadata
		//std::optional<std::unordered_map<std::string, std::string>> metadata;
	};

	struct PluginManifest : PackageManifest {
		Dependency language;
		std::string entry;
		std::optional<std::vector<std::string>> capabilities;
		std::optional<std::vector<Method>> methods;
	};

	struct ModuleManifest : PackageManifest {
		std::string language;
		std::optional<std::filesystem::path> runtime;
		std::optional<std::vector<std::string>> directories;
		std::optional<bool> forceLoad;
	};
}