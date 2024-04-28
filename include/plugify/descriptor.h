#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace plugify {
	/**
	 * @struct Descriptor
	 * @brief Describes the properties of an object. (e.g., module, plugin)
	 *
	 * The Descriptor structure holds information about an object, including file version,
	 * version, version name, friendly name, description, creator information, documentation URL,
	 * download URL, update URL, and supported platforms.
	 */
	struct Descriptor {
		std::int32_t fileVersion{}; ///< The file version of the object.
		std::int32_t version{}; ///< The version number of the object.
		std::string versionName; ///< The version name of the object.
		std::string friendlyName; ///< The friendly name of the object.
		std::string description; ///< The description of the object.
		std::string createdBy; ///< The creator/author of the object.
		std::string createdByURL; ///< The URL of the creator/author.
		std::string docsURL; ///< The documentation URL of the object.
		std::string downloadURL; ///< The download URL of the object.
		std::string updateURL; ///< The update URL of the object.
		std::vector<std::string> supportedPlatforms; ///< The platforms supported by the object.
		std::optional<std::vector<std::string>> resourceDirectories; ///< Optional resource directories for the object.
	};
} // namespace plugify