#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "plugify/core/constraint.hpp"
#include "handle.hpp"

#include "plugify_export.h"

namespace plugify {
	struct ModuleManifest;
	class DependencyHandle;
	class ConflictHandle;

	/**
	 * @class ModuleManifestHandle
	 * @brief A handle class for the `ModuleManifest` structure.
	 */
	class PLUGIFY_API ModuleManifestHandle : public Handle<const ModuleManifest> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the name of the language module.
		 *
		 * @return A string view representing the name.
		 */
		const std::string& GetName() const noexcept;

		/**
		 * @brief Retrieves the version of the language module.
		 *
		 * @return A struct representing the version.
		 */
		const Version& GetVersion() const noexcept;

		/**
		 * @brief Retrieves the description of the language module.
		 *
		 * @return A string view representing the description.
		 */
		const std::string& GetDescription() const noexcept;

		/**
		 * @brief Retrieves the name of the creator of the language module.
		 *
		 * @return A string view representing the name of the creator.
		 */
		const std::string& GetAuthor() const noexcept;

		/**
		 * @brief Retrieves the URL of the creator of the language module.
		 *
		 * @return A string view representing the creator's URL.
		 */
		const std::string& GetWebsite() const noexcept;

		/**
		 * @brief Retrieves the license for the language module.
		 *
		 * @return A string view representing the license.
		 */
		const std::string& GetLicense() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the language module.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		std::span<const std::string> GetPlatforms() const noexcept;

		/**
		 * @brief Retrieves the dependencies of the language module.
		 *
		 * @return A span of `DependencyHandle` objects representing the language module's dependencies.
		 */
		std::span<const DependencyHandle> GetDependencies() const noexcept;

		/**
		 * @brief Retrieves the conflicts of the language module.
		 *
		 * @return A span of `ConflictHandle` objects representing the language module's conflicts.
		 */
		std::span<const ConflictHandle> GetConflicts() const noexcept;

		/**
		 * @brief Retrieves the library directories for the language module.
		 *
		 * @return A span of string views representing the library directories.
		 */
		std::span<const std::string> GetDirectories() const noexcept;

		/**
		 * @brief Retrieves the programming language of the language module.
		 *
		 * @return A string view representing the language used by the module.
		 */
		const std::string& GetLanguage() const noexcept;

		/**
		 * @brief Checks if the language module is forced to load.
		 *
		 * @return `true` if the module is forced to load, otherwise `false`.
		 */
		bool IsForceLoad() const noexcept;
	};
} // namespace plugify
