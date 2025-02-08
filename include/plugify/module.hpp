#pragma once

#include <cstdint>
#include <optional>

#include "handle.hpp"
#include "path.hpp"

#include <plugify_export.h>

namespace plugify {
	class Module;
	class LanguageModuleDescriptorHandle;

	/**
	 * @enum ModuleState
	 * @brief Represents the possible states of a module.
	 *
	 * The ModuleState enum defines the various states that a module can be in,
	 * such as NotLoaded, Error, Loaded, and Unknown.
	 */
	enum class ModuleState {
		NotLoaded,
		Error,
		Loaded,
		Unknown,
	};

	/**
	 * @typedef UniqueId
	 * @brief Represents a unique identifier for modules.
	 */
	using UniqueId = std::ptrdiff_t;

	/**
	 * @class ModuleHandle
	 * @brief Handle wrapper to access language module's information.
	 */
	class PLUGIFY_API ModuleHandle : public Handle<const Module> {
		using Handle::Handle;
	public:
		/**
		 * @brief Get the unique identifier of the language module.
		 * @return The unique identifier.
		 */
		UniqueId GetId() const noexcept;

		/**
		 * @brief Get the name of the language module.
		 * @return The name of the language module.
		 */
		std::string_view GetName() const noexcept;

		/**
		 * @brief Get the language of the language module.
		 * @return The language of the language module.
		 */
		std::string_view GetLanguage() const noexcept;

		/**
		 * @brief Get the friendly name of the language module.
		 * @return The friendly name of the language module.
		 */
		std::string_view GetFriendlyName() const noexcept;

		/**
		 * @brief Get the file path of the language module.
		 * @return The file path as a filesystem path.
		 */
		std::filesystem::path_view GetFilePath() const noexcept;

		/**
		 * @brief Get the base directory of the language module.
		 * @return The base directory as a filesystem path.
		 */
		std::filesystem::path_view GetBaseDir() const noexcept;

		/**
		 * @brief Get the descriptor of the language module.
		 * @return The descriptor of the language module.
		 */
		LanguageModuleDescriptorHandle GetDescriptor() const noexcept;

		/**
		 * @brief Get the state of the language module.
		 * @return The state of the language module.
		 */
		ModuleState GetState() const noexcept;

		/**
		 * @brief Get the error message associated with the language module.
		 * @return The error message.
		 */
		std::string_view GetError() const noexcept;

		/**
		 * @brief Find a resource file associated with the module.
		 *
		 * This method attempts to find a resource file located within the module's directory structure.
		 * If the resource file is found, its path is returned. If the resource file does not exist
		 * within the module's directory, std::nullopt is returned.
		 *
		 * If a user-overridden file exists in the base directory of Plugify with the same name and path,
		 * the path returned by this function will direct to that overridden file.
		 *
		 * @param path The relative path to the resource file.
		 * @return An optional containing the absolute path to the resource file if found, or std::nullopt otherwise.
		 *
		 * @code
		 * Example:
		 * // Assuming the module name is "sample_module"
		 * // File located at: plugify/modules/sample_module/configs/core.cfg
		 * // User-overridden file could be located at: plugify/configs/core.cfg
		 * auto resourcePath = module.FindResource("configs/core.cfg");
		 * @endcode
		 */
		std::optional<std::filesystem::path_view> FindResource(std::filesystem::path_view path) const;
	};

	/**
	 * @brief Namespace containing utility functions of ModuleState enum.
	 */
	namespace ModuleUtils {
		/**
		 * @brief Convert a ModuleState enum value to its string representation.
		 * @param state The ModuleState value to convert.
		 * @return The string representation of the ModuleState.
		 */
		constexpr std::string_view ToString(ModuleState state) noexcept {
			switch (state) {
				case ModuleState::NotLoaded: return "NotLoaded";
				case ModuleState::Error:     return "Error";
				case ModuleState::Loaded:    return "Loaded";
				default:                     return "Unknown";
			}
		}

		/**
		 * @brief Convert a string representation to a ModuleState enum value.
		 * @param state The string representation of ModuleState.
		 * @return The corresponding ModuleState enum value.
		 */
		constexpr ModuleState FromString(std::string_view state) noexcept {
			if (state == "NotLoaded") {
				return ModuleState::NotLoaded;
			} else if (state == "Error") {
				return ModuleState::Error;
			} else if (state == "Loaded") {
				return ModuleState::Loaded;
			}
			return ModuleState::Unknown;
		}
	} // namespace ModuleUtils

} // namespace plugify
