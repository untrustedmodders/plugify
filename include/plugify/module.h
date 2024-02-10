#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <plugify_export.h>

namespace plugify {
	class Module;
	struct LanguageModuleDescriptor;

	/**
	 * @enum ModuleState
	 * @brief Represents the possible states of a module.
	 *
	 * The ModuleState enum defines the various states that a module can be in,
	 * such as NotLoaded, Error, Loaded, and Unknown.
	 */
	enum class ModuleState : std::uint8_t {
		NotLoaded,
		Error,
		Loaded,
		Unknown,
	};

	/**
	 * @typedef UniqueId
	 * @brief Represents a unique identifier for modules.
	 */
	using UniqueId = std::uintmax_t;

	/**
	 * @class IModule
	 * @brief Interface for language modules in the PLUGIFY system.
	 *
	 * The IModule class is a base class for language modules. It provides basic
	 * functionality and methods that language modules must implement to interact with
	 * the PLUGIFY framework.
	 */
	class PLUGIFY_API IModule {
	protected:
		explicit IModule(Module& impl);
		~IModule() = default;

	public:
		/**
		 * @brief Get the unique identifier of the language module.
		 * @return The unique identifier.
		 */
		UniqueId GetId() const;

		/**
		 * @brief Get the name of the language module.
		 * @return The name of the language module.
		 */
		const std::string& GetName() const;

		/**
		 * @brief Get the language of the language module.
		 * @return The language of the language module.
		 */
		const std::string& GetLanguage() const;

		/**
		 * @brief Get the friendly name of the language module.
		 * @return The friendly name of the language module.
		 */
		const std::string& GetFriendlyName() const;

		/**
		 * @brief Get the file path of the language module.
		 * @return The file path as a filesystem path.
		 */
		const std::filesystem::path& GetFilePath() const;

		/**
		 * @brief Get the base directory of the language module.
		 * @return The base directory as a filesystem path.
		 */
		const std::filesystem::path& GetBaseDir() const;

		/**
		 * @brief Get the descriptor of the language module.
		 * @return The descriptor of the language module.
		 */
		const LanguageModuleDescriptor& GetDescriptor() const;

		/**
		 * @brief Get the state of the language module.
		 * @return The state of the language module.
		 */
		ModuleState GetState() const;

		/**
		 * @brief Get the error message associated with the language module.
		 * @return The error message.
		 */
		const std::string& GetError() const;

	private:
		Module& _impl; ///< The implementation of the language module.
	};

	/**
	 * @brief Convert a ModuleState enum value to its string representation.
	 * @param state The ModuleState value to convert.
	 * @return The string representation of the ModuleState.
	 */
	[[maybe_unused]] constexpr std::string_view ModuleStateToString(ModuleState state) {
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
	[[maybe_unused]] constexpr ModuleState ModuleStateFromString(std::string_view state) {
		if (state == "NotLoaded") {
			return ModuleState::NotLoaded;
		} else if (state == "Error") {
			return ModuleState::Error;
		} else if (state == "Loaded") {
			return ModuleState::Loaded;
		}
		return ModuleState::Unknown;
	}
} // namespace plugify
