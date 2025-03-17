#include <core/plugify_provider.hpp>
#include <plugify/plugin.hpp>
#include <plugify/module.hpp>
#include <plugify/plugify_provider.hpp>

using namespace plugify;

void IPlugifyProvider::Log(std::string_view msg, Severity severity) const {
	_impl->Log(msg, severity);
}

fs::path_view IPlugifyProvider::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().native();
}

fs::path_view IPlugifyProvider::GetConfigsDir() const noexcept {
	return _impl->GetConfigsDir().native();
}

fs::path_view IPlugifyProvider::GetDataDir() const noexcept {
	return _impl->GetDataDir().native();
}

fs::path_view IPlugifyProvider::GetLogsDir() const noexcept {
	return _impl->GetLogsDir().native();
}

bool IPlugifyProvider::IsPreferOwnSymbols() const noexcept {
	return _impl->IsPreferOwnSymbols();
}

bool IPlugifyProvider::IsPluginLoaded(std::string_view name, std::optional<plg::version> requiredVersion, bool minimum) const noexcept {
	return _impl->IsPluginLoaded(name, requiredVersion, minimum);
}

bool IPlugifyProvider::IsModuleLoaded(std::string_view name, std::optional<plg::version> requiredVersion, bool minimum) const noexcept {
	return _impl->IsModuleLoaded(name, requiredVersion, minimum);
}

PluginHandle IPlugifyProvider::FindPlugin(std::string_view name) const noexcept {
	return _impl->FindPlugin(name);
}

ModuleHandle IPlugifyProvider::FindModule(std::string_view name) const noexcept {
	return _impl->FindModule(name);
}