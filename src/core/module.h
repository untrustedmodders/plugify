#pragma once

#include <plugify/module.h>
#include <plugify/language_module.h>
#include <plugify/language_module_descriptor.h>
#include <utils/library.h>

namespace plugify {
	class Plugin;
	struct LocalPackage;
	class Module final : public IModule {
	public:
		Module(UniqueId id, const LocalPackage& package);
		~Module();

	public:
		/* IModule interface */
		UniqueId GetId() {
			return _id;
		}

		const std::string& GetName() {
			return _name;
		}

		const std::string& GetLanguage() {
			return _lang;
		}

		const std::string& GetFriendlyName() {
			return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
		}

		const fs::path& GetFilePath() {
			return _filePath;
		}

		const fs::path& GetBaseDir() {
			return _baseDir;
		}

		const fs::path& GetBinariesDir() {
			return _binaryDir;
		}

		const LanguageModuleDescriptor& GetDescriptor() {
			return *_descriptor;
		}

		ModuleState GetState() {
			return _state;
		}

		const std::string& GetError() {
			return _error;
		}

		bool Initialize(std::weak_ptr<IPlugifyProvider> provider);
		void Terminate();

		void LoadPlugin(Plugin& plugin) const;
		void StartPlugin(Plugin& plugin) const;
		void EndPlugin(Plugin& plugin) const;
		void MethodExport(Plugin& plugin) const;

		// TODO: Add more interactions with ILanguageModule

		ILanguageModule& GetLanguageModule() const {
			PL_ASSERT(_languageModule.has_value(), "Language module is not set!");
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
		UniqueId _id{ std::numeric_limits<UniqueId>::max() };
		std::string _name;
		std::string _lang;
		fs::path _filePath;
		fs::path _baseDir;
		fs::path _binaryDir;
		std::shared_ptr<LanguageModuleDescriptor> _descriptor;
		ModuleState _state{ ModuleState::NotLoaded };
		std::string _error;
		std::unique_ptr<Library> _library;
		//std::vector<std::weak_ptr<Plugin>> _loadedPlugins;
		std::optional<std::reference_wrapper<ILanguageModule>> _languageModule;
	};
}