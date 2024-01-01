#pragma once

#include <wizard/module.h>
#include <wizard/language_module.h>
#include <wizard/language_module_descriptor.h>
#include <utils/library.h>

namespace wizard {
	class Plugin;
	class Module final : public IModule {
	public:
		Module(std::string name, fs::path filePath, LanguageModuleDescriptor descriptor);
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
		void MethodExport(const std::shared_ptr<Plugin>& plugin);

		// TODO: Add more interactions with ILanguageModule

		ILanguageModule& GetLanguageModule() {
			return _languageModule.value();
		}

		ModuleState GetState() const {
			return _state;
		}

		void SetLoaded() {
			_state = ModuleState::Loaded;
		}

		void SetError(std::string error);

		const std::string& GetError() const {
			return _error;
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