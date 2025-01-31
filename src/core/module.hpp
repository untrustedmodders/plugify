#pragma once

#include "language_module_descriptor.hpp"
#include <plugify/assembly.hpp>
#include <plugify/language_module.hpp>
#include <plugify/module.hpp>
#include <plugify/date_time.hpp>
#include <utils/hash.hpp>

namespace plugify {
	class Plugin;
	struct LocalPackage;
	class Module {
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
			return _error;
		}

		std::optional<fs::path_view> FindResource(const fs::path& path) const;

		bool Initialize(const std::shared_ptr<ProviderHandle>& provider);
		void Terminate();
		void Update(DateTime dt);

		bool LoadPlugin(Plugin& plugin) const;
		void StartPlugin(Plugin& plugin) const;
		void UpdatePlugin(Plugin& plugin, DateTime dt) const;
		void EndPlugin(Plugin& plugin) const;
		void MethodExport(Plugin& plugin) const;

		void SetError(std::string error);

		ILanguageModule* GetLanguageModule() const {
			return _languageModule;
		}

		void SetLoaded() noexcept {
			_state = ModuleState::Loaded;
		}

		void SetUnloaded() noexcept {
			_state = ModuleState::NotLoaded;
		}

		static inline std::string_view kFileExtension = ".pmodule";

	private:
		ILanguageModule* _languageModule{ nullptr };
		ModuleState _state{ ModuleState::NotLoaded };
		bool _requireUpdate{ false };
		UniqueId _id;
		std::string _name;
		std::string _lang;
		fs::path _filePath;
		fs::path _baseDir;
		std::shared_ptr<LanguageModuleDescriptor> _descriptor;
		std::unordered_map<fs::path, fs::path, path_hash> _resources;
		std::unique_ptr<Assembly> _assembly;
		std::string _error;
	};
}
