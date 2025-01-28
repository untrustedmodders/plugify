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
	class Plugin final {
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
			return *_error;
		}

		std::optional<fs::path_view> FindResource(const fs::path& path) const;

		void SetError(std::string error);

		void SetMethods(std::vector<MethodData> methods) {
			_methods = std::move(methods);
		}

		Module* GetModule() const {
			PL_ASSERT(_module, "Module is not set!");
			return _module;
		}

		void SetModule(Module* module) noexcept {
			_module = module;
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

		bool Initialize(std::weak_ptr<IPlugifyProvider> provider);
		void Terminate();

		static inline const char* const kFileExtension = ".pplugin";

	private:
		PluginState _state{ PluginState::NotLoaded };
		Module* _module{ nullptr };

		UniqueId _id;
		std::string _name;
		fs::path _baseDir;
		std::vector<MethodData> _methods;
		std::shared_ptr<PluginDescriptor> _descriptor;
		std::unordered_map<fs::path, fs::path, path_hash> _resources;
		std::unique_ptr<std::string> _error;
	};
}
