#include <plugify/plugin_descriptor.h>
#include <plugify/plugin_reference_descriptor.h>
#include <plugify/method.h>
#include <core/plugin_descriptor.h>

using namespace plugify;

int32_t IPluginDescriptor::GetFileVersion() const noexcept {
	return _impl->fileVersion;
}

int32_t IPluginDescriptor::GetVersion() const noexcept {
	return _impl->version;
}

std::string_view IPluginDescriptor::GetVersionName() const noexcept {
	return _impl->versionName;
}

std::string_view IPluginDescriptor::GetFriendlyName() const noexcept {
	return _impl->friendlyName;
}

std::string_view IPluginDescriptor::GetDescription() const noexcept {
	return _impl->description;
}

std::string_view IPluginDescriptor::GetCreatedBy() const noexcept {
	return _impl->createdBy;
}

std::string_view IPluginDescriptor::GetCreatedByURL() const noexcept {
	return _impl->createdByURL;
}

std::string_view IPluginDescriptor::GetDocsURL() const noexcept {
	return _impl->docsURL;
}

std::string_view IPluginDescriptor::GetDownloadURL() const noexcept {
	return _impl->downloadURL;
}

std::string_view IPluginDescriptor::GetUpdateURL() const noexcept {
	return _impl->updateURL;
}

std::vector<std::string_view> IPluginDescriptor::GetSupportedPlatforms() const {
	return { _impl->supportedPlatforms.begin(), _impl->supportedPlatforms.end() };
}

std::vector<std::string_view> IPluginDescriptor::GetResourceDirectories() const {
	if (_impl->resourceDirectories.has_value()) {
		return { _impl->resourceDirectories->begin(), _impl->resourceDirectories->end() };
	}
	return {};
}

std::string_view IPluginDescriptor::GetEntryPoint() const noexcept {
	return _impl->entryPoint;
}

std::string_view IPluginDescriptor::GetLanguageModule() const noexcept {
	return _impl->languageModule.name;
}

std::vector<IPluginReferenceDescriptor> IPluginDescriptor::GetDependencies() const {
	return { _impl->dependencies.begin(), _impl->dependencies.end() };
}

std::vector<IMethod> IPluginDescriptor::GetExportedMethods() const {
	return { _impl->exportedMethods.begin(), _impl->exportedMethods.end() };
}