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
	class Plugin {
	public:
		Plugin(UniqueId id, const LocalPackage& package);
		~Plugin();

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

		const PluginDescriptor& GetDescriptor() const noexcept {
			return *_descriptor;
		}

		PluginState GetState() const noexcept {
			return _state;
		}

		std::span<const MethodData> GetMethods() const noexcept {
			return _methods;
		}

		const std::string& GetError() const noexcept {
			return _error;
		}

		std::optional<fs::path_view> FindResource(const fs::path& path) const;

		void SetError(std::string error);

		void SetMethods(std::vector<MethodData> methods) {
			_methods = std::move(methods);
		}

		Module* GetModule() const {
			return _module;
		}

		void SetModule(const std::unique_ptr<Module>& module) noexcept {
			_module = module.get();
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

		bool Initialize(const std::shared_ptr<IPlugifyProvider>& provider);
		void Terminate();

		static inline std::string_view kFileExtension = ".pplugin";

	private:
		Module* _module{ nullptr };
		PluginState _state{ PluginState::NotLoaded };
		UniqueId _id;
		std::string _name;
		fs::path _baseDir;
		std::vector<MethodData> _methods;
		std::shared_ptr<PluginDescriptor> _descriptor;
		std::unordered_map<fs::path, fs::path, path_hash> _resources;
		std::string _error;
	};
}
