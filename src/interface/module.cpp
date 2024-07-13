#include <plugify/module.h>
#include <plugify/language_module_descriptor.h>
#include <core/module.h>

using namespace plugify;

UniqueId IModule::GetId() const noexcept {
	return _impl->GetId();
}

std::string_view IModule::GetName() const noexcept {
	return _impl->GetName();
}

std::string_view IModule::GetLanguage() const noexcept {
	return _impl->GetLanguage();
}

std::string_view IModule::GetFriendlyName() const noexcept {
	return _impl->GetFriendlyName();
}

const fs::path& IModule::GetFilePath() const noexcept {
	return _impl->GetFilePath();
}

const fs::path& IModule::GetBaseDir() const noexcept {
	return _impl->GetBaseDir();
}

ILanguageModuleDescriptor IModule::GetDescriptor() const noexcept {
	return { _impl->GetDescriptor() };
}

ModuleState IModule::GetState() const noexcept {
	return _impl->GetState();
}

std::string_view IModule::GetError() const noexcept {
	return _impl->GetError();
}

std::optional<fs::path> IModule::FindResource(const fs::path& path) const {
	return _impl->FindResource(path);
}