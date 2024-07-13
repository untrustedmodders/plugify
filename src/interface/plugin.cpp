#include <plugify/plugin.h>
#include <plugify/plugin_descriptor.h>
#include <core/plugin.h>

using namespace plugify;

UniqueId IPlugin::GetId() const noexcept {
	return _impl->GetId();
}

std::string_view IPlugin::GetName() const noexcept {
	return _impl->GetName();
}

std::string_view IPlugin::GetFriendlyName() const noexcept {
	return _impl->GetFriendlyName();
}

const fs::path& IPlugin::GetBaseDir() const noexcept {
	return _impl->GetBaseDir();
}

IPluginDescriptor IPlugin::GetDescriptor() const noexcept {
	return { _impl->GetDescriptor() };
}

PluginState IPlugin::GetState() const noexcept {
	return _impl->GetState();
}

std::string_view IPlugin::GetError() const noexcept {
	return _impl->GetError();
}

std::span<const MethodData> IPlugin::GetMethods() const noexcept {
	return _impl->GetMethods();
}

std::optional<fs::path> IPlugin::FindResource(const fs::path& path) const {
	return _impl->FindResource(path);
}