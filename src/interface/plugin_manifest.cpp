#include <core/plugin_manifest.hpp>
#include <plugify/api/dependency.hpp>
#include <plugify/api/conflict.hpp>
#include <plugify/api/method.hpp>
#include <plugify/api/plugin_manifest.hpp>

using namespace plugify;

const std::string& PluginManifestHandle::GetName() const noexcept {
	return _impl->name;
}

const Version& PluginManifestHandle::GetVersion() const noexcept {
	return _impl->version;
}

const std::string& PluginManifestHandle::GetDescription() const noexcept {
	return _impl->description;
}

const std::string& PluginManifestHandle::GetAuthor() const noexcept {
	return _impl->author;
}

const std::string& PluginManifestHandle::GetWebsite() const noexcept {
	return _impl->website;
}

const std::string& PluginManifestHandle::GetLicense() const noexcept {
	return _impl->license;
}

std::span<const std::string> PluginManifestHandle::GetPlatforms() const noexcept {
	if (const auto& platforms = _impl->platforms) {
		return *platforms;
	}
	return {};
}

const std::string& PluginManifestHandle::GetEntry() const noexcept {
	return _impl->entry;
}

const std::string& PluginManifestHandle::GetLanguage() const noexcept {
	return _impl->language.name;
}

std::span<const DependencyHandle> PluginManifestHandle::GetDependencies() const noexcept {
	if (const auto& dependencies = _impl->dependencies) {
		static_assert(sizeof(std::unique_ptr<Dependency>) == sizeof(DependencyHandle), "Unique ptr and handle must have the same size");
		return { reinterpret_cast<const DependencyHandle*>(dependencies->data()), dependencies->size() };
	}
	return {};
}

std::span<const ConflictHandle> PluginManifestHandle::GetConflicts() const noexcept {
	if (const auto& conflicts = _impl->conflicts) {
		static_assert(sizeof(std::unique_ptr<Conflict>) == sizeof(ConflictHandle), "Unique ptr and handle must have the same size");
		return { reinterpret_cast<const ConflictHandle*>(conflicts->data()), conflicts->size() };
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
