#pragma once

#include <wizard/plugin.h>
#include <wizard/plugin_descriptor.h>

namespace wizard {
	class Module;
	class Plugin final : public IPlugin {
	public:
		Plugin(uint64_t id, std::string name, fs::path filePath, PluginDescriptor descriptor);
		~Plugin() = default;

		/* IPlugin interface */
		uint64_t GetId() const {
			return _id;
		}

		const std::string& GetName() const {
			return _name;
		}

		const std::string& GetFriendlyName() const {
			return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
		}

		const fs::path& GetFilePath() const {
			return _filePath;
		}

		fs::path GetBaseDir() const {
			return "";
		}

		fs::path GetContentDir() const {
			return "";
		}

		fs::path GetMountedAssetPath() const {
			return "";
		}

		const PluginDescriptor& GetDescriptor() const {
			return _descriptor;
		}

		const Module& GetModule() const {
			WZ_ASSERT(_module.has_value(), "Module is not set!");
			return _module.value().get();
		}

		void SetModule(const Module& module) {
			_module = module;
		}

		PluginState GetState() const {
			return _state;
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

		void SetError(std::string error);

		const std::string& GetError() const {
			return _error;
		}

		void SetMethods(std::vector<MethodData> methods) {
			_methods = std::move(methods);
		}

		const std::vector<MethodData>& GetMethods() const {
			return _methods;
		}

		static inline const char* const kFileExtension = ".wplugin";

	private:
		uint64_t _id{ std::numeric_limits<uint64_t>::max() };
		std::string _name;
		fs::path _filePath;
		std::string _error;
		std::optional<std::reference_wrapper<const Module>> _module;
		std::vector<MethodData> _methods;
		PluginDescriptor _descriptor;
		PluginState _state{ PluginState::NotLoaded };
	};
}
