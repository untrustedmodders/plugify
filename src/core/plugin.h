#pragma once

#include <plugify/plugin.h>
#include <plugify/plugin_descriptor.h>

namespace plugify {
	class Module;
	struct LocalPackage;
	class IPlugifyProvider;
	class Plugin final : public IPlugin {
	public:
		Plugin(UniqueId id, const LocalPackage& package);
		~Plugin();

	public:
		/* IPlugin interface */
		UniqueId GetId() {
			return _id;
		}

		const std::string& GetName() {
			return _name;
		}

		const std::string& GetFriendlyName() {
			return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
		}

		const fs::path& GetBaseDir() {
			return _baseDir;
		}

		const PluginDescriptor& GetDescriptor() {
			return *_descriptor;
		}

		PluginState GetState() {
			return _state;
		}

		const std::string& GetError() {
			return _error;
		}

		const std::vector<MethodData>& GetMethods() {
			return _methods;
		}

		std::optional<fs::path> FindResource(const fs::path& path);

		void SetError(std::string error);

		void SetMethods(std::vector<MethodData> methods) {
			_methods = std::move(methods);
		}

		Module& GetModule() const {
			PL_ASSERT(_module.has_value(), "Module is not set!");
			return _module.value().get();
		}

		void SetModule(Module& module) {
			_module = module;
		}

		void SetLoaded() {
			_state = PluginState::Loaded;
		}

		void SetRunning() {
			_state = PluginState::Running;
		}

		void SetTerminating() {
			_state = PluginState::Terminating;
		}

		void SetUnloaded() {
			_state = PluginState::NotLoaded;
		}

		bool Initialize(std::weak_ptr<IPlugifyProvider> provider);
		void Terminate();

		static inline const char* const kFileExtension = ".pplugin";

	private:
		UniqueId _id{ -1 };
		std::string _name;
		fs::path _baseDir;
		std::string _error;
		std::optional<std::reference_wrapper<Module>> _module;
		std::vector<MethodData> _methods;
		std::shared_ptr<PluginDescriptor> _descriptor;
		std::unordered_map<fs::path, fs::path, PathHash> _resources;
		PluginState _state{ PluginState::NotLoaded };
	};
}
