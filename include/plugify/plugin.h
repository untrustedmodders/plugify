#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <filesystem>
#include <plugify/mem_addr.h>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

namespace plugify {
#if PLUGIFY_CORE
	class Plugin;
#endif
	class PluginDescriptorRef;

	/**
	 * @enum PluginState
	 * @brief Represents the possible states of a plugin.
	 *
	 * The PluginState enum defines the various states that a plugin can be in,
	 * such as NotLoaded, Error, Loaded, Running, Terminating, and Unknown.
	 */
	enum class PluginState : uint8_t {
		NotLoaded,
		Error,
		Loaded,
		Running,
		Terminating,
		Unknown,
	};

	/**
	 * @typedef UniqueId
	 * @brief Represents a unique identifier for plugins.
	 */
	using UniqueId = std::ptrdiff_t;

	/**
	 * @typedef MethodData
	 * @brief Represents data related to a plugin method.
	 *
	 * The MethodData type is a pair consisting of a method name (string) and a
	 * pointer to the method's address (void*).
	 */
	using MethodData = std::pair<std::string, MemAddr>;

	/**
	 * @class PluginRef
	 * @brief Reference wrapper to access for plugin's information.
	 */
	class PLUGIFY_API PluginRef {
		PLUGUFY_REFERENCE(PluginRef, const Plugin)
	public:
		/**
		 * @brief Get the unique identifier of the plugin.
		 * @return The unique identifier.
		 */
		[[nodiscard]] UniqueId GetId() const noexcept;

		/**
		 * @brief Get the name of the plugin.
		 * @return The name of the plugin.
		 */
		[[nodiscard]] std::string_view GetName() const noexcept;

		/**
		 * @brief Get the friendly name of the plugin.
		 * @return The friendly name of the plugin.
		 */
		[[nodiscard]] std::string_view GetFriendlyName() const noexcept;

		/**
		 * @brief Get the base directory of the plugin.
		 * @return The base directory as a filesystem path.
		 */
		[[nodiscard]] const std::filesystem::path& GetBaseDir() const noexcept;

		/**
		 * @brief Get the descriptor of the plugin.
		 * @return The descriptor of the plugin.
		 */
		[[nodiscard]] PluginDescriptorRef GetDescriptor() const noexcept;

		/**
		 * @brief Get the state of the plugin.
		 * @return The state of the plugin.
		 */
		[[nodiscard]] PluginState GetState() const noexcept;

		/**
		 * @brief Get the error message associated with the plugin.
		 * @return The error message.
		 */
		[[nodiscard]] std::string_view GetError() const noexcept;

		/**
		 * @brief Get the list of methods supported by the plugin.
		 * @return The list of method data.
		 */
		[[nodiscard]] std::span<const MethodData> GetMethods() const noexcept;

		/**
		 * @brief Find a resource file associated with the plugin.
		 *
		 * This method attempts to find a resource file located within the plugin's directory structure.
		 * If the resource file is found, its path is returned. If the resource file does not exist
		 * within the plugin's directory, std::nullopt is returned.
		 *
		 * If a user-overridden file exists in the base directory of Plugify with the same name and path,
		 * the path returned by this function will direct to that overridden file.
		 *
		 * @param path The relative path to the resource file.
		 * @return An optional containing the absolute path to the resource file if found, or std::nullopt otherwise.
		 *
		 * @code
		 * Example:
		 * // Assuming the plugin name is "sample_plugin"
		 * // File located at: plugify/plugins/sample_plugin/configs/core.cfg
		 * // User-overridden file could be located at: plugify/configs/core.cfg
		 * auto resourcePath = plugin.FindResource("configs/core.cfg");
		 * @endcode
		 */
		[[nodiscard]] std::optional<std::filesystem::path> FindResource(const std::filesystem::path& path) const;
	};
	static_assert(is_ref_v<PluginRef>);

	/**
	 * @brief Namespace containing utility functions of PluginState enum.
	 */
	namespace PluginUtils {
		/**
		 * @brief Convert a PluginState enum value to its string representation.
		 * @param state The PluginState value to convert.
		 * @return The string representation of the PluginState.
		 */
		[[maybe_unused]] constexpr std::string_view ToString(PluginState state) {
			switch (state) {
				case PluginState::NotLoaded:   return "NotLoaded";
				case PluginState::Error:       return "Error";
				case PluginState::Loaded:      return "Loaded";
				case PluginState::Running:     return "Running";
				case PluginState::Terminating: return "Terminating";
				default:                       return "Unknown";
			}
		}

		/**
		 * @brief Convert a string representation to a PluginState enum value.
		 * @param state The string representation of PluginState.
		 * @return The corresponding PluginState enum value.
		 */
		[[maybe_unused]] constexpr PluginState FromString(std::string_view state) {
			if (state == "NotLoaded") {
				return PluginState::NotLoaded;
			} else if (state == "Error") {
				return PluginState::Error;
			} else if (state == "Loaded") {
				return PluginState::Loaded;
			} else if (state == "Running") {
				return PluginState::Running;
			} else if (state == "Terminating") {
				return PluginState::Terminating;
			}
			return PluginState::Unknown;
		}
	} // namespace PluginUtils

} // namespace plugify
