#include <plugify/language_module_descriptor.h>
#include <core/language_module_descriptor.h>

using namespace plugify;

int32_t LanguageModuleDescriptorRef::GetFileVersion() const noexcept {
	return _impl->fileVersion;
}

int32_t LanguageModuleDescriptorRef::GetVersion() const noexcept {
	return _impl->version;
}

std::string_view LanguageModuleDescriptorRef::GetVersionName() const noexcept {
	return _impl->versionName;
}

std::string_view LanguageModuleDescriptorRef::GetFriendlyName() const noexcept {
	return _impl->friendlyName;
}

std::string_view LanguageModuleDescriptorRef::GetDescription() const noexcept {
	return _impl->description;
}

std::string_view LanguageModuleDescriptorRef::GetCreatedBy() const noexcept {
	return _impl->createdBy;
}

std::string_view LanguageModuleDescriptorRef::GetCreatedByURL() const noexcept {
	return _impl->createdByURL;
}

std::string_view LanguageModuleDescriptorRef::GetDocsURL() const noexcept {
	return _impl->docsURL;
}

std::string_view LanguageModuleDescriptorRef::GetDownloadURL() const noexcept {
	return _impl->downloadURL;
}

std::string_view LanguageModuleDescriptorRef::GetUpdateURL() const noexcept {
	return _impl->updateURL;
}

std::vector<std::string_view> LanguageModuleDescriptorRef::GetSupportedPlatforms() const {
	return { _impl->supportedPlatforms.begin(), _impl->supportedPlatforms.end() };
}

std::vector<std::string_view> LanguageModuleDescriptorRef::GetResourceDirectories() const {
	if (_impl->resourceDirectories.has_value()) {
		return { _impl->resourceDirectories->begin(), _impl->resourceDirectories->end() };
	}
	return {};
}

std::vector<std::string_view> LanguageModuleDescriptorRef::GetLibraryDirectories() const {
	if (_impl->libraryDirectories.has_value()) {
		return { _impl->libraryDirectories->begin(), _impl->libraryDirectories->end() };
	}
	return {};
}

std::string_view LanguageModuleDescriptorRef::GetLanguage() const noexcept {
	return _impl->language;
}

bool LanguageModuleDescriptorRef::IsForceLoad() const noexcept {
	return _impl->forceLoad;
}