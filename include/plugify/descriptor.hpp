#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <plugify/version.hpp>

namespace plugify {
	/**
	 * @struct Descriptor
	 * @brief Describes the properties of an object. (e.g., module, plugin)
	 */
	struct Descriptor {
		int32_t fileVersion{}; ///< The file version of the object.
		plg::version version{}; ///< The semantic version of the object.
		std::string versionName; ///< The version name of the object.
		std::string friendlyName; ///< The friendly name of the object.
		std::optional<std::string> description; ///< The description of the object.
		std::optional<std::string> createdBy; ///< The creator/author of the object.
		std::optional<std::string> createdByURL; ///< The URL of the creator/author.
		std::optional<std::string> docsURL; ///< The documentation URL of the object.
		std::optional<std::string> downloadURL; ///< The download URL of the object.
		std::optional<std::string> updateURL; ///< The update URL of the object.
		std::optional<std::vector<std::string>> supportedPlatforms; ///< The platforms supported by the object.
		std::optional<std::vector<std::string>> resourceDirectories; ///< Optional resource directories for the object.
	};
} // namespace plugify
