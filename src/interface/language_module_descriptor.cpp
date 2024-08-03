#include <plugify/language_module_descriptor.h>
#include <core/language_module_descriptor.h>

using namespace plugify;

int32_t LanguageModuleDescriptorRef::GetFileVersion() const noexcept {
	return _impl->fileVersion;
}

int32_t LanguageModuleDescriptorRef::GetVersion() const noexcept {
	return _impl->version;
}

const std::string& LanguageModuleDescriptorRef::GetVersionName() const noexcept {
	return _impl->versionName;
}

const std::string& LanguageModuleDescriptorRef::GetFriendlyName() const noexcept {
	return _impl->friendlyName;
}

const std::string& LanguageModuleDescriptorRef::GetDescription() const noexcept {
	return _impl->description;
}

const std::string& LanguageModuleDescriptorRef::GetCreatedBy() const noexcept {
	return _impl->createdBy;
}

const std::string& LanguageModuleDescriptorRef::GetCreatedByURL() const noexcept {
	return _impl->createdByURL;
}

const std::string& LanguageModuleDescriptorRef::GetDocsURL() const noexcept {
	return _impl->docsURL;
}

const std::string& LanguageModuleDescriptorRef::GetDownloadURL() const noexcept {
	return _impl->downloadURL;
}

const std::string& LanguageModuleDescriptorRef::GetUpdateURL() const noexcept {
	return _impl->updateURL;
}

std::span<const std::string> LanguageModuleDescriptorRef::GetSupportedPlatforms() const noexcept {
	return _impl->supportedPlatforms;
}

std::span<const std::string> LanguageModuleDescriptorRef::GetResourceDirectories() const noexcept {
	if (_impl->resourceDirectories.has_value()) {
		return *_impl->resourceDirectories;
	} else {
		return {};
	}
}

std::span<const std::string> LanguageModuleDescriptorRef::GetLibraryDirectories() const noexcept {
	if (_impl->libraryDirectories.has_value()) {
		return *_impl->libraryDirectories;
	} else {
		return {};
	}
}

const std::string& LanguageModuleDescriptorRef::GetLanguage() const noexcept {
	return _impl->language;
}

bool LanguageModuleDescriptorRef::IsForceLoad() const noexcept {
	return _impl->forceLoad;
}