#pragma once

#include "plugin_descriptor.hpp"
#include <plugify/plugin.hpp>
#include <plugify/date_time.hpp>
#include <utils/hash.hpp>
#include <utils/pointer.hpp>

namespace plugify {
	class Module;
	struct LocalPackage;
	class IPlugifyProvider;
	struct BasePaths;
	class Plugin {
	public:
		Plugin(UniqueId id, const LocalPackage& package, const BasePaths& paths);
		Plugin(const Plugin& plugin) = delete;
		Plugin(Plugin&& plugin) noexcept;
		~Plugin() = default;

	public:
		UniqueId GetId() const noexcept {
			return _id;
		}

		const std::string& GetName() const noexcept {
			return _name;
		}

		const std::string& GetFriendlyName() const noexcept {
			return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
		}

		const fs::path& GetBaseDir() const noexcept {
			return _baseDir;
		}

		const fs::path& GetConfigsDir() const noexcept {
			return _configsDir;
		}

		const fs::path& GetDataDir() const noexcept {
			return _dataDir;
		}

		const fs::path& GetLogsDir() const noexcept {
			return _logsDir;
		}

		const PluginDescriptor& GetDescriptor() const noexcept {
			return *_descriptor;
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
			return *_error;
		}

		std::optional<fs::path_view> FindResource(const fs::path& path) const;

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

		bool Initialize(const std::shared_ptr<IPlugifyProvider>& provider);
		void Terminate();

		Plugin& operator=(const Plugin&) = delete;
		Plugin& operator=(Plugin&& other) noexcept;

		static inline std::string_view kFileExtension = ".pplugin";

	private:
		Module* _module{ nullptr };
		PluginState _state{ PluginState::NotLoaded };
		MethodTable _table;
		UniqueId _id;
		MemAddr _data;
		std::string _name;
		fs::path _baseDir;
		fs::path _configsDir;
		fs::path _dataDir;
		fs::path _logsDir;
		std::vector<MethodData> _methods;
		std::shared_ptr<PluginDescriptor> _descriptor;
		std::unordered_map<fs::path, fs::path, path_hash> _resources;
		std::unique_ptr<std::string> _error;
	};

	struct BasePaths {
		fs::path configs;
		fs::path data;
		fs::path logs;
	};
}
