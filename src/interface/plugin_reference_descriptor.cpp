#include <core/plugin_reference_descriptor.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <utils/pointer.hpp>

using namespace plugify;

std::string_view PluginReferenceDescriptorRef::GetName() const noexcept {
	return _impl->name;
}

bool PluginReferenceDescriptorRef::IsOptional() const noexcept {
	return _impl->optional;
}

std::span<std::string_view> PluginReferenceDescriptorRef::GetSupportedPlatforms() const noexcept {
	if (!_impl->_supportedPlatforms) {
		_impl->_supportedPlatforms = make_shared_nothrow<std::vector<std::string_view>>(_impl->supportedPlatforms.begin(), _impl->supportedPlatforms.end());
	}
	if (_impl->_supportedPlatforms) {
		return *_impl->_supportedPlatforms;
	} else {
		return {};
	}
}

std::optional<int32_t> PluginReferenceDescriptorRef::GetRequestedVersion() const noexcept {
	return _impl->requestedVersion;
}
