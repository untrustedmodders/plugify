#pragma once

#include <wizard/module.h>
#include <wizard/language_module.h>
#include <wizard/language_module_descriptor.h>
#include <utils/library.h>

namespace wizard {
    enum class ModuleState : uint8_t {
        NotLoaded,
        Error,
        Loaded
    };

    class Plugin;
    class Module final : public IModule {
    public:
        Module(fs::path filePath, LanguageModuleDescriptor descriptor);
        ~Module();

        const std::string& GetName() const {
            return _name;
        }

        const std::string& GetFriendlyName() const {
            return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
        }

        const fs::path& GetFilePath() const {
            return _filePath;
        }

        fs::path GetBaseDir() const {
            return _filePath.parent_path().parent_path();
        }

        fs::path GetBinariesDir() const {
            return _filePath.parent_path();
        }

        const LanguageModuleDescriptor& GetDescriptor() const {
            return _descriptor;
        }

        bool Initialize(std::weak_ptr<IWizardProvider> provider);
        void Terminate();

        void LoadPlugin(const std::shared_ptr<Plugin>& plugin);
        void StartPlugin(const std::shared_ptr<Plugin>& plugin);
        void EndPlugin(const std::shared_ptr<Plugin>& plugin);

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

        static inline const char* const kFileExtension = ".wmodule";

    private:
        std::string _name;
        fs::path _filePath;
        LanguageModuleDescriptor _descriptor;
        ModuleState _state{ ModuleState::NotLoaded  };
        std::string _error;
        std::unique_ptr<Library> _library;
        std::vector<std::weak_ptr<Plugin>> _loadedPlugins;
        std::optional<std::reference_wrapper<ILanguageModule>> _languageModule;
    };
}