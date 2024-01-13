#pragma once

#include <wizard/plugin.h>
#include <wizard/plugin_descriptor.h>

namespace wizard {
	class Module;
	class LocalPackage;
	class Plugin final : public IPlugin {
	public:
		Plugin(UniqueId id, const LocalPackage& package);
		~Plugin() = default;

	private:
		friend class IPlugin;

		/* IPlugin interface */
		UniqueId GetId_() const {
			return _id;
		}

		const std::string& GetName_() const {
			return _name;
		}

		const std::string& GetFriendlyName_() const {
			return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
		}

		const fs::path& GetFilePath_() const {
			return _filePath;
		}

		const fs::path& GetBaseDir_() const {
			return _baseDir;
		}

		const fs::path& GetContentDir_() const {
			return _contentDir;
		}

		const PluginDescriptor& GetDescriptor_() const {
			return *_descriptor;
		}

		PluginState GetState_() const {
			return _state;
		}

		const std::string& GetError_() const {
			return _error;
		}

		const std::vector<MethodData>& GetMethods_() const {
			return _methods;
		}

	public:
		void SetError(std::string error);

		void SetMethods(std::vector<MethodData> methods) {
			_methods = std::move(methods);
		}

		const Module& GetModule() const {
			WZ_ASSERT(_module.has_value(), "Module is not set!");
			return _module.value().get();
		}

		void SetModule(const Module& module) {
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

		static inline const char* const kFileExtension = ".wplugin";

	private:
		UniqueId _id{ std::numeric_limits<UniqueId>::max() };
		std::string _name;
		fs::path _filePath;
		fs::path _baseDir;
		fs::path _contentDir;
		std::string _error;
		std::optional<std::reference_wrapper<const Module>> _module;
		std::vector<MethodData> _methods;
		std::shared_ptr<PluginDescriptor> _descriptor;
		PluginState _state{ PluginState::NotLoaded };
	};
}
