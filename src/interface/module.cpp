#include <core/module.hpp>
#include <plugify/api/module.hpp>
#include <plugify/api/module_manifest.hpp>

using namespace plugify;

UniqueId ModuleHandle::GetId() const noexcept {
	return _impl->GetId();
}

std::string_view ModuleHandle::GetName() const noexcept {
	return _impl->GetName();
}

std::string_view ModuleHandle::GetLanguage() const noexcept {
	return _impl->GetLanguage();
}

fs::path_view ModuleHandle::GetFilePath() const noexcept {
	return _impl->GetFilePath().native();
}

fs::path_view ModuleHandle::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().native();
}

ModuleManifestHandle ModuleHandle::GetManifest() const noexcept {
	return { _impl->GetManifest() };
}

ModuleState ModuleHandle::GetState() const noexcept {
	return _impl->GetState();
}

std::string_view ModuleHandle::GetError() const noexcept {
	return _impl->GetError();
}
