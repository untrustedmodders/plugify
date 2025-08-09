#pragma once

#include <cstdint>
#include <span>
#include <string>

#include <plugify/api/handle.hpp>
#include <plugify/api/version.hpp>

#include <plugify_export.h>

namespace plugify {
	struct ModuleManifest;

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
		std::string_view GetName() const noexcept;

		/**
		 * @brief Retrieves the version of the language module.
		 *
		 * @return A struct representing the version.
		 */
		plg::version GetVersion() const noexcept;

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
		std::string_view GetAuthor() const noexcept;

		/**
		 * @brief Retrieves the URL of the creator of the language module.
		 *
		 * @return A string view representing the creator's URL.
		 */
		std::string_view GetWebsite() const noexcept;

		/**
		 * @brief Retrieves the license for the language module.
		 *
		 * @return A string view representing the license.
		 */
		std::string_view GetLicense() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the language module.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		std::span<const std::string_view> GetPlatforms() const noexcept;

		/**
		 * @brief Retrieves the library directories for the language module.
		 *
		 * @return A span of string views representing the library directories.
		 */
		std::span<const std::string_view> GetDirectories() const noexcept;

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
