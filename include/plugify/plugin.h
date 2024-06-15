#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <filesystem>
#include <plugify_export.h>

namespace plugify {
	class Plugin;
	struct PluginDescriptor;

	/**
	 * @enum PluginState
	 * @brief Represents the possible states of a plugin.
	 *
	 * The PluginState enum defines the various states that a plugin can be in,
	 * such as NotLoaded, Error, Loaded, Running, Terminating, and Unknown.
	 */
	enum class PluginState : std::uint8_t {
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
	using MethodData = std::pair<std::string, void*>;

	/**
	 * @class IPlugin
	 * @brief Interface for plugin functionality.
	 *
	 * The IPlugin class defines an interface for plugins in the PLUGIFY system.
	 * Plugins should inherit from this class and implement its methods to be compatible
	 * with the PLUGIFY framework.
	 */
	class PLUGIFY_API IPlugin {
	protected:
		explicit IPlugin(Plugin& impl);
		~IPlugin();

	public:
		/**
		 * @brief Get the unique identifier of the plugin.
		 * @return The unique identifier.
		 */
		[[nodiscard]] UniqueId GetId() const;

		/**
		 * @brief Get the name of the plugin.
		 * @return The name of the plugin.
		 */
		[[nodiscard]] const std::string& GetName() const;

		/**
		 * @brief Get the friendly name of the plugin.
		 * @return The friendly name of the plugin.
		 */
		[[nodiscard]] const std::string& GetFriendlyName() const;

		/**
		 * @brief Get the base directory of the plugin.
		 * @return The base directory as a filesystem path.
		 */
		[[nodiscard]] const std::filesystem::path& GetBaseDir() const;

		/**
		 * @brief Get the descriptor of the plugin.
		 * @return The descriptor of the plugin.
		 */
		[[nodiscard]] const PluginDescriptor& GetDescriptor() const;

		/**
		 * @brief Get the state of the plugin.
		 * @return The state of the plugin.
		 */
		[[nodiscard]] PluginState GetState() const;

		/**
		 * @brief Get the error message associated with the plugin.
		 * @return The error message.
		 */
		[[nodiscard]] const std::string& GetError() const;

		/**
		 * @brief Get the list of methods supported by the plugin.
		 * @return The list of method data.
		 */
		[[nodiscard]] const std::vector<MethodData>& GetMethods() const;

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

	private:
		Plugin& _impl; ///< The implementation of the plugin.
	};

	/**
	 * @brief Convert a PluginState enum value to its string representation.
	 * @param state The PluginState value to convert.
	 * @return The string representation of the PluginState.
	 */
	[[maybe_unused]] constexpr std::string_view PluginStateToString(PluginState state) {
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
	[[maybe_unused]] constexpr PluginState PluginStateFromString(std::string_view state) {
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
} // namespace plugify
