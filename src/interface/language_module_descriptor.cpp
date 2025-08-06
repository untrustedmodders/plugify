#include <core/language_module_descriptor.hpp>
#include <plugify/language_module_descriptor.hpp>
#include <utils/pointer.hpp>

using namespace plugify;

int32_t LanguageModuleDescriptorHandle::GetFileVersion() const noexcept {
	return _impl->fileVersion;
}

plg::version LanguageModuleDescriptorHandle::GetVersion() const noexcept {
	return _impl->version;
}

std::string_view LanguageModuleDescriptorHandle::GetVersionName() const noexcept {
	return _impl->versionName;
}

std::string_view LanguageModuleDescriptorHandle::GetFriendlyName() const noexcept {
	return _impl->friendlyName;
}

std::string_view LanguageModuleDescriptorHandle::GetDescription() const noexcept {
	return _impl->description.has_value() ? *_impl->description : std::string_view{};
}

std::string_view LanguageModuleDescriptorHandle::GetCreatedBy() const noexcept {
	return _impl->createdBy.has_value() ? *_impl->createdBy : std::string_view{};
}

std::string_view LanguageModuleDescriptorHandle::GetCreatedByURL() const noexcept {
	return _impl->createdByURL.has_value() ? *_impl->createdByURL : std::string_view{};
}

std::string_view LanguageModuleDescriptorHandle::GetDocsURL() const noexcept {
	return _impl->docsURL.has_value() ? *_impl->docsURL : std::string_view{};
}

std::string_view LanguageModuleDescriptorHandle::GetDownloadURL() const noexcept {
	return _impl->downloadURL.has_value() ? *_impl->downloadURL : std::string_view{};
}

std::string_view LanguageModuleDescriptorHandle::GetUpdateURL() const noexcept {
	return _impl->updateURL.has_value() ? *_impl->updateURL : std::string_view{};
}

std::span<const std::string_view> LanguageModuleDescriptorHandle::GetSupportedPlatforms() const noexcept {
	if (!_impl->_supportedPlatforms) {
		_impl->_supportedPlatforms = make_shared_nothrow<std::vector<std::string_view>>(_impl->supportedPlatforms->begin(), _impl->supportedPlatforms->end());
	}
	if (_impl->_supportedPlatforms) {
		return *_impl->_supportedPlatforms;
	} else {
		return {};
	}
}

std::span<const std::string_view> LanguageModuleDescriptorHandle::GetLibraryDirectories() const noexcept {
	if (const auto& libraryDirectories = _impl->libraryDirectories) {
		if (!_impl->_libraryDirectories) {
			_impl->_libraryDirectories = make_shared_nothrow<std::vector<std::string_view>>(libraryDirectories->begin(), libraryDirectories->end());
		}
		if (_impl->_libraryDirectories) {
			return *_impl->_libraryDirectories;
		}
	}
	return {};
}

std::string_view LanguageModuleDescriptorHandle::GetLanguage() const noexcept {
	return _impl->language;
}

bool LanguageModuleDescriptorHandle::IsForceLoad() const noexcept {
	return _impl->forceLoad.value_or(false);
}
