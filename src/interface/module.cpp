#include <wizard/module.h>
#include <core/module.h>

using namespace wizard;

IModule::IModule(Module& impl) : _impl{impl} {
}

const std::string& IModule::GetName() const {
    return _impl.GetName();
}

const std::string& IModule::GetFriendlyName() const {
    return _impl.GetFriendlyName();
}

const std::filesystem::path& IModule::GetFilePath() const {
    return _impl.GetFilePath();
}

std::filesystem::path IModule::GetBaseDir() const {
    return _impl.GetBaseDir();
}

std::filesystem::path IModule::GetBinariesDir() const {
    return _impl.GetBinariesDir();
}

const LanguageModuleDescriptor& IModule::GetDescriptor() const {
    return _impl.GetDescriptor();
}

