#include <core/plugin_descriptor.hpp>
#include <plugify/method.hpp>
#include <plugify/plugin_descriptor.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <utils/pointer.hpp>

using namespace plugify;

int32_t PluginDescriptorHandle::GetFileVersion() const noexcept {
	return _impl->fileVersion;
}

plg::version PluginDescriptorHandle::GetVersion() const noexcept {
	return _impl->version;
}

std::string_view PluginDescriptorHandle::GetVersionName() const noexcept {
	return _impl->versionName;
}

std::string_view PluginDescriptorHandle::GetFriendlyName() const noexcept {
	return _impl->friendlyName;
}

std::string_view PluginDescriptorHandle::GetDescription() const noexcept {
	return _impl->description.has_value() ? *_impl->description : std::string_view{};
}

std::string_view PluginDescriptorHandle::GetCreatedBy() const noexcept {
	return _impl->createdBy.has_value() ? *_impl->createdBy : std::string_view{};
}

std::string_view PluginDescriptorHandle::GetCreatedByURL() const noexcept {
	return _impl->createdByURL.has_value() ? *_impl->createdByURL : std::string_view{};
}

std::string_view PluginDescriptorHandle::GetDocsURL() const noexcept {
	return _impl->docsURL.has_value() ? *_impl->docsURL : std::string_view{};
}

std::string_view PluginDescriptorHandle::GetDownloadURL() const noexcept {
	return _impl->downloadURL.has_value() ? *_impl->downloadURL : std::string_view{};
}

std::string_view PluginDescriptorHandle::GetUpdateURL() const noexcept {
	return _impl->updateURL.has_value() ? *_impl->updateURL : std::string_view{};
}

std::span<const std::string_view> PluginDescriptorHandle::GetSupportedPlatforms() const noexcept {
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

std::string_view PluginDescriptorHandle::GetEntryPoint() const noexcept {
	return _impl->entryPoint;
}

std::string_view PluginDescriptorHandle::GetLanguageModule() const noexcept {
	return _impl->languageModule.name;
}

std::span<const PluginReferenceDescriptorHandle> PluginDescriptorHandle::GetDependencies() const noexcept {
	if (const auto& dependencies = _impl->dependencies) {
		if (!_impl->_dependencies) {
			_impl->_dependencies = make_shared_nothrow<std::vector<PluginReferenceDescriptorHandle>>(dependencies->begin(), dependencies->end());
		}
		if (_impl->_dependencies) {
			return *_impl->_dependencies;
		}
	}
	return {};
}

std::span<const MethodHandle> PluginDescriptorHandle::GetExportedMethods() const noexcept {
	if (const auto& exportedMethods = _impl->exportedMethods) {
		if (!_impl->_exportedMethods) {
			_impl->_exportedMethods = make_shared_nothrow<std::vector<MethodHandle>>(exportedMethods->begin(), exportedMethods->end());
		}
		if (_impl->_exportedMethods) {
			return *_impl->_exportedMethods;
		}
	}
	return {};
}
