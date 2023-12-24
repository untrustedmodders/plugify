#pragma once

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
        virtual fs::path GetBaseDir() const = 0;
        virtual fs::path GetBinariesDir() const = 0;
        virtual const LanguageModuleDescriptor& GetDescriptor() const = 0;
    };
}
