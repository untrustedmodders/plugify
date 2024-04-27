#include <plugify/module.h>
#include <core/module.h>

using namespace plugify;

IModule::IModule(Module& impl) : _impl{impl} {
}

UniqueId IModule::GetId() const {
	return _impl.GetId();
}

const std::string& IModule::GetName() const {
    return _impl.GetName();
}

const std::string& IModule::GetLanguage() const {
    return _impl.GetLanguage();
}

const std::string& IModule::GetFriendlyName() const {
    return _impl.GetFriendlyName();
}

const fs::path& IModule::GetFilePath() const {
    return _impl.GetFilePath();
}

const fs::path& IModule::GetBaseDir() const {
    return _impl.GetBaseDir();
}

const LanguageModuleDescriptor& IModule::GetDescriptor() const {
    return _impl.GetDescriptor();
}

ModuleState IModule::GetState() const {
    return _impl.GetState();
}

const std::string& IModule::GetError() const {
    return _impl.GetError();
}

std::optional<fs::path> IModule::FindResource(const fs::path& path) const {
	return _impl.FindResource(path);
}