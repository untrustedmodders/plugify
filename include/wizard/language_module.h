#pragma once

namespace wizard {
    class LanguageModuleDescriptor;

    class ILanguageModule {
    public:
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void* GetMethod(const std::string& name) = 0;
        virtual void OnNativeAdded(/*data*/) = 0;
        virtual bool OnLoadPlugin(/*data*/) = 0;
        virtual const std::string& GetName() const = 0;
        virtual const std::string& GetFriendlyName() const = 0;
        virtual const fs::path& GetDescriptorFilePath() const = 0;
        virtual fs::path GetBaseDir() const = 0;
        virtual fs::path GetBinariesDir() const = 0;
        virtual const LanguageModuleDescriptor& GetDescriptor() const = 0;
    };
}
