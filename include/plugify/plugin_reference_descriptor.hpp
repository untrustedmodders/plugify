#pragma once

#include <optional>
#include <span>
#include <string>
#include <plugify/handle.hpp>
#include <plugify_export.h>

namespace plugify {
	struct PluginReferenceDescriptor;

	/**
	 * @class PluginReferenceDescriptorHandle
	 * @brief A handle class for the `PluginReferenceDescriptor` structure.
	 */
	class PLUGIFY_API PluginReferenceDescriptorHandle : public Handle<const PluginReferenceDescriptor> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the name of the referenced plugin.
		 *
		 * @return A string view representing the name of the plugin.
		 */
		std::string_view GetName() const noexcept;

		/**
		 * @brief Checks if the referenced plugin is optional.
		 *
		 * @return `true` if the plugin is optional, otherwise `false`.
		 */
		bool IsOptional() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the referenced plugin.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		std::span<const std::string_view> GetSupportedPlatforms() const noexcept;

		/**
		 * @brief Retrieves the requested version of the referenced plugin, if specified.
		 *
		 * @return An optional integer representing the requested version. If no version is requested, returns an empty optional.
		 */
		std::optional<plg::version> GetRequestedVersion() const noexcept;
	};
} // namespace plugify
