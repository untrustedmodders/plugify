#pragma once

#include <cstdint>
#include <string>
#include <filesystem>

#include <plugify/api/handle.hpp>

#include <plugify_export.h>

namespace plugify {
	class Module;
	class ModuleManifestHandle;

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
		const std::string& GetName() const noexcept;

		/**
		 * @brief Get the language of the language module.
		 * @return The language of the language module.
		 */
		const std::string& GetLanguage() const noexcept;

		/**
		 * @brief Get the base directory of the language module.
		 * @return The base directory as a filesystem path.
		 */
		const std::filesystem::path& GetBaseDir() const noexcept;

		/**
		 * @brief Get the manifest of the language module.
		 * @return The manifest of the language module.
		 */
		ModuleManifestHandle GetManifest() const noexcept;

		/**
		 * @brief Get the state of the language module.
		 * @return The state of the language module.
		 */
		ModuleState GetState() const noexcept;

		/**
		 * @brief Get the error message associated with the language module.
		 * @return The error message.
		 */
		const std::string& GetError() const noexcept;
	};

	/**
	 * @brief Namespace containing utility functions of ModuleState enum.
	 */
	struct ModuleUtils {
		/**
		 * @brief Convert a ModuleState enum value to its string representation.
		 * @param state The ModuleState value to convert.
		 * @return The string representation of the ModuleState.
		 */
		static constexpr std::string_view ToString(ModuleState state) noexcept {
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
		static constexpr ModuleState FromString(std::string_view state) noexcept {
			if (state == "NotLoaded") {
				return ModuleState::NotLoaded;
			} else if (state == "Error") {
				return ModuleState::Error;
			} else if (state == "Loaded") {
				return ModuleState::Loaded;
			}
			return ModuleState::Unknown;
		}
	};

} // namespace plugify
