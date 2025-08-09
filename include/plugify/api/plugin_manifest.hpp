#pragma once

#include <span>
#include <string>

#include <plugify/api/handle.hpp>
#include <plugify/api/version.hpp>
#include <plugify/api/method.hpp>

#include <plugify_export.h>

namespace plugify {
	struct PluginManifest;
	class DependencyHandle;
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
		std::string_view GetName() const noexcept;

		/**
		 * @brief Retrieves the version of the plugin.
		 *
		 * @return A struct representing the plugin version.
		 */
		plg::version GetVersion() const noexcept;

		/**
		 * @brief Retrieves the description of the plugin.
		 *
		 * @return A string view representing the plugin description.
		 */
		std::string_view GetDescription() const noexcept;

		/**
		 * @brief Retrieves the name of the creator of the plugin.
		 *
		 * @return A string view representing the name of the creator.
		 */
		std::string_view GetAuthor() const noexcept;

		/**
		 * @brief Retrieves the URL of the creator of the plugin.
		 *
		 * @return A string view representing the creator's URL.
		 */
		std::string_view GetWebsite() const noexcept;

		/**
		 * @brief Retrieves the license for the plugin.
		 *
		 * @return A string view representing the license.
		 */
		std::string_view GetLicense() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the plugin.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		std::span<const std::string_view> GetPlatforms() const noexcept;

		/**
		 * @brief Retrieves the entry point of the plugin.
		 *
		 * @return A string view representing the plugin's entry point.
		 */
		std::string_view GetEntry() const noexcept;

		/**
		 * @brief Retrieves the language used by the plugin.
		 *
		 * @return A string view representing the language.
		 */
		std::string_view GetLanguage() const noexcept;

		/**
		 * @brief Retrieves the dependencies of the plugin.
		 *
		 * @return A span of `DependencyHandle` objects representing the plugin's dependencies.
		 */
		std::span<const DependencyHandle> GetDependencies() const noexcept;

		/**
		 * @brief Retrieves the methods exported by the plugin.
		 *
		 * @return A span of `MethodHandle` objects representing the exported methods.
		 */
		std::span<const MethodHandle> GetMethods() const noexcept;
	};
} // namespace plugify
