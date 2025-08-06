#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "handle.hpp"
#include "version.hpp"

#include <plugify_export.h>

namespace plugify {
	struct LanguageModuleDescriptor;

	/**
	 * @class LanguageModuleDescriptorHandle
	 * @brief A handle class for the `LanguageModuleDescriptor` structure.
	 */
	class PLUGIFY_API LanguageModuleDescriptorHandle : public Handle<const LanguageModuleDescriptor> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the file version of the language module.
		 *
		 * @return An integer representing the file version.
		 */
		int32_t GetFileVersion() const noexcept;

		/**
		 * @brief Retrieves the version of the language module.
		 *
		 * @return A struct representing the version.
		 */
		plg::version GetVersion() const noexcept;

		/**
		 * @brief Retrieves the version name of the language module.
		 *
		 * @return A string view representing the version name.
		 */
		std::string_view GetVersionName() const noexcept;

		/**
		 * @brief Retrieves the friendly name of the language module.
		 *
		 * @return A string view representing the friendly name.
		 */
		std::string_view GetFriendlyName() const noexcept;

		/**
		 * @brief Retrieves the description of the language module.
		 *
		 * @return A string view representing the description.
		 */
		std::string_view GetDescription() const noexcept;

		/**
		 * @brief Retrieves the name of the creator of the language module.
		 *
		 * @return A string view representing the name of the creator.
		 */
		std::string_view GetCreatedBy() const noexcept;

		/**
		 * @brief Retrieves the URL of the creator of the language module.
		 *
		 * @return A string view representing the creator's URL.
		 */
		std::string_view GetCreatedByURL() const noexcept;

		/**
		 * @brief Retrieves the documentation URL for the language module.
		 *
		 * @return A string view representing the documentation URL.
		 */
		std::string_view GetDocsURL() const noexcept;

		/**
		 * @brief Retrieves the download URL for the language module.
		 *
		 * @return A string view representing the download URL.
		 */
		std::string_view GetDownloadURL() const noexcept;

		/**
		 * @brief Retrieves the update URL for the language module.
		 *
		 * @return A string view representing the update URL.
		 */
		std::string_view GetUpdateURL() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the language module.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		std::span<const std::string_view> GetSupportedPlatforms() const noexcept;

		/**
		 * @brief Retrieves the library directories for the language module.
		 *
		 * @return A span of string views representing the library directories.
		 */
		std::span<const std::string_view> GetLibraryDirectories() const noexcept;

		/**
		 * @brief Retrieves the programming language of the language module.
		 *
		 * @return A string view representing the language used by the module.
		 */
		std::string_view GetLanguage() const noexcept;

		/**
		 * @brief Checks if the language module is forced to load.
		 *
		 * @return `true` if the module is forced to load, otherwise `false`.
		 */
		bool IsForceLoad() const noexcept;
	};
} // namespace plugify
