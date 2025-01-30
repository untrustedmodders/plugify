#include <core/plugify_provider.hpp>
#include <plugify/plugin.hpp>
#include <plugify/module.hpp>
#include <plugify/plugify_provider.hpp>

using namespace plugify;

void ProviderHandle::Log(std::string_view msg, Severity severity) const {
	_impl->Log(msg, severity);
}

fs::path_view ProviderHandle::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().native();
}

bool ProviderHandle::IsPreferOwnSymbols() const noexcept {
	return _impl->IsPreferOwnSymbols();
}

bool ProviderHandle::IsPluginLoaded(std::string_view name, std::optional<int32_t> requiredVersion, bool minimum) const noexcept {
	return _impl->IsPluginLoaded(name, requiredVersion, minimum);
}

bool ProviderHandle::IsModuleLoaded(std::string_view name, std::optional<int32_t> requiredVersion, bool minimum) const noexcept {
	return _impl->IsModuleLoaded(name, requiredVersion, minimum);
}

PluginHandle ProviderHandle::FindPlugin(std::string_view name) const noexcept {
	return _impl->FindPlugin(name);
}

ModuleHandle ProviderHandle::FindModule(std::string_view name) const noexcept {
	return _impl->FindModule(name);
}