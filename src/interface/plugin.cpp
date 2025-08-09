#include <core/plugin.hpp>
#include <plugify/api/plugin.hpp>
#include <plugify/api/plugin_manifest.hpp>

using namespace plugify;

UniqueId PluginHandle::GetId() const noexcept {
	return _impl->GetId();
}

std::string_view PluginHandle::GetName() const noexcept {
	return _impl->GetName();
}

fs::path_view PluginHandle::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().native();
}

fs::path_view PluginHandle::GetConfigsDir() const noexcept {
	return _impl->GetConfigsDir().native();
}

fs::path_view PluginHandle::GetDataDir() const noexcept {
	return _impl->GetDataDir().native();
}

fs::path_view PluginHandle::GetLogsDir() const noexcept {
	return _impl->GetLogsDir().native();
}

PluginManifestHandle PluginHandle::GetManifest() const noexcept {
	return { _impl->GetManifest() };
}

PluginState PluginHandle::GetState() const noexcept {
	return _impl->GetState();
}

std::string_view PluginHandle::GetError() const noexcept {
	return _impl->GetError();
}

std::span<const MethodData> PluginHandle::GetMethods() const noexcept {
	return _impl->GetMethods();
}

MemAddr PluginHandle::GetData() const noexcept {
	return _impl->GetData();
}
