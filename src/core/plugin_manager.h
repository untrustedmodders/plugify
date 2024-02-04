#pragma once

#include "plugify_context.h"
#include <plugify/plugin_manager.h>
#include <plugify/plugin.h>
#include <plugify/language_module.h>

namespace plugify {
	class Plugin;
	class Module;
	class IPlugify;

	class PluginManager final : public IPluginManager, public PlugifyContext {
	public:
		explicit PluginManager(std::weak_ptr<IPlugify> plugify);
		~PluginManager();

	public:
		/** IPluginManager interface */
		bool Initialize();
		void Terminate();
		bool IsInitialized();

		ModuleOpt FindModule(const std::string& moduleName);
		ModuleOpt FindModule(std::string_view moduleName);
		ModuleOpt FindModuleFromId(UniqueId moduleId);
		ModuleOpt FindModuleFromLang(const std::string& moduleLang);
		ModuleOpt FindModuleFromPath(const std::filesystem::path& moduleFilePath);
		std::vector<ModuleRef> GetModules();

		PluginOpt FindPlugin(const std::string& pluginName);
		PluginOpt FindPlugin(std::string_view pluginName);
		PluginOpt FindPluginFromId(UniqueId pluginId);
		PluginOpt FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor);
		std::vector<PluginRef> GetPlugins();

		bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies);
		bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies);

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