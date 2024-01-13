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

	private:
		friend class IPluginManager;

		/** IPluginManager interface */
		ModuleRef FindModule_(const std::string& moduleName) const;
		ModuleRef FindModule_(std::string_view moduleName) const;
		ModuleRef FindModuleFromId_(uint64_t moduleId) const;
		ModuleRef FindModuleFromLang_(const std::string& moduleLang) const;
		ModuleRef FindModuleFromPath_(const std::filesystem::path& moduleFilePath) const;
		ModuleRef FindModuleFromDescriptor_(const PluginReferenceDescriptor& moduleDescriptor) const;
		std::vector<std::reference_wrapper<const IModule>> GetModules_() const;

		PluginRef FindPlugin_(const std::string& pluginName) const;
		PluginRef FindPlugin_(std::string_view pluginName) const;
		PluginRef FindPluginFromId_(uint64_t pluginId) const;
		PluginRef FindPluginFromPath_(const fs::path& pluginFilePath) const;
		PluginRef FindPluginFromDescriptor_(const PluginReferenceDescriptor& pluginDescriptor) const;
		std::vector<std::reference_wrapper<const IPlugin>> GetPlugins_() const;

		bool GetPluginDependencies_(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;
		bool GetPluginDependencies_FromFilePath_(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;
		bool GetPluginDependencies_FromDescriptor_(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;

	public:
		static bool IsSupportsPlatform(std::span<const std::string> supportedPlatforms) {
			return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), WIZARD_PLATFORM) != supportedPlatforms.end();
		}

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

	private:
		ModuleList _allModules;
		PluginList _allPlugins;
	};
}