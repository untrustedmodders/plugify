#include <plugify/plugify_provider.h>
#include <core/plugify_provider.h>

using namespace plugify;

IPlugifyProvider::IPlugifyProvider(PlugifyProvider& impl) : _impl{impl} {
}

IPlugifyProvider::~IPlugifyProvider() = default;

void IPlugifyProvider::Log(const std::string& msg, Severity severity) const {
	_impl.Log(msg, severity);
}

const fs::path& IPlugifyProvider::GetBaseDir() const {
	return _impl.GetBaseDir();
}

bool IPlugifyProvider::IsPreferOwnSymbols() const {
	return _impl.IsPreferOwnSymbols();
}

bool IPlugifyProvider::IsPluginLoaded(const std::string& name, std::optional<int32_t> requiredVersion, bool minimum) const {
	return _impl.IsPluginLoaded(name, requiredVersion, minimum);
}

bool IPlugifyProvider::IsModuleLoaded(const std::string& name, std::optional<int32_t> requiredVersion, bool minimum) const {
	return _impl.IsModuleLoaded(name, requiredVersion, minimum);
}
