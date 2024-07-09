#pragma once

#include <plugify/module.h>
#include <plugify/language_module.h>
#include <plugify/language_module_descriptor.h>
#include <plugify/assembly.h>

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

		const LanguageModuleDescriptor& GetDescriptor() {
			return *_descriptor;
		}

		ModuleState GetState() {
			return _state;
		}

		const std::string& GetError() {
			return _error;
		}

		std::optional<fs::path> FindResource(const fs::path& path);

		bool Initialize(std::weak_ptr<IPlugifyProvider> provider);
		void Terminate();

		bool LoadPlugin(Plugin& plugin) const;
		void StartPlugin(Plugin& plugin) const;
		void EndPlugin(Plugin& plugin) const;
		void MethodExport(Plugin& plugin) const;

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

		static inline const char* const kFileExtension = ".pmodule";

	private:
		UniqueId _id{ -1 };
		std::string _name;
		std::string _lang;
		fs::path _filePath;
		fs::path _baseDir;
		std::shared_ptr<LanguageModuleDescriptor> _descriptor;
		std::unordered_map<fs::path, fs::path, PathHash> _resources;
		ModuleState _state{ ModuleState::NotLoaded };
		std::string _error;
		std::unique_ptr<Assembly> _assembly;
		std::optional<std::reference_wrapper<ILanguageModule>> _languageModule;
	};
}