#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

namespace plugify {
	struct LanguageModuleDescriptor;

	/**
	 * @class LanguageModuleDescriptorRef
	 * @brief A reference class for the `LanguageModuleDescriptor` structure.
	 *
	 * This class holds a reference to a `LanguageModuleDescriptor` object, allowing users to
	 * retrieve detailed information about a language module, such as version, name, description,
	 * URLs, supported platforms, resource directories, and other metadata.
	 */
	class PLUGIFY_API LanguageModuleDescriptorRef : public Ref<const LanguageModuleDescriptor> {
		using Ref::Ref; ///< Inherit constructors from Ref<const LanguageModuleDescriptor>.
	public:
		/**
		 * @brief Retrieves the file version of the language module.
		 *
		 * @return An integer representing the file version.
		 */
		[[nodiscard]] int32_t GetFileVersion() const noexcept;

		/**
		 * @brief Retrieves the version of the language module.
		 *
		 * @return An integer representing the version.
		 */
		[[nodiscard]] int32_t GetVersion() const noexcept;

		/**
		 * @brief Retrieves the version name of the language module.
		 *
		 * @return A string view representing the version name.
		 */
		[[nodiscard]] std::string_view GetVersionName() const noexcept;

		/**
		 * @brief Retrieves the friendly name of the language module.
		 *
		 * @return A string view representing the friendly name.
		 */
		[[nodiscard]] std::string_view GetFriendlyName() const noexcept;

		/**
		 * @brief Retrieves the description of the language module.
		 *
		 * @return A string view representing the description.
		 */
		[[nodiscard]] std::string_view GetDescription() const noexcept;

		/**
		 * @brief Retrieves the name of the creator of the language module.
		 *
		 * @return A string view representing the name of the creator.
		 */
		[[nodiscard]] std::string_view GetCreatedBy() const noexcept;

		/**
		 * @brief Retrieves the URL of the creator of the language module.
		 *
		 * @return A string view representing the creator's URL.
		 */
		[[nodiscard]] std::string_view GetCreatedByURL() const noexcept;

		/**
		 * @brief Retrieves the documentation URL for the language module.
		 *
		 * @return A string view representing the documentation URL.
		 */
		[[nodiscard]] std::string_view GetDocsURL() const noexcept;

		/**
		 * @brief Retrieves the download URL for the language module.
		 *
		 * @return A string view representing the download URL.
		 */
		[[nodiscard]] std::string_view GetDownloadURL() const noexcept;

		/**
		 * @brief Retrieves the update URL for the language module.
		 *
		 * @return A string view representing the update URL.
		 */
		[[nodiscard]] std::string_view GetUpdateURL() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the language module.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		[[nodiscard]] std::span<std::string_view> GetSupportedPlatforms() const noexcept;

		/**
		 * @brief Retrieves the resource directories for the language module.
		 *
		 * @return A span of string views representing the resource directories.
		 */
		[[nodiscard]] std::span<std::string_view> GetResourceDirectories() const noexcept;

		/**
		 * @brief Retrieves the library directories for the language module.
		 *
		 * @return A span of string views representing the library directories.
		 */
		[[nodiscard]] std::span<std::string_view> GetLibraryDirectories() const noexcept;

		/**
		 * @brief Retrieves the programming language of the language module.
		 *
		 * @return A string view representing the language used by the module.
		 */
		[[nodiscard]] std::string_view GetLanguage() const noexcept;

		/**
		 * @brief Checks if the language module is forced to load.
		 *
		 * @return `true` if the module is forced to load, otherwise `false`.
		 */
		[[nodiscard]] bool IsForceLoad() const noexcept;
	};
	static_assert(is_ref_v<LanguageModuleDescriptorRef>);
} // namespace plugify