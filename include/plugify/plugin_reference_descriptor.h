#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace plugify {
	/**
	 * @struct PluginReferenceDescriptor
	 * @brief Describes the properties of a plugin reference.
	 *
	 * The PluginReferenceDescriptor structure holds information about a plugin reference,
	 * including the name, whether it is optional, supported platforms, and an optional requested version.
	 */
	struct PluginReferenceDescriptor final {
		std::string name; ///< The name of the plugin reference.
		bool optional{false}; ///< Indicates whether the plugin reference is optional.
		//std::string description; ///< The description of the plugin reference.
		//std::string downloadURL; ///< The download URL of the plugin reference.
		std::vector<std::string> supportedPlatforms; ///< The platforms supported by the plugin reference.
		std::optional<int32_t> requestedVersion; ///< Optional requested version of the plugin reference.

		/**
		 * @brief Overloaded equality operator for comparing PluginReferenceDescriptor instances.
		 * @param rhs The right-hand side PluginReferenceDescriptor for comparison.
		 * @return True if the names of this instance and rhs are equal.
		 */
		[[nodiscard]] bool operator==(const PluginReferenceDescriptor& rhs) const { return name == rhs.name; }
	};
} // namespace plugify