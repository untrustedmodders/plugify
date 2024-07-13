#include <plugify/plugin_reference_descriptor.h>
#include <core/plugin_reference_descriptor.h>

using namespace plugify;

std::string_view IPluginReferenceDescriptor::GetName() const noexcept {
	return _impl->name;
}

bool IPluginReferenceDescriptor::IsOptional() const noexcept {
	return _impl->optional;
}

std::vector<std::string_view> IPluginReferenceDescriptor::GetSupportedPlatforms() const {
	return { _impl->supportedPlatforms.begin(), _impl->supportedPlatforms.end() };
}

std::optional<int32_t> IPluginReferenceDescriptor::GetRequestedVersion() const noexcept {
	return _impl->requestedVersion;
}