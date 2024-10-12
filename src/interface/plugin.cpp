#include <plugify/plugin.h>
#include <plugify/plugin_descriptor.h>
#include <core/plugin.h>

using namespace plugify;

UniqueId PluginRef::GetId() const noexcept {
	return _impl->GetId();
}

std::string_view PluginRef::GetName() const noexcept {
	return _impl->GetName();
}

std::string_view PluginRef::GetFriendlyName() const noexcept {
	return _impl->GetFriendlyName();
}

fs::path_view PluginRef::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().c_str();
}

PluginDescriptorRef PluginRef::GetDescriptor() const noexcept {
	return { _impl->GetDescriptor() };
}

PluginState PluginRef::GetState() const noexcept {
	return _impl->GetState();
}

std::string_view PluginRef::GetError() const noexcept {
	return _impl->GetError();
}

std::span<const MethodData> PluginRef::GetMethods() const noexcept {
	return _impl->GetMethods();
}

std::optional<fs::path_view> PluginRef::FindResource(fs::path_view path) const {
	return _impl->FindResource(path);
}
