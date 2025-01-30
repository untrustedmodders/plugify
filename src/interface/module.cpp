#include <core/module.hpp>
#include <plugify/language_module_descriptor.hpp>
#include <plugify/module.hpp>

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

std::string_view ModuleHandle::GetFriendlyName() const noexcept {
	return _impl->GetFriendlyName();
}

fs::path_view ModuleHandle::GetFilePath() const noexcept {
	return _impl->GetFilePath().native();
}

fs::path_view ModuleHandle::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().native();
}

LanguageModuleDescriptorHandle ModuleHandle::GetDescriptor() const noexcept {
	return { _impl->GetDescriptor() };
}

ModuleState ModuleHandle::GetState() const noexcept {
	return _impl->GetState();
}

std::string_view ModuleHandle::GetError() const noexcept {
	return _impl->GetError();
}

std::optional<fs::path_view> ModuleHandle::FindResource(fs::path_view path) const {
	return _impl->FindResource(path);
}

#if PLUGIFY_INTERFACE
std::optional<fs::path_view> Module::FindResource(const fs::path&) const {
	return {};
}
#endif // PLUGIFY_INTERFACE