#include <plugify/plugin_reference_descriptor.h>
#include <core/plugin_reference_descriptor.h>

using namespace plugify;

std::string_view PluginReferenceDescriptorRef::GetName() const noexcept {
	return _impl->name;
}

bool PluginReferenceDescriptorRef::IsOptional() const noexcept {
	return _impl->optional;
}

std::span<const std::string> PluginReferenceDescriptorRef::GetSupportedPlatforms() const noexcept {
	return _impl->supportedPlatforms;
}

std::optional<int32_t> PluginReferenceDescriptorRef::GetRequestedVersion() const noexcept {
	return _impl->requestedVersion;
}