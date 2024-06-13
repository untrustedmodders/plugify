#include "plugify_provider.h"
#include <plugify/plugin_manager.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin.h>
#include <plugify/module.h>

using namespace plugify;

PlugifyProvider::PlugifyProvider(std::weak_ptr<IPlugify> plugify) : IPlugifyProvider(*this), PlugifyContext(std::move(plugify)) {
}

PlugifyProvider::~PlugifyProvider() = default;

void PlugifyProvider::Log(const std::string& msg, Severity severity) {
	if (auto plugify = _plugify.lock()) {
		plugify->Log(msg, severity);
	}
}

const fs::path& PlugifyProvider::GetBaseDir() {
	if (auto plugify = _plugify.lock()) {
		return plugify->GetConfig().baseDir;
	}
	static fs::path _;
	return _;
}

bool PlugifyProvider::IsPreferOwnSymbols() const {
	if (auto plugify = _plugify.lock()) {
		return plugify->GetConfig().preferOwnSymbols;
	}
	return false;
}

bool PlugifyProvider::IsPluginLoaded(const std::string& name, std::optional<int32_t> requiredVersion, bool minimum) {
	if (auto plugify = _plugify.lock()) {
		if (auto pluginManager = plugify->GetPluginManager().lock()) {
			auto pluginRef = pluginManager->FindPlugin(name);
			if (!pluginRef.has_value())
				return false;
			const auto& plugin = pluginRef->get();
			if (plugin.GetState() != PluginState::Loaded && plugin.GetState() != PluginState::Running)
				return false;
			if (requiredVersion.has_value()) {
				if (minimum) {
					return plugin.GetDescriptor().version >= *requiredVersion;
				} else {
					return plugin.GetDescriptor().version == *requiredVersion;
				}
			} else {
				return true;
			}
		}
	}
	return false;
}

bool PlugifyProvider::IsModuleLoaded(const std::string& name, std::optional<int32_t> requiredVersion, bool minimum) {
	if (auto plugify = _plugify.lock()) {
		if (auto pluginManager = plugify->GetPluginManager().lock()) {
			auto moduleRef = pluginManager->FindModule(name);
			if (!moduleRef.has_value())
				return false;
			const auto& module = moduleRef->get();
			if (module.GetState() != ModuleState::Loaded)
				return false;
			if (requiredVersion.has_value()) {
				if (minimum) {
					return module.GetDescriptor().version >= *requiredVersion;
				} else {
					return module.GetDescriptor().version == *requiredVersion;
				}
			} else {
				return true;
			}
		}
	}
	return false;
}