#include <plugify/api/module_handle.hpp>
#include <plugify/api/module_manifest_handle.hpp>
#include <plugify/core/module.hpp>

using namespace plugify;

UniqueId ModuleHandle::GetId() const noexcept {
	return _impl->GetId();
}

const std::string& ModuleHandle::GetName() const noexcept {
	return _impl->GetName();
}

const std::string& ModuleHandle::GetLanguage() const noexcept {
	return _impl->GetLanguage();
}

const std::filesystem::path& ModuleHandle::GetBaseDir() const noexcept {
	return _impl->GetBaseDir();
}

ModuleManifestHandle ModuleHandle::GetManifest() const noexcept {
	return _impl->GetManifest();
}

ModuleState ModuleHandle::GetState() const noexcept {
	return _impl->GetState();
}

const std::string& ModuleHandle::GetError() const noexcept {
	return _impl->GetError();
}
