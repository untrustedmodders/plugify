#include <plugify/module.h>
#include <plugify/language_module_descriptor.h>
#include <core/module.h>

using namespace plugify;

UniqueId ModuleRef::GetId() const noexcept {
	return _impl->GetId();
}

const std::string& ModuleRef::GetName() const noexcept {
	return _impl->GetName();
}

const std::string& ModuleRef::GetLanguage() const noexcept {
	return _impl->GetLanguage();
}

const std::string& ModuleRef::GetFriendlyName() const noexcept {
	return _impl->GetFriendlyName();
}

const fs::path& ModuleRef::GetFilePath() const noexcept {
	return _impl->GetFilePath();
}

const fs::path& ModuleRef::GetBaseDir() const noexcept {
	return _impl->GetBaseDir();
}

LanguageModuleDescriptorRef ModuleRef::GetDescriptor() const noexcept {
	return { _impl->GetDescriptor() };
}

ModuleState ModuleRef::GetState() const noexcept {
	return _impl->GetState();
}

const std::string& ModuleRef::GetError() const noexcept {
	return _impl->GetError();
}

std::optional<fs::path> ModuleRef::FindResource(const fs::path& path) const {
	return _impl->FindResource(path);
}