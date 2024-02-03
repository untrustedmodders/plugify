#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <filesystem>
#include <functional>
#include <optional>
#include <plugify_export.h>

namespace plugify {
	class IModule;
	class IPlugin;
	class PluginManager;
	struct PluginReferenceDescriptor;

	using UniqueId = std::uintmax_t;
	using ModuleRef = std::reference_wrapper<const IModule>;
	using PluginRef = std::reference_wrapper<const IPlugin>;
	using ModuleOpt = std::optional<ModuleRef>;
	using PluginOpt = std::optional<PluginRef>;

	// Plugin manager provided to user, which implemented in core
	class PLUGIFY_API IPluginManager {
	protected:
		explicit IPluginManager(PluginManager& impl);
		~IPluginManager() = default;

	public:
		bool Initialize() const;
		void Terminate() const;
		bool IsInitialized() const;

		ModuleOpt FindModule(const std::string& moduleName) const;
		ModuleOpt FindModule(std::string_view moduleName) const;
		ModuleOpt FindModuleFromId(UniqueId moduleId) const;
		ModuleOpt FindModuleFromLang(const std::string& moduleLang) const;
		ModuleOpt FindModuleFromPath(const std::filesystem::path& moduleFilePath) const;
		ModuleOpt FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor) const;

		std::vector<ModuleRef> GetModules() const;

		PluginOpt FindPlugin(const std::string& pluginName) const;
		PluginOpt FindPlugin(std::string_view pluginName) const;
		PluginOpt FindPluginFromId(UniqueId pluginId) const;
		PluginOpt FindPluginFromPath(const std::filesystem::path& pluginFilePath) const;
		PluginOpt FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) const;

		std::vector<PluginRef> GetPlugins() const;

		bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;
		bool GetPluginDependencies_FromFilePath(const std::filesystem::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;
		bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;

	private:
		PluginManager& _impl;
	};
}
