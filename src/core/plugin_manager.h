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
		~PluginManager() override;

	public:
		/** IPluginManager interface */
		bool Initialize() override;
		void Terminate() override;
		bool IsInitialized() const override;

		ModuleOpt FindModule(const std::string& moduleName) const override;
		ModuleOpt FindModule(std::string_view moduleName) const override;
		ModuleOpt FindModuleFromId(UniqueId moduleId) const override;
		ModuleOpt FindModuleFromLang(const std::string& moduleLang) const override;
		ModuleOpt FindModuleFromPath(const fs::path& moduleFilePath) const override;
		std::vector<ModuleRef> GetModules() const override;

		PluginOpt FindPlugin(const std::string& pluginName) const override;
		PluginOpt FindPlugin(std::string_view pluginName) const override;
		PluginOpt FindPluginFromId(UniqueId pluginId) const override;
		PluginOpt FindPluginFromDescriptor(const PluginReferenceDescriptorRef& pluginDescriptor) const override;
		std::vector<PluginRef> GetPlugins() const override;

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