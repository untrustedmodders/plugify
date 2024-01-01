#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <wizard_export.h>

namespace wizard {
	class Plugin;
	struct PluginDescriptor;

	enum class PluginState : std::uint8_t {
		NotLoaded,
		Error,
		Loaded,
		Running,
		Terminating,
		Unloaded,
		Unknown,
	};

	using MethodData = std::pair<std::string, void*>;

	// Plugin provided to user, which implemented in core
	class WIZARD_API IPlugin {
	protected:
		explicit IPlugin(Plugin& impl);
		~IPlugin();

	public:
		std::uint64_t GetId() const;
		const std::string& GetName() const;
		const std::string& GetFriendlyName() const;
		const std::filesystem::path& GetFilePath() const;
		std::filesystem::path GetBaseDir() const;
		std::filesystem::path GetContentDir() const;
		std::filesystem::path GetMountedAssetPath() const;
		const PluginDescriptor& GetDescriptor() const;
		PluginState GetState() const;
		const std::string& GetError() const;
		const std::vector<MethodData>& GetMethods() const;

	private:
		Plugin& _impl;
	};

	[[maybe_unused]] constexpr std::string_view PluginStateToString(PluginState state) {
		using enum PluginState;
		switch (state) {
			case NotLoaded:   return "NotLoaded";
			case Error:	   return "Error";
			case Loaded:	  return "Loaded";
			case Running:	 return "Running";
			case Terminating: return "Terminating";
			case Unloaded:	return "Unloaded";
			default:					   return "Unknown";
		}
	}
	[[maybe_unused]] constexpr PluginState PluginStateFromString(std::string_view state) {
		using enum PluginState;
		if (state == "NotLoaded") {
			return NotLoaded;
		} else if (state == "Error") {
			return Error;
		} else if (state == "Loaded") {
			return Loaded;
		} else if (state == "Running") {
			return Running;
		} else if (state == "Terminating") {
			return Terminating;
		} else if (state == "Unloaded") {
			return Unloaded;
		}
		return Unknown;
	}
}
