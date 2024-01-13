#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <filesystem>
#include <functional>
#include <optional>
#include <wizard_export.h>

namespace wizard {
	class IModule;
	class IPlugin;
	class PluginManager;
	struct PluginReferenceDescriptor;

	using ModuleRef = std::optional<std::reference_wrapper<const IModule>>;
	using PluginRef = std::optional<std::reference_wrapper<const IPlugin>>;

	// Plugin manager provided to user, which implemented in core
	class WIZARD_API IPluginManager {
	protected:
		explicit IPluginManager(PluginManager& impl);
		~IPluginManager() = default;

	public:
		ModuleRef FindModule(const std::string& moduleName) const;
		ModuleRef FindModule(std::string_view moduleName) const;
		ModuleRef FindModuleFromId(std::uint64_t moduleId) const;
		ModuleRef FindModuleFromLang(const std::string& moduleLang) const;
		ModuleRef FindModuleFromPath(const std::filesystem::path& moduleFilePath) const;
		ModuleRef FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor) const;

		std::vector<std::reference_wrapper<const IModule>> GetModules() const;

		PluginRef FindPlugin(const std::string& pluginName) const;
		PluginRef FindPlugin(std::string_view pluginName) const;
		PluginRef FindPluginFromId(std::uint64_t pluginId) const;
		PluginRef FindPluginFromPath(const std::filesystem::path& pluginFilePath) const;
		PluginRef FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) const;

		std::vector<std::reference_wrapper<const IPlugin>> GetPlugins() const;

		bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;
		bool GetPluginDependencies_FromFilePath(const std::filesystem::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;
		bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;

	private:
		PluginManager& _impl;
	};

}
