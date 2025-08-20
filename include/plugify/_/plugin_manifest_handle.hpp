#pragma once

#include <span>
#include <string>

#include "../core/constraint.hpp"
#include "handle.hpp"
#include "method_handle.hpp"

#include "plugify_export.h"

namespace plugify {
	struct PluginManifest;
	class DependencyHandle;
	class ConflictHandle;
	class MethodHandle;

	/**
	 * @class PluginManifestHandle
	 * @brief A handle class for the `PluginDescriptor` structure.
	 */
	class PLUGIFY_API PluginManifestHandle : public Handle<const PluginManifest> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the name of the plugin.
		 *
		 * @return A string view representing the name.
		 */
		const std::string& GetName() const noexcept;

		/**
		 * @brief Retrieves the version of the plugin.
		 *
		 * @return A struct representing the plugin version.
		 */
		const Version& GetVersion() const noexcept;

		/**
		 * @brief Retrieves the description of the plugin.
		 *
		 * @return A string view representing the plugin description.
		 */
		const std::string& GetDescription() const noexcept;

		/**
		 * @brief Retrieves the name of the creator of the plugin.
		 *
		 * @return A string view representing the name of the creator.
		 */
		const std::string& GetAuthor() const noexcept;

		/**
		 * @brief Retrieves the URL of the creator of the plugin.
		 *
		 * @return A string view representing the creator's URL.
		 */
		const std::string& GetWebsite() const noexcept;

		/**
		 * @brief Retrieves the license for the plugin.
		 *
		 * @return A string view representing the license.
		 */
		const std::string& GetLicense() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the plugin.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		std::span<const std::string> GetPlatforms() const noexcept;

		/**
		 * @brief Retrieves the entry point of the plugin.
		 *
		 * @return A string view representing the plugin's entry point.
		 */
		const std::string& GetEntry() const noexcept;

		/**
		 * @brief Retrieves the language used by the plugin.
		 *
		 * @return A string view representing the language.
		 */
		const std::string& GetLanguage() const noexcept;

		/**
		 * @brief Retrieves the dependencies of the plugin.
		 *
		 * @return A span of `DependencyHandle` objects representing the plugin's dependencies.
		 */
		std::span<const DependencyHandle> GetDependencies() const noexcept;

		/**
		 * @brief Retrieves the conflicts of the plugin.
		 *
		 * @return A span of `ConflictHandle` objects representing the plugin's conflicts.
		 */
		std::span<const ConflictHandle> GetConflicts() const noexcept;

		/**
		 * @brief Retrieves the methods exported by the plugin.
		 *
		 * @return A span of `MethodHandle` objects representing the exported methods.
		 */
		std::span<const MethodHandle> GetMethods() const noexcept;
	};
} // namespace plugify
