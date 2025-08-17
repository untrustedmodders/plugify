#pragma once

#include "plugify/api/module.hpp"
#include "plugin_manifest.hpp"

#include <plugify/api/plugin.hpp>

namespace plugify {
	class Plugify;
	class Module;
	class Plugin {
	public:
		using State = PluginState;
		using Utils = PluginUtils;

		Plugin(UniqueId id, BasePaths paths, std::shared_ptr<Manifest> manifest);
		Plugin(const Plugin& plugin) = delete;
		Plugin(Plugin&& plugin) noexcept;
		~Plugin() = default;

	public:
		UniqueId GetId() const noexcept {
			return _id;
		}

		const std::string& GetName() const noexcept {
			return _manifest->name;
		}

		const fs::path& GetBaseDir() const noexcept {
			return _paths.base;
		}

		const fs::path& GetConfigsDir() const noexcept {
			return _paths.configs;
		}

		const fs::path& GetDataDir() const noexcept {
			return _paths.data;
		}

		const fs::path& GetLogsDir() const noexcept {
			return _paths.logs;
		}

		const PluginManifest& GetManifest() const noexcept {
			return *_manifest;
		}

		PluginState GetState() const noexcept {
			return _state;
		}

		std::span<const MethodData> GetMethods() const noexcept {
			return _methods;
		}

		MemAddr GetData() const noexcept {
			return _data;
		}

		const std::string& GetError() const noexcept {
			return _error;
		}

		void SetError(std::string error);

		void SetMethods(std::vector<MethodData> methods) {
			_methods = std::move(methods);
		}

		void SetData(MemAddr data) {
			_data = data;
		}

		void SetTable(MethodTable table) {
			_table = table;
		}

		Module* GetModule() const {
			return _module;
		}

		void SetModule(Module& module) noexcept {
			_module = &module;
		}

		void SetLoaded() noexcept {
			_state = PluginState::Loaded;
		}

		void SetRunning() noexcept {
			_state = PluginState::Running;
		}

		void SetTerminating() noexcept {
			_state = PluginState::Terminating;
		}

		void SetUnloaded() noexcept {
			_state = PluginState::NotLoaded;
		}

		bool HasUpdate() const noexcept {
			return _table.hasUpdate;
		}

		bool HasStart() const noexcept {
			return _table.hasStart;
		}

		bool HasEnd() const noexcept {
			return _table.hasEnd;
		}

		bool HasExport() const noexcept {
			return _table.hasExport;
		}

		bool Initialize(Plugify&);
		void Terminate();

		Plugin& operator=(const Plugin&) = delete;
		Plugin& operator=(Plugin&& other) noexcept = default;

		static inline constexpr std::string_view kFileExtension = ".pplugin";

	private:
		Module* _module{ nullptr };
		PluginState _state{ PluginState::NotLoaded };
		MethodTable _table;
		UniqueId _id;
		MemAddr _data;
		BasePaths _paths;
		std::vector<MethodData> _methods;
		std::shared_ptr<PluginManifest> _manifest;
		std::string _error;
	};
}
