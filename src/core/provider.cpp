#include "plugify/core/plugify.hpp"
#include "plugin_manifest.hpp"

#include "plugify/_/module_handle.hpp"
#include "plugify/_/plugin_manifest_handle.hpp"
#include "plugify/core/manager.hpp"
#include "plugify/core/module_manifest_handle.hpp"
#include "plugify/core/plugin_handle.hpp"
#include "plugify/core/provider.hpp"

using namespace plugify;

class Provider::Impl {
	explicit Impl(Plugify& self) : plugify(self) {}
	Plugify& plugify;
};

Provider::Provider(Plugify& plugify) : _impl(std::make_unique<Impl>(plugify)) {}

Provider::~Provider() = default;

void Provider::Log(std::string_view msg, Severity severity) const {
	_impl->plugify.Log(msg, severity);
}

const std::filesystem::path& Provider::GetBaseDir() const noexcept {
	return _impl->plugify.GetConfig().baseDir;
}

const std::filesystem::path& Provider::GetConfigsDir() const noexcept {
	return _impl->plugify.GetConfig().configsDir;
}

const std::filesystem::path& Provider::GetDataDir() const noexcept {
	return _impl->plugify.GetConfig().dataDir;
}

const std::filesystem::path& Provider::GetLogsDir() const noexcept {
	return _impl->plugify.GetConfig().logsDir;
}

bool Provider::IsPreferOwnSymbols() const noexcept {
	return _impl->plugify.GetConfig().preferOwnSymbols.value_or(false);
}

bool Provider::IsPluginLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
	auto plugin = _impl->plugify.GetManager().FindPlugin(name);
	if (!plugin)
		return false;
	if (plugin.GetState() != PluginState::Loaded && plugin.GetState() != PluginState::Running)
		return false;
	if (constraint)
		return constraint->IsSatisfiedBy(plugin.GetManifest().GetVersion());
	return true;
}

bool Provider::IsModuleLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
	auto module = _impl->plugify.GetManager().FindModule(name);
	if (!module)
		return false;
	if (module.GetState() != ModuleState::Loaded)
		return false;
	if (constraint)
		return constraint->IsSatisfiedBy(module.GetManifest().GetVersion());
	return true;
}

PluginHandle Provider::FindPlugin(std::string_view name) const noexcept {
	return _impl->plugify.GetManager().FindPlugin(name);
}

ModuleHandle Provider::FindModule(std::string_view name) const noexcept {
	return _impl->plugify.GetManager().FindModule(name);
}

std::shared_ptr<IAssemblyLoader> Provider::GetAssemblyLoader() const noexcept {
	return _impl->plugify.GetAssemblyLoader();
}

std::shared_ptr<IFileSystem> Provider::GetFileSystem() const noexcept {
	return _impl->plugify.GetFileSystem();
}

