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
		bool Initialize_();
		void Terminate_();

		ModuleOpt FindModule_(const std::string& moduleName) const;
		ModuleOpt FindModule_(std::string_view moduleName) const;
		ModuleOpt FindModuleFromId_(UniqueId moduleId) const;
		ModuleOpt FindModuleFromLang_(const std::string& moduleLang) const;
		ModuleOpt FindModuleFromPath_(const std::filesystem::path& moduleFilePath) const;
		ModuleOpt FindModuleFromDescriptor_(const PluginReferenceDescriptor& moduleDescriptor) const;
		std::vector<ModuleRef> GetModules_() const;

		PluginOpt FindPlugin_(const std::string& pluginName) const;
		PluginOpt FindPlugin_(std::string_view pluginName) const;
		PluginOpt FindPluginFromId_(UniqueId pluginId) const;
		PluginOpt FindPluginFromPath_(const fs::path& pluginFilePath) const;
		PluginOpt FindPluginFromDescriptor_(const PluginReferenceDescriptor& pluginDescriptor) const;
		std::vector<PluginRef> GetPlugins_() const;

		bool GetPluginDependencies_(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;
		bool GetPluginDependencies_FromFilePath_(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;
		bool GetPluginDependencies_FromDescriptor_(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;

	private:
		using PluginList = std::vector<std::unique_ptr<Plugin>>;
		using ModuleList = std::vector<std::unique_ptr<Module>>;
		using VisitedPluginMap = std::unordered_map<std::string, std::pair<bool, bool>>;

		void DiscoverAllModulesAndPlugins();
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