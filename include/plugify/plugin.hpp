#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <plugify/handle.hpp>
#include <plugify/mem_addr.hpp>
#include <plugify/path.hpp>
#include <plugify_export.h>

namespace plugify {
	class Plugin;
	class PluginDescriptorHandle;
	class MethodHandle;

	/**
	 * @enum PluginState
	 * @brief Represents the possible states of a plugin.
	 *
	 * The PluginState enum defines the various states that a plugin can be in,
	 * such as NotLoaded, Error, Loaded, Running, Terminating, and Unknown.
	 */
	enum class PluginState {
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
	 * The MethodData type is a pair consisting of a method handle and a
	 * pointer to the method's address (void*).
	 */
	using MethodData = std::pair<MethodHandle, MemAddr>;

	/**
	 * @class PluginHandle
	 * @brief Handle wrapper to access plugin's information.
	 */
	class PLUGIFY_API PluginHandle : public Handle<const Plugin> {
		using Handle::Handle;
	public:
		/**
		 * @brief Get the unique identifier of the plugin.
		 * @return The unique identifier.
		 */
		UniqueId GetId() const noexcept;

		/**
		 * @brief Get the name of the plugin.
		 * @return The name of the plugin.
		 */
		std::string_view GetName() const noexcept;

		/**
		 * @brief Get the friendly name of the plugin.
		 * @return The friendly name of the plugin.
		 */
		std::string_view GetFriendlyName() const noexcept;

		/**
		 * @brief Get the base directory of the plugin.
		 * @return The base directory as a filesystem path.
		 */
		std::filesystem::path_view GetBaseDir() const noexcept;

		/**
		 * @brief Get the descriptor of the plugin.
		 * @return The descriptor of the plugin.
		 */
		PluginDescriptorHandle GetDescriptor() const noexcept;

		/**
		 * @brief Get the state of the plugin.
		 * @return The state of the plugin.
		 */
		PluginState GetState() const noexcept;

		/**
		 * @brief Get the error message associated with the plugin.
		 * @return The error message.
		 */
		std::string_view GetError() const noexcept;

		/**
		 * @brief Get the list of methods supported by the plugin.
		 * @return The list of method data.
		 */
		std::span<const MethodData> GetMethods() const noexcept;

		/**
		 * @brief The data argument specified when loading the plugin.
		 * @return The data of the plugin.
		 */
		MemAddr GetData() const noexcept;

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
		std::optional<std::filesystem::path_view> FindResource(std::filesystem::path_view path) const;
	};

	/**
	 * @brief Namespace containing utility functions of PluginState enum.
	 */
	namespace PluginUtils {
		/**
		 * @brief Convert a PluginState enum value to its string representation.
		 * @param state The PluginState value to convert.
		 * @return The string representation of the PluginState.
		 */
		constexpr std::string_view ToString(PluginState state) {
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
		constexpr PluginState FromString(std::string_view state) {
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
