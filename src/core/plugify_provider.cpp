#include "plugify_provider.hpp"
#include "plugin_manifest.hpp"
#include <plugify/api/module.hpp>
#include <plugify/api/module_manifest.hpp>
#include <plugify/api/plugin.hpp>
#include <plugify/api/plugin_manager.hpp>
#include <plugify/api/plugin_manifest.hpp>

using namespace plugify;
static fs::path dummy;

PlugifyProvider::PlugifyProvider(std::weak_ptr<IPlugify> plugify)
	: IPlugifyProvider(*this)
	, PlugifyContext(std::move(plugify)) {
}

PlugifyProvider::~PlugifyProvider() = default;

void PlugifyProvider::Log(std::string_view msg, Severity severity) {
	if (auto plugify = _plugify.lock()) {
		plugify->Log(msg, severity);
	}
}

const fs::path& PlugifyProvider::GetBaseDir() noexcept {
	if (auto plugify = _plugify.lock()) {
		return plugify->GetConfig().baseDir;
	}
	return dummy;
}

const fs::path& PlugifyProvider::GetConfigsDir() noexcept {
	if (auto plugify = _plugify.lock()) {
		return plugify->GetConfig().configsDir;
	}
	return dummy;
}

const fs::path& PlugifyProvider::GetDataDir() noexcept {
	if (auto plugify = _plugify.lock()) {
		return plugify->GetConfig().dataDir;
	}
	return dummy;
}

const fs::path& PlugifyProvider::GetLogsDir() noexcept {
	if (auto plugify = _plugify.lock()) {
		return plugify->GetConfig().logsDir;
	}
	return dummy;
}

bool PlugifyProvider::IsPreferOwnSymbols() noexcept {
	if (auto plugify = _plugify.lock()) {
		return plugify->GetConfig().preferOwnSymbols.value_or(false);
	}
	return false;
}

bool PlugifyProvider::IsPluginLoaded(std::string_view name, std::optional<Constraint> constraint) noexcept {
	if (auto plugify = _plugify.lock()) {
		if (auto pluginManager = plugify->GetPluginManager().lock()) {
			auto plugin = pluginManager->FindPlugin(name);
			if (!plugin)
				return false;
			if (plugin.GetState() != PluginState::Loaded && plugin.GetState() != PluginState::Running)
				return false;
			if (constraint)
				return constraint->IsSatisfiedBy(plugin.GetManifest().GetVersion());
			return true;
		}
	}
	return false;
}

bool PlugifyProvider::IsModuleLoaded(std::string_view name, std::optional<Constraint> constraint) noexcept {
	if (auto plugify = _plugify.lock()) {
		if (auto pluginManager = plugify->GetPluginManager().lock()) {
			auto module = pluginManager->FindModule(name);
			if (!module)
				return false;
			if (module.GetState() != ModuleState::Loaded)
				return false;
			if (constraint)
				return constraint->IsSatisfiedBy(module.GetManifest().GetVersion());
			return true;
		}
	}
	return false;
}

PluginHandle PlugifyProvider::FindPlugin(std::string_view name) noexcept {
	if (auto plugify = _plugify.lock()) {
		if (auto pluginManager = plugify->GetPluginManager().lock()) {
			return pluginManager->FindPlugin(name);
		}
	}
	return {};
}

ModuleHandle PlugifyProvider::FindModule(std::string_view name) noexcept {
	if (auto plugify = _plugify.lock()) {
		if (auto pluginManager = plugify->GetPluginManager().lock()) {
			return pluginManager->FindModule(name);
		}
	}
	return {};
}
