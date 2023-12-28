#pragma once

#include <string>
#include <filesystem>
#include <wizard_export.h>

namespace wizard {
    class Module;
    struct LanguageModuleDescriptor;

    // Language module provided to user implemented in core !
    class WIZARD_API IModule {
    protected:
        explicit IModule(Module& impl);
        ~IModule() = default;

    public:
        const std::string& GetName() const;
        const std::string& GetFriendlyName() const;
        const std::filesystem::path& GetFilePath() const;
        std::filesystem::path GetBaseDir() const;
        std::filesystem::path GetBinariesDir() const;
        const LanguageModuleDescriptor& GetDescriptor() const;

    private:
        Module& _impl;
    };
}
