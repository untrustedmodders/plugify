#pragma once

#include <string>
#include <optional>
#include <filesystem>

#include "plugify/global.h"
#include "plugify/conflict.hpp"
#include "plugify/dependency.hpp"
#include "plugify/enum.hpp"
#include "plugify/method.hpp"
#include "plugify/class.hpp"

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
		std::optional<std::vector<Class>> classes;

		// Shared type tables. A manifest may declare a prototype or an enum here
		// once and refer to it by name from any paramTypes/retType entry instead
		// of repeating the definition. After Resolve() these also hold every
		// definition that was written inline, so each distinct type appears
		// exactly once and consumers can walk a flat list.
		std::optional<std::vector<std::shared_ptr<Method>>> prototypes;
		std::optional<std::vector<std::shared_ptr<Enum>>> enums;

		// Module-specific fields (only used when type == Module)
		std::optional<std::filesystem::path> runtime;
		std::optional<std::vector<std::filesystem::path>> directories;

		void ResolvePaths(const std::filesystem::path& base, const std::filesystem::path& file);

		// Links every by-name prototype/enum reference to its definition and
		// hoists inline definitions into `prototypes`/`enums`. Must run before
		// Validate() and before anything reads Property::GetPrototype/GetEnumerate.
		[[nodiscard]] Result<void> Resolve();

		[[nodiscard]] Result<void> Validate() const;
	};
}
