#pragma once

#include <wizard/module.h>
#include <wizard/language_module.h>
#include "language_module_descriptor.h"
#include "utils/library.h"

namespace wizard {
    class IModule;
    class Module final : public IModule {
    public:
        Module(fs::path filePath, LanguageModuleDescriptor descriptor);
        ~Module() = default;

        const std::string& GetName() const override {
            return _name;
        }

        const std::string& GetFriendlyName() const override {
            return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
        }

        const fs::path& GetDescriptorFilePath() const override {
            return _filePath;
        }

        // TODO: Implement
        fs::path GetBaseDir() const override {
            return "";
        }
        fs::path GetBinariesDir() const override {
            return "";
        }

        const LanguageModuleDescriptor& GetDescriptor() const override {
            return _descriptor;
        }

        bool IsInitialized() const { return _languageModule.has_value(); }

        bool Initialize();
        void Terminate();
        // TODO: Add more interactions with ILanguageModule

    private:
        ILanguageModule& GetLanguageModule() {
            return _languageModule.value().get();
        }

    private:
        std::string _name;
        fs::path _filePath;
        LanguageModuleDescriptor _descriptor;
        std::unique_ptr<Library> _library;
        std::optional<std::reference_wrapper<ILanguageModule>> _languageModule;
    };
}