#pragma once

#include "module_manifest.hpp"
#include "dependency.hpp"
#include <plugify/api/date_time.hpp>
#include <plugify/api/language_module.hpp>
#include <plugify/api/module.hpp>
#include <plugify/asm/assembly.hpp>

namespace plugify {
	struct Manifest;
	class Plugin;
	class Plugify;
	class Module {
	public:
		Module(UniqueId id, BasePaths paths, std::unique_ptr<Manifest> manifest);
		Module(const Module& module) = delete;
		Module(Module&& module) noexcept;
		~Module() = default;

	public:
		UniqueId GetId() const noexcept {
			return _id;
		}

		const std::string& GetName() const noexcept {
			return _manifest->name;
		}

		const std::string& GetLanguage() const noexcept {
			return _manifest->language;
		}

		const fs::path& GetBaseDir() const noexcept {
			return _paths.base;
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

		bool Initialize(Plugify& plugify);
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
		BasePaths _paths;
		std::unique_ptr<ModuleManifest> _manifest;
		std::unique_ptr<IAssembly> _assembly;
		std::string _error;
		mutable std::vector<PluginHandle> _loadedPlugins; // debug only
	};
}
