#pragma once

#include "wizard_context.h"
#include <wizard/plugin_manager.h>
#include <wizard/plugin.h>
#include <wizard/language_module.h>

namespace wizard {
	class Plugin;
	class Module;
	class IWizard;

	class PluginManager final : public IPluginManager, public WizardContext {
	public:
		explicit PluginManager(std::weak_ptr<IWizard> wizard);
		~PluginManager();

		/** IPluginManager interface */
		ModuleRef FindModule(const std::string& moduleName);
		ModuleRef FindModule(std::string_view moduleName);
		ModuleRef FindModuleFromLang(const std::string& moduleLang);
		ModuleRef FindModuleFromPath(const std::filesystem::path& moduleFilePath);
		ModuleRef FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor);
		std::vector<std::reference_wrapper<const IModule>> GetModules();

		PluginRef FindPlugin(const std::string& pluginName);
		PluginRef FindPlugin(std::string_view pluginName);
		PluginRef FindPluginFromId(uint64_t pluginId);
		PluginRef FindPluginFromPath(const fs::path& pluginFilePath);
		PluginRef FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor);
		std::vector<std::reference_wrapper<const IPlugin>> GetPlugins();

		bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies);
		bool GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies);
		bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies);

	private:
		using PluginList = std::vector<std::unique_ptr<Plugin>>;
		using ModuleList = std::vector<std::unique_ptr<Module>>;
		using VisitedPluginMap = std::unordered_map<std::string, std::pair<bool, bool>>;

		void DiscoverAllPlugins();
		void DiscoverAllModules();
		void ReadAllPluginsDescriptors();

		void LoadRequiredLanguageModules();
		void LoadAndStartAvailablePlugins();
		void TerminateAllPlugins();

		static void SortPluginsByDependencies(const std::string& pluginName, PluginList& sourceList, PluginList& targetList);
		static bool HasCyclicDependencies(PluginList& plugins);
		static bool IsCyclic(const std::unique_ptr<Plugin>& plugin, PluginList& plugins, VisitedPluginMap& visitedPlugins);

		static bool IsSupportsPlatform(std::span<const std::string> supportedPlatforms) {
			return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), WIZARD_PLATFORM) != supportedPlatforms.end();
		}

	private:
		ModuleList _allModules;
		PluginList _allPlugins;
	};
}