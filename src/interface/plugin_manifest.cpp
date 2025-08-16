#include <core/plugin_manifest.hpp>
#include <plugify/api/dependency.hpp>
#include <plugify/api/method.hpp>
#include <plugify/api/plugin_manifest.hpp>
#include <util/pointer.hpp>

using namespace plugify;

std::string_view PluginManifestHandle::GetName() const noexcept {
	return _impl->name;
}

Version PluginManifestHandle::GetVersion() const noexcept {
	return _impl->version;
}

std::string_view PluginManifestHandle::GetDescription() const noexcept {
	return _impl->description.has_value() ? *_impl->description : std::string_view{};
}

std::string_view PluginManifestHandle::GetAuthor() const noexcept {
	return _impl->author.has_value() ? *_impl->author : std::string_view{};
}

std::string_view PluginManifestHandle::GetWebsite() const noexcept {
	return _impl->website.has_value() ? *_impl->website : std::string_view{};
}

std::string_view PluginManifestHandle::GetLicense() const noexcept {
	return _impl->license.has_value() ? *_impl->license : std::string_view{};
}

std::span<const std::string_view> PluginManifestHandle::GetPlatforms() const noexcept {
	if (const auto& platforms = _impl->platforms) {
		if (!_impl->_platforms) {
			_impl->_platforms = make_shared_nothrow<std::vector<std::string_view>>(platforms->begin(), platforms->end());
		}
		if (_impl->_platforms) {
			return *_impl->_platforms;
		}
	}
	return {};
}

std::string_view PluginManifestHandle::GetEntry() const noexcept {
	return _impl->entry;
}

std::string_view PluginManifestHandle::GetLanguage() const noexcept {
	return _impl->language.name;
}

std::span<const DependencyHandle> PluginManifestHandle::GetDependencies() const noexcept {
	if (const auto& dependencies = _impl->dependencies) {
		static_assert(sizeof(std::unique_ptr<Dependency>) == sizeof(DependencyHandle), "Unique ptr and handle must have the same size");
		return { reinterpret_cast<const DependencyHandle*>(dependencies->data()), dependencies->size() };
	}
	return {};
}

std::span<const MethodHandle> PluginManifestHandle::GetMethods() const noexcept {
	if (const auto& methods = _impl->methods) {
		static_assert(sizeof(std::unique_ptr<Method>) == sizeof(MethodHandle), "Unique ptr and handle must have the same size");
		return { reinterpret_cast<const MethodHandle*>(methods->data()), methods->size() };
	}
	return {};
}
