#include <core/language_module_descriptor.h>
#include <plugify/language_module_descriptor.h>
#include <utils/pointer.h>

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

std::span<std::string_view> LanguageModuleDescriptorRef::GetSupportedPlatforms() const noexcept {
	if (!_impl->_supportedPlatforms) {
		_impl->_supportedPlatforms = make_shared_nothrow<std::vector<std::string_view>>(_impl->supportedPlatforms.begin(), _impl->supportedPlatforms.end());
	}
	if (_impl->_supportedPlatforms) {
		return *_impl->_supportedPlatforms;
	} else {
		return {};
	}
}

std::span<std::string_view> LanguageModuleDescriptorRef::GetResourceDirectories() const noexcept {
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

std::span<std::string_view> LanguageModuleDescriptorRef::GetLibraryDirectories() const noexcept {
	if (_impl->libraryDirectories.has_value()) {
		if (!_impl->_libraryDirectories) {
			_impl->_libraryDirectories = make_shared_nothrow<std::vector<std::string_view>>(_impl->libraryDirectories->begin(), _impl->libraryDirectories->end());
		}
		if (_impl->_libraryDirectories) {
			return *_impl->_libraryDirectories;
		} else {
			return {};
		}
	} else {
		return {};
	}
}

std::string_view LanguageModuleDescriptorRef::GetLanguage() const noexcept {
	return _impl->language;
}

bool LanguageModuleDescriptorRef::IsForceLoad() const noexcept {
	return _impl->forceLoad;
}
