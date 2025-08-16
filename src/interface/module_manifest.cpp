#include <core/module_manifest.hpp>
#include <plugify/api/module_manifest.hpp>
#include <util/pointer.hpp>

using namespace plugify;

std::string_view ModuleManifestHandle::GetName() const noexcept {
	return _impl->name;
}

Version ModuleManifestHandle::GetVersion() const noexcept {
	return _impl->version;
}

std::string_view ModuleManifestHandle::GetDescription() const noexcept {
	return _impl->description.has_value() ? *_impl->description : std::string_view{};
}

std::string_view ModuleManifestHandle::GetAuthor() const noexcept {
	return _impl->author.has_value() ? *_impl->author : std::string_view{};
}

std::string_view ModuleManifestHandle::GetWebsite() const noexcept {
	return _impl->website.has_value() ? *_impl->website : std::string_view{};
}

std::string_view ModuleManifestHandle::GetLicense() const noexcept {
	return _impl->license.has_value() ? *_impl->license : std::string_view{};
}

std::span<const std::string_view> ModuleManifestHandle::GetPlatforms() const noexcept {
	if (!_impl->_platforms) {
		_impl->_platforms = make_shared_nothrow<std::vector<std::string_view>>(_impl->platforms->begin(), _impl->platforms->end());
	}
	if (_impl->_platforms) {
		return *_impl->_platforms;
	}
	return {};
}

std::span<const std::string_view> ModuleManifestHandle::GetDirectories() const noexcept {
	if (const auto& directories = _impl->directories) {
		if (!_impl->_directories) {
			_impl->_directories = make_shared_nothrow<std::vector<std::string_view>>(directories->begin(), directories->end());
		}
		if (_impl->_directories) {
			return *_impl->_directories;
		}
	}
	return {};
}

std::string_view ModuleManifestHandle::GetLanguage() const noexcept {
	return _impl->language;
}

bool ModuleManifestHandle::IsForceLoad() const noexcept {
	return _impl->forceLoad.value_or(false);
}
