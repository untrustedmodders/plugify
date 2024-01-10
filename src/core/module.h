#pragma once

#include <wizard/module.h>
#include <wizard/language_module.h>
#include <wizard/language_module_descriptor.h>
#include <utils/library.h>

namespace wizard {
	class Plugin;
	class Module final : public IModule {
	public:
		Module(uint64_t id, std::string name, std::string lang, fs::path filePath, LanguageModuleDescriptor descriptor);
		~Module();

		/* IModule interface */
		uint64_t GetId() const {
			return _id;
		}

		const std::string& GetName() const {
			return _name;
		}

		const std::string& GetLanguage() const {
			return _lang;
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

		void LoadPlugin(Plugin& plugin) const;
		void StartPlugin(Plugin& plugin) const;
		void EndPlugin(Plugin& plugin) const;
		void MethodExport(Plugin& plugin) const;

		// TODO: Add more interactions with ILanguageModule

		ILanguageModule& GetLanguageModule() const {
			WZ_ASSERT(_languageModule.has_value(), "Language module is not set!");
			return _languageModule.value();
		}

		ModuleState GetState() const {
			return _state;
		}

		void SetLoaded() {
			_state = ModuleState::Loaded;
		}

		void SetUnloaded() {
			_state = ModuleState::NotLoaded;
		}

		void SetError(std::string error);

		const std::string& GetError() const {
			return _error;
		}

		static inline const char* const kFileExtension = ".wmodule";

	private:
		uint64_t _id{ std::numeric_limits<uint64_t>::max() };
		std::string _name;
		std::string _lang;
		fs::path _filePath;
		LanguageModuleDescriptor _descriptor;
		ModuleState _state{ ModuleState::NotLoaded };
		std::string _error;
		std::unique_ptr<Library> _library;
		//std::vector<std::weak_ptr<Plugin>> _loadedPlugins;
		std::optional<std::reference_wrapper<ILanguageModule>> _languageModule;
	};
}