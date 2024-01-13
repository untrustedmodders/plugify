#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <wizard_export.h>

namespace wizard {
	class Module;
	struct LanguageModuleDescriptor;

	enum class ModuleState : std::uint8_t {
		NotLoaded,
		Error,
		Loaded,
		Unknown,
	};

	using UniqueId = std::uintmax_t;

	// Language module provided to user implemented in core !
	class WIZARD_API IModule {
	protected:
		explicit IModule(Module& impl);
		~IModule() = default;

	public:
		UniqueId GetId() const;
		const std::string& GetName() const;
		const std::string& GetLanguage() const;
		const std::string& GetFriendlyName() const;
		const std::filesystem::path& GetFilePath() const;
		const std::filesystem::path& GetBaseDir() const;
		const std::filesystem::path& GetBinariesDir() const;
		const LanguageModuleDescriptor& GetDescriptor() const;
		ModuleState GetState() const;
		const std::string& GetError() const;

	private:
		Module& _impl;
	};

	[[maybe_unused]] constexpr std::string_view ModuleStateToString(ModuleState state) {
		switch (state) {
			case ModuleState::NotLoaded: return "NotLoaded";
			case ModuleState::Error:     return "Error";
			case ModuleState::Loaded:    return "Loaded";
			default:                     return "Unknown";
		}
	}
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
}
