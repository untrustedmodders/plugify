#pragma once

#include <span>
#include <string>

#include "handle.hpp"
#include "method.hpp"
#include "version.hpp"

#include <plugify_export.h>

namespace plugify {
	struct PluginDescriptor;
	class PluginReferenceDescriptorHandle;
	class MethodHandle;

	/**
	 * @class PluginDescriptorHandle
	 * @brief A handle class for the `PluginDescriptor` structure.
	 */
	class PLUGIFY_API PluginDescriptorHandle : public Handle<const PluginDescriptor> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the file version of the plugin.
		 *
		 * @return An integer representing the file version.
		 */
		int32_t GetFileVersion() const noexcept;

		/**
		 * @brief Retrieves the version of the plugin.
		 *
		 * @return A struct representing the plugin version.
		 */
		plg::version GetVersion() const noexcept;

		/**
		 * @brief Retrieves the version name of the plugin.
		 *
		 * @return A string view representing the version name.
		 */
		std::string_view GetVersionName() const noexcept;

		/**
		 * @brief Retrieves the friendly name of the plugin.
		 *
		 * @return A string view representing the friendly name.
		 */
		std::string_view GetFriendlyName() const noexcept;

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
		std::string_view GetCreatedBy() const noexcept;

		/**
		 * @brief Retrieves the URL of the creator of the plugin.
		 *
		 * @return A string view representing the creator's URL.
		 */
		std::string_view GetCreatedByURL() const noexcept;

		/**
		 * @brief Retrieves the documentation URL for the plugin.
		 *
		 * @return A string view representing the documentation URL.
		 */
		std::string_view GetDocsURL() const noexcept;

		/**
		 * @brief Retrieves the download URL for the plugin.
		 *
		 * @return A string view representing the download URL.
		 */
		std::string_view GetDownloadURL() const noexcept;

		/**
		 * @brief Retrieves the update URL for the plugin.
		 *
		 * @return A string view representing the update URL.
		 */
		std::string_view GetUpdateURL() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the plugin.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		std::span<const std::string_view> GetSupportedPlatforms() const noexcept;

		/**
		 * @brief Retrieves the resource directories for the plugin.
		 *
		 * @return A span of string views representing the resource directories.
		 */
		std::span<const std::string_view> GetResourceDirectories() const noexcept;

		/**
		 * @brief Retrieves the entry point of the plugin.
		 *
		 * @return A string view representing the plugin's entry point.
		 */
		std::string_view GetEntryPoint() const noexcept;

		/**
		 * @brief Retrieves the language module used by the plugin.
		 *
		 * @return A string view representing the language module.
		 */
		std::string_view GetLanguageModule() const noexcept;

		/**
		 * @brief Retrieves the dependencies of the plugin.
		 *
		 * @return A span of `PluginReferenceDescriptorRef` objects representing the plugin's dependencies.
		 */
		std::span<const PluginReferenceDescriptorHandle> GetDependencies() const noexcept;

		/**
		 * @brief Retrieves the methods exported by the plugin.
		 *
		 * @return A span of `MethodRef` objects representing the exported methods.
		 */
		std::span<const MethodHandle> GetExportedMethods() const noexcept;
	};
} // namespace plugify
