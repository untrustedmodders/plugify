#include <plugify/plugin_reference_descriptor.h>
#include <core/plugin_reference_descriptor.h>

using namespace plugify;

std::string_view PluginReferenceDescriptorRef::GetName() const noexcept {
	return _impl->name;
}

bool PluginReferenceDescriptorRef::IsOptional() const noexcept {
	return _impl->optional;
}

std::vector<std::string_view> PluginReferenceDescriptorRef::GetSupportedPlatforms() const {
	return { _impl->supportedPlatforms.begin(), _impl->supportedPlatforms.end() };
}

std::optional<int32_t> PluginReferenceDescriptorRef::GetRequestedVersion() const noexcept {
	return _impl->requestedVersion;
}