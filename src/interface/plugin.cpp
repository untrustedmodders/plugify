#include <core/plugin.hpp>
#include <plugify/api/plugin.hpp>
#include <plugify/api/plugin_manifest.hpp>

using namespace plugify;

UniqueId PluginHandle::GetId() const noexcept {
	return _impl->GetId();
}

const std::string& PluginHandle::GetName() const noexcept {
	return _impl->GetName();
}

const std::filesystem::path& PluginHandle::GetBaseDir() const noexcept {
	return _impl->GetBaseDir();
}

const std::filesystem::path& PluginHandle::GetConfigsDir() const noexcept {
	return _impl->GetConfigsDir();
}

const std::filesystem::path& PluginHandle::GetDataDir() const noexcept {
	return _impl->GetDataDir();
}

const std::filesystem::path& PluginHandle::GetLogsDir() const noexcept {
	return _impl->GetLogsDir();
}

PluginManifestHandle PluginHandle::GetManifest() const noexcept {
	return _impl->GetManifest();
}

PluginState PluginHandle::GetState() const noexcept {
	return _impl->GetState();
}

const std::string& PluginHandle::GetError() const noexcept {
	return _impl->GetError();
}

std::span<const MethodData> PluginHandle::GetMethods() const noexcept {
	return _impl->GetMethods();
}

MemAddr PluginHandle::GetData() const noexcept {
	return _impl->GetData();
}
