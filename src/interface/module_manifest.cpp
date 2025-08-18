#include <plugify/api/conflict_handle.hpp>
#include <plugify/api/dependency_handle.hpp>
#include <plugify/api/module_manifest_handle.hpp>
#include <plugify/core/manifest.hpp>

using namespace plugify;

const std::string& ModuleManifestHandle::GetName() const noexcept {
	return _impl->name;
}

const Version& ModuleManifestHandle::GetVersion() const noexcept {
	return _impl->version;
}

const std::string& ModuleManifestHandle::GetDescription() const noexcept {
	return _impl->description;
}

const std::string& ModuleManifestHandle::GetAuthor() const noexcept {
	return _impl->author;
}

const std::string& ModuleManifestHandle::GetWebsite() const noexcept {
	return _impl->website;
}

const std::string& ModuleManifestHandle::GetLicense() const noexcept {
	return _impl->license;
}

std::span<const std::string> ModuleManifestHandle::GetPlatforms() const noexcept {
	if (const auto& platforms = _impl->platforms) {
		return *platforms;
	}
	return {};
}

std::span<const DependencyHandle> ModuleManifestHandle::GetDependencies() const noexcept {
	if (const auto& dependencies = _impl->dependencies) {
		static_assert(sizeof(std::unique_ptr<Dependency>) == sizeof(DependencyHandle), "Unique ptr and handle must have the same size");
		return { reinterpret_cast<const DependencyHandle*>(dependencies->data()), dependencies->size() };
	}
	return {};
}

std::span<const ConflictHandle> ModuleManifestHandle::GetConflicts() const noexcept {
	if (const auto& conflicts = _impl->conflicts) {
		static_assert(sizeof(std::unique_ptr<Conflict>) == sizeof(ConflictHandle), "Unique ptr and handle must have the same size");
		return { reinterpret_cast<const ConflictHandle*>(conflicts->data()), conflicts->size() };
	}
	return {};
}

std::span<const std::string> ModuleManifestHandle::GetDirectories() const noexcept {
	if (const auto& directories = _impl->directories) {
		return *directories;
	}
	return {};
}

const std::string& ModuleManifestHandle::GetLanguage() const noexcept {
	return _impl->language;
}

bool ModuleManifestHandle::IsForceLoad() const noexcept {
	return _impl->forceLoad.value_or(false);
}
