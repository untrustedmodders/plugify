#include <wizard/module.h>
#include <core/module.h>

using namespace wizard;

IModule::IModule(Module& impl) : impl{impl} {
}

const std::string& IModule::GetName() const {
    return impl.GetName();
}

const std::string& IModule::GetFriendlyName() const {
    return impl.GetFriendlyName();
}

const std::filesystem::path& IModule::GetFilePath() const {
    return impl.GetFilePath();
}

std::filesystem::path IModule::GetBaseDir() const {
    return impl.GetBaseDir();
}

std::filesystem::path IModule::GetBinariesDir() const {
    return impl.GetBinariesDir();
}

const LanguageModuleDescriptor& IModule::GetDescriptor() const {
    return impl.GetDescriptor();
}

