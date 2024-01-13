#include <wizard/module.h>
#include <core/module.h>

using namespace wizard;

IModule::IModule(Module& impl) : _impl{impl} {
}

UniqueId IModule::GetId() const {
	return _impl.GetId_();
}

const std::string& IModule::GetName() const {
    return _impl.GetName_();
}

const std::string& IModule::GetLanguage() const {
    return _impl.GetLanguage_();
}

const std::string& IModule::GetFriendlyName() const {
    return _impl.GetFriendlyName_();
}

const fs::path& IModule::GetFilePath() const {
    return _impl.GetFilePath_();
}

const fs::path& IModule::GetBaseDir() const {
    return _impl.GetBaseDir_();
}

const fs::path& IModule::GetBinariesDir() const {
    return _impl.GetBinariesDir_();
}

const LanguageModuleDescriptor& IModule::GetDescriptor() const {
    return _impl.GetDescriptor_();
}

ModuleState IModule::GetState() const {
    return _impl.GetState_();
}

const std::string& IModule::GetError() const {
    return _impl.GetError_();
}