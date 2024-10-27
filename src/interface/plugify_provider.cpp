#include <core/plugify_provider.hpp>
#include <plugify/plugify_provider.hpp>

using namespace plugify;

void IPlugifyProvider::Log(std::string_view msg, Severity severity) const {
	_impl->Log(msg, severity);
}

fs::path_view IPlugifyProvider::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().c_str();
}

bool IPlugifyProvider::IsPreferOwnSymbols() const noexcept {
	return _impl->IsPreferOwnSymbols();
}

bool IPlugifyProvider::IsPluginLoaded(std::string_view name, std::optional<int32_t> requiredVersion, bool minimum) const noexcept {
	return _impl->IsPluginLoaded(name, requiredVersion, minimum);
}

bool IPlugifyProvider::IsModuleLoaded(std::string_view name, std::optional<int32_t> requiredVersion, bool minimum) const noexcept {
	return _impl->IsModuleLoaded(name, requiredVersion, minimum);
}
