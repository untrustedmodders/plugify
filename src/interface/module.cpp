#include <core/module.hpp>
#include <plugify/language_module_descriptor.hpp>
#include <plugify/module.hpp>

using namespace plugify;

UniqueId ModuleRef::GetId() const noexcept {
	return _impl->GetId();
}

std::string_view ModuleRef::GetName() const noexcept {
	return _impl->GetName();
}

std::string_view ModuleRef::GetLanguage() const noexcept {
	return _impl->GetLanguage();
}

std::string_view ModuleRef::GetFriendlyName() const noexcept {
	return _impl->GetFriendlyName();
}

fs::path_view ModuleRef::GetFilePath() const noexcept {
	return _impl->GetFilePath().c_str();
}

fs::path_view ModuleRef::GetBaseDir() const noexcept {
	return _impl->GetBaseDir().c_str();
}

LanguageModuleDescriptorRef ModuleRef::GetDescriptor() const noexcept {
	return { _impl->GetDescriptor() };
}

ModuleState ModuleRef::GetState() const noexcept {
	return _impl->GetState();
}

std::string_view ModuleRef::GetError() const noexcept {
	return _impl->GetError();
}

std::optional<fs::path_view> ModuleRef::FindResource(fs::path_view path) const {
	return _impl->FindResource(path);
}
