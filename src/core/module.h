#pragma once

#include <wizard/module.h>
#include <wizard/language_module.h>
#include "language_module_descriptor.h"
#include "utils/library.h"

namespace wizard {
    enum class ModuleState : uint8_t {
        NotLoaded,
        Error,
        Loaded
    };

    class Module final : public IModule {
    public:
        Module(fs::path&& filePath, LanguageModuleDescriptor&& descriptor);
        ~Module();

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

        bool Initialize();
        void Terminate();

        // TODO: Add more interactions with ILanguageModule

        ILanguageModule& GetLanguageModule() {
            return _languageModule.value();
        }

        ModuleState GetState() const {
            return _state;
        }

        void SetError(std::string error) {
            _error = std::move(error);
            _state = ModuleState::Error;
            WIZARD_LOG(_error, ErrorLevel::ERROR);
        }

        void SetLoaded() {
            _state = ModuleState::Loaded;
        }

    private:
        std::string _name;
        fs::path _filePath;
        LanguageModuleDescriptor _descriptor;
        ModuleState _state;
        std::string _error;
        std::unique_ptr<Library> _library;
        std::optional<std::reference_wrapper<ILanguageModule>> _languageModule;
    };
}