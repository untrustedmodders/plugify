#include <core/plugin_reference_descriptor.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <utils/pointer.hpp>

using namespace plugify;

std::string_view PluginReferenceDescriptorHandle::GetName() const noexcept {
	return _impl->name;
}

bool PluginReferenceDescriptorHandle::IsOptional() const noexcept {
	return _impl->optional.value_or(false);
}

std::span<const std::string_view> PluginReferenceDescriptorHandle::GetSupportedPlatforms() const noexcept {
	if (const auto& supportedPlatforms = _impl->supportedPlatforms) {
		if (!_impl->_supportedPlatforms) {
			_impl->_supportedPlatforms = make_shared_nothrow<std::vector<std::string_view>>(supportedPlatforms->begin(), supportedPlatforms->end());
		}
		if (_impl->_supportedPlatforms) {
			return *_impl->_supportedPlatforms;
		}
	}
	return {};
}

std::optional<plg::version> PluginReferenceDescriptorHandle::GetVersion() const noexcept {
	return _impl->version;
}
