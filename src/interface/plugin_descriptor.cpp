#include <plugify/plugin_descriptor.h>
#include <plugify/plugin_reference_descriptor.h>
#include <plugify/method.h>
#include <core/plugin_descriptor.h>

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

std::span<const std::string> PluginDescriptorRef::GetSupportedPlatforms() const {
	return _impl->supportedPlatforms;
}

std::span<const std::string> PluginDescriptorRef::GetResourceDirectories() const {
	if (_impl->resourceDirectories.has_value()) {
		return *_impl->resourceDirectories;
	}
	return {};
}

std::string_view PluginDescriptorRef::GetEntryPoint() const noexcept {
	return _impl->entryPoint;
}

std::string_view PluginDescriptorRef::GetLanguageModule() const noexcept {
	return _impl->languageModule.name;
}

std::span<const PluginReferenceDescriptorRef> PluginDescriptorRef::GetDependencies() const {
	if (!_impl->_dependencies) {
		_impl->_dependencies = std::make_shared<std::vector<PluginReferenceDescriptorRef>>(_impl->dependencies.begin(), _impl->dependencies.end());
	}
	return *_impl->_dependencies;
}

std::span<const MethodRef> PluginDescriptorRef::GetExportedMethods() const {
	if (!_impl->_exportedMethods) {
		_impl->_exportedMethods = std::make_shared<std::vector<MethodRef>>(_impl->exportedMethods.begin(), _impl->exportedMethods.end());
	}
	return *_impl->_exportedMethods;
}