#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <plugify_export.h>

namespace plugify {
	class Plugin;
	struct PluginDescriptor;

	enum class PluginState : std::uint8_t {
		NotLoaded,
		Error,
		Loaded,
		Running,
		Terminating,
		Unknown,
	};

	using UniqueId = std::uintmax_t;
	using MethodData = std::pair<std::string, void*>;

	// Plugin provided to user, which implemented in core
	class PLUGIFY_API IPlugin {
	protected:
		explicit IPlugin(Plugin& impl);
		~IPlugin();

	public:
		UniqueId GetId() const;
		const std::string& GetName() const;
		const std::string& GetFriendlyName() const;
		const std::filesystem::path& GetBaseDir() const;
		const PluginDescriptor& GetDescriptor() const;
		PluginState GetState() const;
		const std::string& GetError() const;
		const std::vector<MethodData>& GetMethods() const;

	private:
		Plugin& _impl;
	};

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
}
