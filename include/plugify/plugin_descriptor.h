#pragma once

#include <span>
#include <string>
#include <plugify/method.h>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

namespace plugify {
	struct PluginDescriptor;
	class PluginReferenceDescriptorRef;
	class MethodRef;

	/**
	 * @class PluginDescriptorRef
	 * @brief A reference class for the `PluginDescriptor` structure.
	 *
	 * This class holds a reference to a `PluginDescriptor` object, allowing users to retrieve
	 * metadata about a plugin, such as version information, description, creator information,
	 * URLs, supported platforms, dependencies, and exported methods.
	 */
	class PLUGIFY_API PluginDescriptorRef : public Ref<const PluginDescriptor> {
		using Ref::Ref; ///< Inherit constructors from Ref<const PluginDescriptor>.
	public:
		/**
		 * @brief Retrieves the file version of the plugin.
		 *
		 * @return An integer representing the file version.
		 */
		[[nodiscard]] int32_t GetFileVersion() const noexcept;

		/**
		 * @brief Retrieves the version of the plugin.
		 *
		 * @return An integer representing the plugin version.
		 */
		[[nodiscard]] int32_t GetVersion() const noexcept;

		/**
		 * @brief Retrieves the version name of the plugin.
		 *
		 * @return A string view representing the version name.
		 */
		[[nodiscard]] std::string_view GetVersionName() const noexcept;

		/**
		 * @brief Retrieves the friendly name of the plugin.
		 *
		 * @return A string view representing the friendly name.
		 */
		[[nodiscard]] std::string_view GetFriendlyName() const noexcept;

		/**
		 * @brief Retrieves the description of the plugin.
		 *
		 * @return A string view representing the plugin description.
		 */
		[[nodiscard]] std::string_view GetDescription() const noexcept;

		/**
		 * @brief Retrieves the name of the creator of the plugin.
		 *
		 * @return A string view representing the name of the creator.
		 */
		[[nodiscard]] std::string_view GetCreatedBy() const noexcept;

		/**
		 * @brief Retrieves the URL of the creator of the plugin.
		 *
		 * @return A string view representing the creator's URL.
		 */
		[[nodiscard]] std::string_view GetCreatedByURL() const noexcept;

		/**
		 * @brief Retrieves the documentation URL for the plugin.
		 *
		 * @return A string view representing the documentation URL.
		 */
		[[nodiscard]] std::string_view GetDocsURL() const noexcept;

		/**
		 * @brief Retrieves the download URL for the plugin.
		 *
		 * @return A string view representing the download URL.
		 */
		[[nodiscard]] std::string_view GetDownloadURL() const noexcept;

		/**
		 * @brief Retrieves the update URL for the plugin.
		 *
		 * @return A string view representing the update URL.
		 */
		[[nodiscard]] std::string_view GetUpdateURL() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the plugin.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		[[nodiscard]] std::span<std::string_view> GetSupportedPlatforms() const noexcept;

		/**
		 * @brief Retrieves the resource directories for the plugin.
		 *
		 * @return A span of string views representing the resource directories.
		 */
		[[nodiscard]] std::span<std::string_view> GetResourceDirectories() const noexcept;

		/**
		 * @brief Retrieves the entry point of the plugin.
		 *
		 * @return A string view representing the plugin's entry point.
		 */
		[[nodiscard]] std::string_view GetEntryPoint() const noexcept;

		/**
		 * @brief Retrieves the language module used by the plugin.
		 *
		 * @return A string view representing the language module.
		 */
		[[nodiscard]] std::string_view GetLanguageModule() const noexcept;

		/**
		 * @brief Retrieves the dependencies of the plugin.
		 *
		 * @return A span of `PluginReferenceDescriptorRef` objects representing the plugin's dependencies.
		 */
		[[nodiscard]] std::span<const PluginReferenceDescriptorRef> GetDependencies() const noexcept;

		/**
		 * @brief Retrieves the methods exported by the plugin.
		 *
		 * @return A span of `MethodRef` objects representing the exported methods.
		 */
		[[nodiscard]] std::span<const MethodRef> GetExportedMethods() const noexcept;
	};
	static_assert(is_ref_v<PluginDescriptorRef>);
} // namespace plugify
