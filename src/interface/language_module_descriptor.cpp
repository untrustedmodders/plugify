#include <plugify/language_module_descriptor.h>
#include <core/language_module_descriptor.h>

using namespace plugify;

int32_t ILanguageModuleDescriptor::GetFileVersion() const noexcept {
	return _impl->fileVersion;
}

int32_t ILanguageModuleDescriptor::GetVersion() const noexcept {
	return _impl->version;
}

std::string_view ILanguageModuleDescriptor::GetVersionName() const noexcept {
	return _impl->versionName;
}

std::string_view ILanguageModuleDescriptor::GetFriendlyName() const noexcept {
	return _impl->friendlyName;
}

std::string_view ILanguageModuleDescriptor::GetDescription() const noexcept {
	return _impl->description;
}

std::string_view ILanguageModuleDescriptor::GetCreatedBy() const noexcept {
	return _impl->createdBy;
}

std::string_view ILanguageModuleDescriptor::GetCreatedByURL() const noexcept {
	return _impl->createdByURL;
}

std::string_view ILanguageModuleDescriptor::GetDocsURL() const noexcept {
	return _impl->docsURL;
}

std::string_view ILanguageModuleDescriptor::GetDownloadURL() const noexcept {
	return _impl->downloadURL;
}

std::string_view ILanguageModuleDescriptor::GetUpdateURL() const noexcept {
	return _impl->updateURL;
}

std::vector<std::string_view> ILanguageModuleDescriptor::GetSupportedPlatforms() const {
	return { _impl->supportedPlatforms.begin(), _impl->supportedPlatforms.end() };
}

std::vector<std::string_view> ILanguageModuleDescriptor::GetResourceDirectories() const {
	if (_impl->resourceDirectories.has_value()) {
		return { _impl->resourceDirectories->begin(), _impl->resourceDirectories->end() };
	}
	return {};
}

std::vector<std::string_view> ILanguageModuleDescriptor::GetLibraryDirectories() const {
	if (_impl->libraryDirectories.has_value()) {
		return { _impl->libraryDirectories->begin(), _impl->libraryDirectories->end() };
	}
	return {};
}

std::string_view ILanguageModuleDescriptor::GetLanguage() const noexcept {
	return _impl->language;
}

bool ILanguageModuleDescriptor::IsForceLoad() const noexcept {
	return _impl->forceLoad;
}