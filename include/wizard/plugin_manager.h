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

	using ModuleRef = std::optional<IModule>;
	using PluginRef = std::optional<IPlugin>;

	// Plugin manager provided to user, which implemented in core
	class WIZARD_API IPluginManager {
	protected:
		explicit IPluginManager(PluginManager& impl);
		~IPluginManager() = default;

	public:
		ModuleRef FindModule(const std::string& moduleName);
		ModuleRef FindModule(std::string_view moduleName);
		ModuleRef FindModuleFromLang(const std::string& moduleLang);
		ModuleRef FindModuleFromPath(const std::filesystem::path& moduleFilePath);
		ModuleRef FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor);

		std::vector<std::reference_wrapper<const IModule>> GetModules();

		PluginRef FindPlugin(const std::string& pluginName);
		PluginRef FindPlugin(std::string_view pluginName);
		PluginRef FindPluginFromId(std::uint64_t pluginId);
		PluginRef FindPluginFromPath(const std::filesystem::path& pluginFilePath);
		PluginRef FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor);

		std::vector<std::reference_wrapper<const IPlugin>> GetPlugins();

		bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies);
		bool GetPluginDependencies_FromFilePath(const std::filesystem::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies);
		bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies);

	private:
		PluginManager& _impl;
	};

}
