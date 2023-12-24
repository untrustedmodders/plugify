#pragma once

#include <string>
#include <filesystem>

namespace wizard {
    struct LanguageModuleDescriptor;

    // Language module provided to user implemented in core !
    class IModule {
    protected:
        ~IModule() = default;

    public:
        virtual const std::string& GetName() const = 0;
        virtual const std::string& GetFriendlyName() const = 0;
        virtual const fs::path& GetDescriptorFilePath() const = 0;
        virtual std::filesystem::path GetBaseDir() const = 0;
        virtual std::filesystem::path GetBinariesDir() const = 0;
        virtual const LanguageModuleDescriptor& GetDescriptor() const = 0;
    };
}
