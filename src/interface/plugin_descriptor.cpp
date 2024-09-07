#include <plugify/method.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin_reference_descriptor.h>
#include <core/plugin_descriptor.h>
#include <utils/pointer.h>

using namespace plugify;

int32_t PluginDescriptorRef::GetFileVersion() const noexcept {
	return _impl->fileVersion;
}

int32_t PluginDescriptorRef::GetVersion() const noexcept {
	return _impl->version;
}

std::string_view PluginDescriptorRef::GetVersionName() const noexcept {
	return _impl->versionName;
}

std::string_view PluginDescriptorRef::GetFriendlyName() const noexcept {
	return _impl->friendlyName;
}

std::string_view PluginDescriptorRef::GetDescription() const noexcept {
	return _impl->description;
}

std::string_view PluginDescriptorRef::GetCreatedBy() const noexcept {
	return _impl->createdBy;
}

std::string_view PluginDescriptorRef::GetCreatedByURL() const noexcept {
	return _impl->createdByURL;
}

std::string_view PluginDescriptorRef::GetDocsURL() const noexcept {
	return _impl->docsURL;
}

std::string_view PluginDescriptorRef::GetDownloadURL() const noexcept {
	return _impl->downloadURL;
}

std::string_view PluginDescriptorRef::GetUpdateURL() const noexcept {
	return _impl->updateURL;
}

std::span<std::string_view> PluginDescriptorRef::GetSupportedPlatforms() const noexcept {
	if (!_impl->_supportedPlatforms) {
		_impl->_supportedPlatforms = make_shared_nothrow<std::vector<std::string_view>>(_impl->supportedPlatforms.begin(), _impl->supportedPlatforms.end());
	}
	if (_impl->_supportedPlatforms) {
		return *_impl->_supportedPlatforms;
	} else {
		return {};
	}
}

std::span<std::string_view> PluginDescriptorRef::GetResourceDirectories() const noexcept {
	if (_impl->resourceDirectories.has_value()) {
		if (!_impl->_resourceDirectories) {
			_impl->_resourceDirectories = make_shared_nothrow<std::vector<std::string_view>>(_impl->resourceDirectories->begin(), _impl->resourceDirectories->end());
		}
		if (_impl->_resourceDirectories) {
			return *_impl->_resourceDirectories;
		} else {
			return {};
		}
	} else {
		return {};
	}
}

std::string_view PluginDescriptorRef::GetEntryPoint() const noexcept {
	return _impl->entryPoint;
}

std::string_view PluginDescriptorRef::GetLanguageModule() const noexcept {
	return _impl->languageModule.name;
}

std::span<const PluginReferenceDescriptorRef> PluginDescriptorRef::GetDependencies() const noexcept {
	if (!_impl->_dependencies) {
		_impl->_dependencies = make_shared_nothrow<std::vector<PluginReferenceDescriptorRef>>(_impl->dependencies.begin(), _impl->dependencies.end());
	}
	if (_impl->_dependencies) {
		return *_impl->_dependencies;
	} else {
		return {};
	}
}

std::span<const MethodRef> PluginDescriptorRef::GetExportedMethods() const noexcept {
	if (!_impl->_exportedMethods) {
		_impl->_exportedMethods = make_shared_nothrow<std::vector<MethodRef>>(_impl->exportedMethods.begin(), _impl->exportedMethods.end());
	}
	if (_impl->_exportedMethods) {
		return *_impl->_exportedMethods;
	} else {
		return {};
	}
}