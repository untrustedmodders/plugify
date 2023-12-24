#pragma once

#include <wizard/module.h>
#include <wizard/language_module.h>
#include "language_module_descriptor.h"
#include "library.h"

namespace wizard {
    class IModule;
    class Module final : public IModule {
    public:
        Module(fs::path filePath, LanguageModuleDescriptor descriptor);
        ~Module() override = default;

        const std::string& GetName() const override {
            return m_name;
        }

        const std::string& GetFriendlyName() const override {
            return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
        }

        const fs::path& GetDescriptorFilePath() const override {
            return m_filePath;
        }

        // TODO: Implement
        fs::path GetBaseDir() const override {
            return "";
        }
        fs::path GetBinariesDir() const override {
            return "";
        }

        const LanguageModuleDescriptor& GetDescriptor() const override {
            return m_descriptor;
        }

        bool IsInitialized() const { return languageModule.has_value(); }

    private:
        bool Initialize();
        void Shutdown();
        // TODO: Add more interactions with ILanguageModule

        ILanguageModule& GetLanguageModule() {
            return languageModule.value().get();
        }

    private:
        std::string m_name;
        fs::path m_filePath;
        LanguageModuleDescriptor m_descriptor;
        std::unique_ptr<Library> library;
        std::optional<std::reference_wrapper<ILanguageModule>> languageModule;
    };
}