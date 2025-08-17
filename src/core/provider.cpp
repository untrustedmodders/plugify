#include "plugify.hpp"
#include "provider.hpp"
#include "plugin_manifest.hpp"
#include <plugify/api/manager.hpp>
#include <plugify/api/module.hpp>
#include <plugify/api/module_manifest.hpp>
#include <plugify/api/plugin.hpp>
#include <plugify/api/plugin_manifest.hpp>

using namespace plugify;

Provider::Provider(Plugify& plugify) : Context(plugify) {}

Provider::~Provider() = default;

void Provider::Log(std::string_view msg, Severity severity) const {
	_plugify.Log(msg, severity);
}

const fs::path& Provider::GetBaseDir() const noexcept {
	return _plugify.GetConfig().baseDir;
}

const fs::path& Provider::GetConfigsDir() const noexcept {
	return _plugify.GetConfig().configsDir;
}

const fs::path& Provider::GetDataDir() const noexcept {
	return _plugify.GetConfig().dataDir;
}

const fs::path& Provider::GetLogsDir() const noexcept {
	return _plugify.GetConfig().logsDir;
}

bool Provider::IsPreferOwnSymbols() const noexcept {
	return _plugify.GetConfig().preferOwnSymbols.value_or(false);
}

bool Provider::IsPluginLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
	auto plugin = _plugify.GetManager().FindPlugin(name);
	if (!plugin)
		return false;
	if (plugin.GetState() != PluginState::Loaded && plugin.GetState() != PluginState::Running)
		return false;
	if (constraint)
		return constraint->IsSatisfiedBy(plugin.GetManifest().GetVersion());
	return true;
}

bool Provider::IsModuleLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
	auto module = _plugify.GetManager().FindModule(name);
	if (!module)
		return false;
	if (module.GetState() != ModuleState::Loaded)
		return false;
	if (constraint)
		return constraint->IsSatisfiedBy(module.GetManifest().GetVersion());
	return true;
}

PluginHandle Provider::FindPlugin(std::string_view name) const noexcept {
	return _plugify.GetManager().FindPlugin(name);
}

ModuleHandle Provider::FindModule(std::string_view name) const noexcept {
	return _plugify.GetManager().FindModule(name);
}

std::shared_ptr<IAssemblyLoader> Provider::GetAssemblyLoader() const noexcept {
	return _plugify.GetAssemblyLoader();
}

std::shared_ptr<IFileSystem> Provider::GetFileSystem() const noexcept {
	return _plugify.GetFileSystem();
}
