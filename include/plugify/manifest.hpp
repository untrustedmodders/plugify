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
	/**
	 * @struct Manifest
	 * @brief The parsed manifest file of an extension.
	 *
	 * One structure covers both kinds of extension, so the plugin-specific and
	 * the module-specific groups below are optional and only one of them is
	 * filled in for a given extension. Which one applies follows from the file
	 * the manifest was read from, see Extension::GetExtensionType.
	 *
	 * A freshly parsed manifest is not yet usable: ResolvePaths() anchors its
	 * relative paths, Resolve() links the type tables, and Validate() checks the
	 * result, in that order.
	 *
	 * @see Extension
	 */
	struct Manifest {
		// Common fields

		std::string name;  ///< Unique identifier of the extension, as other extensions refer to it.
		Version version;  ///< Semantic version, used for dependency resolution and compatibility checks.
		std::string language;  ///< Language identifier; for a plugin, the language module that runs it.
		std::optional<std::string> description;  ///< What the extension does.
		std::optional<std::string> author;  ///< Name or identifier of the author.
		std::optional<std::string> website;  ///< Website, repository or documentation URL.
		std::optional<std::string> license;  ///< License identifier, an SPDX one where possible.

		// Dependencies and conflicts

		std::optional<std::vector<std::string>> platforms;  ///< Supported platforms as 'os_arch'; empty means every platform.
		std::optional<std::vector<Dependency>> dependencies;  ///< Extensions that have to be loaded first.
		std::optional<std::vector<Conflict>> conflicts;  ///< Extensions that cannot be loaded alongside this one.
		std::optional<std::vector<Obsolete>> obsoletes;  ///< Extensions that this one supersedes.

		// Plugin-specific fields (only used when type == Plugin)

		std::optional<std::string> entry;  ///< Entry point file, relative to the location of the extension.
		std::optional<std::vector<Method>> methods;  ///< Methods exposed to other extensions.
		std::optional<std::vector<Class>> classes;  ///< Class wrappers over handle-based methods.

		/**
		 * @brief Shared prototype table.
		 *
		 * A manifest may declare a prototype here once and refer to it by name
		 * from any paramTypes/retType entry instead of repeating the definition.
		 * After Resolve() this also holds every definition that was written
		 * inline, so each distinct type appears exactly once and consumers can
		 * walk a flat list.
		 */
		std::optional<std::vector<std::shared_ptr<Prototype>>> prototypes;

		/**
		 * @brief Shared enumeration table.
		 *
		 * Works exactly like @ref prototypes: declared once here and referenced
		 * by name, or written inline and hoisted in by Resolve().
		 */
		std::optional<std::vector<std::shared_ptr<Enum>>> enums;

		// Module-specific fields (only used when type == Module)

		std::optional<std::filesystem::path> runtime;  ///< Shared library of the module, without the platform prefix or suffix.
		std::optional<std::vector<std::filesystem::path>> directories;  ///< Extra library search paths, relative to the location of the module.

		/**
		 * @brief Anchor the relative paths of this manifest.
		 * @param base Directory the paths of the manifest are relative to.
		 * @param file Path of the manifest file itself.
		 */
		void ResolvePaths(const std::filesystem::path& base, const std::filesystem::path& file);

		/**
		 * @brief Link the type tables of this manifest.
		 *
		 * Links every by-name prototype/enum reference to its definition and
		 * hoists inline definitions into @ref prototypes / @ref enums.
		 *
		 * @return Nothing on success, an error message if a reference names a definition that does
		 * not exist or two definitions share a name.
		 * @note Must run before Validate() and before anything reads
		 * Property::GetPrototype/GetEnumerate.
		 */
		[[nodiscard]] Result<void> Resolve();

		/**
		 * @brief Check that this manifest describes a usable extension.
		 * @return Nothing if the manifest is valid, an error message describing the first problem
		 * otherwise.
		 */
		[[nodiscard]] Result<void> Validate() const;
	};
}
