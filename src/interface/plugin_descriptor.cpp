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

const std::string& PluginDescriptorRef::GetVersionName() const noexcept {
	return _impl->versionName;
}

const std::string& PluginDescriptorRef::GetFriendlyName() const noexcept {
	return _impl->friendlyName;
}

const std::string& PluginDescriptorRef::GetDescription() const noexcept {
	return _impl->description;
}

const std::string& PluginDescriptorRef::GetCreatedBy() const noexcept {
	return _impl->createdBy;
}

const std::string& PluginDescriptorRef::GetCreatedByURL() const noexcept {
	return _impl->createdByURL;
}

const std::string& PluginDescriptorRef::GetDocsURL() const noexcept {
	return _impl->docsURL;
}

const std::string& PluginDescriptorRef::GetDownloadURL() const noexcept {
	return _impl->downloadURL;
}

const std::string& PluginDescriptorRef::GetUpdateURL() const noexcept {
	return _impl->updateURL;
}

std::span<const std::string> PluginDescriptorRef::GetSupportedPlatforms() const noexcept {
	return _impl->supportedPlatforms;
}

std::span<const std::string> PluginDescriptorRef::GetResourceDirectories() const noexcept {
	if (_impl->resourceDirectories.has_value()) {
		return *_impl->resourceDirectories;
	} else {
		return {};
	}
}

const std::string& PluginDescriptorRef::GetEntryPoint() const noexcept {
	return _impl->entryPoint;
}

const std::string& PluginDescriptorRef::GetLanguageModule() const noexcept {
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