#pragma once

#include "language_module_descriptor.h"
#include <plugify/assembly.h>
#include <plugify/language_module.h>
#include <plugify/module.h>
#include <utils/hash.h>

namespace plugify {
	class Plugin;
	struct LocalPackage;
	class Module final {
	public:
		Module(UniqueId id, const LocalPackage& package);
		~Module();

	public:
		UniqueId GetId() const noexcept {
			return _id;
		}

		const std::string& GetName() const noexcept {
			return _name;
		}

		const std::string& GetLanguage() const noexcept {
			return _lang;
		}

		const std::string& GetFriendlyName() const noexcept {
			return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
		}

		const fs::path& GetFilePath() const noexcept {
			return _filePath;
		}

		const fs::path& GetBaseDir() const noexcept {
			return _baseDir;
		}

		const LanguageModuleDescriptor& GetDescriptor() const noexcept {
			return *_descriptor;
		}

		ModuleState GetState() const noexcept {
			return _state;
		}

		const std::string& GetError() const noexcept {
			return *_error;
		}

		std::optional<fs::path> FindResource(const fs::path& path) const;

		bool Initialize(std::weak_ptr<IPlugifyProvider> provider);
		void Terminate();

		bool LoadPlugin(Plugin& plugin) const;
		void StartPlugin(Plugin& plugin) const;
		void EndPlugin(Plugin& plugin) const;
		void MethodExport(Plugin& plugin) const;

		void SetError(std::string error);

		ILanguageModule& GetLanguageModule() const {
			PL_ASSERT(_languageModule.has_value(), "Language module is not set!");
			return _languageModule.value();
		}

		void SetLoaded() noexcept {
			_state = ModuleState::Loaded;
		}

		void SetUnloaded() noexcept {
			_state = ModuleState::NotLoaded;
		}

		static inline const char* const kFileExtension = ".pmodule";

	private:
		UniqueId _id{ -1 };
		std::string _name;
		std::string _lang;
		fs::path _filePath;
		fs::path _baseDir;
		std::shared_ptr<LanguageModuleDescriptor> _descriptor;
		std::unordered_map<fs::path, fs::path, path_hash> _resources;
		ModuleState _state{ ModuleState::NotLoaded };
		std::unique_ptr<std::string> _error;
		std::unique_ptr<Assembly> _assembly;
		std::optional<std::reference_wrapper<ILanguageModule>> _languageModule;
	};
}