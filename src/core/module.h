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

	private:
		friend class IModule;

		/* IModule interface */
		uint64_t GetId_() const {
			return _id;
		}

		const std::string& GetName_() const {
			return _name;
		}

		const std::string& GetLanguage_() const {
			return _lang;
		}

		const std::string& GetFriendlyName_() const {
			return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
		}

		const fs::path& GetFilePath_() const {
			return _filePath;
		}

		const fs::path& GetBaseDir_() const {
			return _baseDir;
		}

		const fs::path& GetBinariesDir_() const {
			return _binaryDir;
		}

		const LanguageModuleDescriptor& GetDescriptor_() const {
			return _descriptor;
		}

		ModuleState GetState_() const {
			return _state;
		}

		const std::string& GetError_() const {
			return _error;
		}

	public:
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

		void SetLoaded() {
			_state = ModuleState::Loaded;
		}

		void SetUnloaded() {
			_state = ModuleState::NotLoaded;
		}

		void SetError(std::string error);

		static inline const char* const kFileExtension = ".wmodule";

	private:
		uint64_t _id{ std::numeric_limits<uint64_t>::max() };
		std::string _name;
		std::string _lang;
		fs::path _filePath;
		fs::path _baseDir;
		fs::path _binaryDir;
		LanguageModuleDescriptor _descriptor;
		ModuleState _state{ ModuleState::NotLoaded };
		std::string _error;
		std::unique_ptr<Library> _library;
		//std::vector<std::weak_ptr<Plugin>> _loadedPlugins;
		std::optional<std::reference_wrapper<ILanguageModule>> _languageModule;
	};
}