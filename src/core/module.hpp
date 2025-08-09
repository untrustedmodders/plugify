#pragma once

#include "module_manifest.hpp"
#include <plugify/api/date_time.hpp>
#include <plugify/api/language_module.hpp>
#include <plugify/api/module.hpp>
#include <plugify/asm/assembly.hpp>

namespace plugify {
	class Plugin;
	struct ModuleManifest;
	class Module {
	public:
		Module(UniqueId id, std::unique_ptr<ModuleManifest> manifest, fs::path path);
		Module(const Module& module) = delete;
		Module(Module&& module) noexcept;
		~Module() = default;

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

		const fs::path& GetFilePath() const noexcept {
			return _filePath;
		}

		const fs::path& GetBaseDir() const noexcept {
			return _baseDir;
		}

		const ModuleManifest& GetManifest() const noexcept {
			return *_manifest;
		}

		ModuleState GetState() const noexcept {
			return _state;
		}

		const std::string& GetError() const noexcept {
			return _error;
		}

		bool Initialize(const std::shared_ptr<IPlugifyProvider>& provider);
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

		Module& operator=(const Module&) = delete;
		Module& operator=(Module&& other) noexcept = default;

		static inline constexpr std::string_view kFileExtension = ".pmodule";
		static inline constexpr std::string_view kGetLanguageModuleFn = "GetLanguageModule";

	private:
		ILanguageModule* _languageModule{ nullptr };
		ModuleState _state{ ModuleState::NotLoaded };
		MethodTable _table;
		UniqueId _id;
		std::string _name;
		std::string _lang;
		fs::path _filePath;
		fs::path _baseDir;
		std::unique_ptr<ModuleManifest> _manifest;
		std::unique_ptr<IAssembly> _assembly;
		std::string _error;
#if PLUGIFY_IS_DEBUG
		std::vector<Plugin*> _loadedPlugins;
#endif
	};
}
