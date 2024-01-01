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

	// Language module provided to user implemented in core !
	class WIZARD_API IModule {
	protected:
		explicit IModule(Module& impl);
		~IModule() = default;

	public:
		const std::string& GetName() const;
		const std::string& GetFriendlyName() const;
		const std::filesystem::path& GetFilePath() const;
		std::filesystem::path GetBaseDir() const;
		std::filesystem::path GetBinariesDir() const;
		const LanguageModuleDescriptor& GetDescriptor() const;
		ModuleState GetState() const;
		const std::string& GetError() const;

	private:
		Module& _impl;
	};

	[[maybe_unused]] constexpr std::string_view ModuleStateToString(ModuleState state) {
		using enum ModuleState;
		switch (state) {
			case NotLoaded: return "NotLoaded";
			case Error:	 return "Error";
			case Loaded:	return "Loaded";
			default:		return "Unknown";
		}
	}
	[[maybe_unused]] constexpr ModuleState ModuleStateFromString(std::string_view state) {
		using enum ModuleState;
		if (state == "NotLoaded") {
			return NotLoaded;
		} else if (state == "Error") {
			return Error;
		} else if (state == "Loaded") {
			return Loaded;
		}
		return Unknown;
	}
}
