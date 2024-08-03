#include <plugify/plugin.h>
#include <plugify/plugin_descriptor.h>
#include <core/plugin.h>

using namespace plugify;

UniqueId PluginRef::GetId() const noexcept {
	return _impl->GetId();
}

const std::string& PluginRef::GetName() const noexcept {
	return _impl->GetName();
}

const std::string& PluginRef::GetFriendlyName() const noexcept {
	return _impl->GetFriendlyName();
}

const fs::path& PluginRef::GetBaseDir() const noexcept {
	return _impl->GetBaseDir();
}

PluginDescriptorRef PluginRef::GetDescriptor() const noexcept {
	return { _impl->GetDescriptor() };
}

PluginState PluginRef::GetState() const noexcept {
	return _impl->GetState();
}

const std::string& PluginRef::GetError() const noexcept {
	return _impl->GetError();
}

std::span<const MethodData> PluginRef::GetMethods() const noexcept {
	return _impl->GetMethods();
}

std::optional<fs::path> PluginRef::FindResource(const fs::path& path) const {
	return _impl->FindResource(path);
}