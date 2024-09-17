#pragma once

#include <span>
#include <string>
#include <optional>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

namespace plugify {
	struct PluginReferenceDescriptor;

	/**
	 * @class PluginReferenceDescriptorRef
	 * @brief A reference class for the `PluginReferenceDescriptor` structure.
	 *
	 * This class holds a reference to a `PluginReferenceDescriptor` object, allowing users to retrieve
	 * information about plugin dependencies, such as the plugin name, optionality, supported platforms,
	 * and requested version.
	 */
	class PLUGIFY_API PluginReferenceDescriptorRef : public Ref<const PluginReferenceDescriptor> {
		using Ref::Ref; ///< Inherit constructors from Ref<const PluginReferenceDescriptor>.
	public:
		/**
		 * @brief Retrieves the name of the referenced plugin.
		 *
		 * @return A string view representing the name of the plugin.
		 */
		[[nodiscard]] std::string_view GetName() const noexcept;

		/**
		 * @brief Checks if the referenced plugin is optional.
		 *
		 * @return `true` if the plugin is optional, otherwise `false`.
		 */
		[[nodiscard]] bool IsOptional() const noexcept;

		/**
		 * @brief Retrieves the supported platforms for the referenced plugin.
		 *
		 * @return A span of string views representing the supported platforms.
		 */
		[[nodiscard]] std::span<std::string_view> GetSupportedPlatforms() const noexcept;

		/**
		 * @brief Retrieves the requested version of the referenced plugin, if specified.
		 *
		 * @return An optional integer representing the requested version. If no version is requested, returns an empty optional.
		 */
		[[nodiscard]] std::optional<int32_t> GetRequestedVersion() const noexcept;
	};
	static_assert(is_ref_v<PluginReferenceDescriptorRef>);
} // namespace plugify