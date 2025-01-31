#include <core/plugin.hpp>
#include <plugify/plugin.hpp>
#include <plugify/plugin_descriptor.hpp>

using namespace plugify;

UniqueId PluginHandle::GetId() const noexcept {
	return _impl->GetId();
}

std::string_view PluginHandle::GetName() const noexcept {
	return _impl->GetName();
}

std::string_view PluginHandle::GetFriendlyName() const noexcept {
	return _impl->GetFriendlyName();
}

fs::path_view PluginHandle::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().native();
}

PluginDescriptorHandle PluginHandle::GetDescriptor() const noexcept {
	return { _impl->GetDescriptor() };
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

std::optional<fs::path_view> PluginHandle::FindResource(fs::path_view path) const {
	return _impl->FindResource(path);
}

#if PLUGIFY_INTERFACE
std::optional<fs::path_view> Plugin::FindResource(const fs::path&) const {
	return {};
}
#endif // PLUGIFY_INTERFACE