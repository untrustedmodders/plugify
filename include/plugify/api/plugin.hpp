#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>

#include <plugify/api/handle.hpp>
#include <plugify/api/method.hpp>
#include <plugify/asm/mem_addr.hpp>

#include <plugify_export.h>

namespace plugify {
	class Plugin;
	class PluginManifestHandle;
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
		const std::string& GetName() const noexcept;

		/**
		 * @brief Get the base directory of the plugin.
		 * @return The base directory as a filesystem path.
		 */
		const std::filesystem::path& GetBaseDir() const noexcept;

		/**
		 * @brief Get the configs directory of the plugin.
		 * @return The configs directory as a filesystem path.
		 */
		const std::filesystem::path& GetConfigsDir() const noexcept;

		/**
		 * @brief Get the data directory of the plugin.
		 * @return The data directory as a filesystem path.
		 */
		const std::filesystem::path& GetDataDir() const noexcept;

		/**
		 * @brief Get the logs directory of the plugin.
		 * @return The logs directory as a filesystem path.
		 */
		const std::filesystem::path& GetLogsDir() const noexcept;

		/**
		 * @brief Get the manifest of the plugin.
		 * @return The manifest of the plugin.
		 */
		PluginManifestHandle GetManifest() const noexcept;

		/**
		 * @brief Get the state of the plugin.
		 * @return The state of the plugin.
		 */
		PluginState GetState() const noexcept;

		/**
		 * @brief Get the error message associated with the plugin.
		 * @return The error message.
		 */
		const std::string& GetError() const noexcept;

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
	};

	/**
	 * @brief Namespace containing utility functions of PluginState enum.
	 */
	struct PluginUtils {
		/**
		 * @brief Convert a PluginState enum value to its string representation.
		 * @param state The PluginState value to convert.
		 * @return The string representation of the PluginState.
		 */
		static constexpr std::string_view ToString(PluginState state) {
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
		static constexpr PluginState FromString(std::string_view state) {
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
	};

} // namespace plugify
