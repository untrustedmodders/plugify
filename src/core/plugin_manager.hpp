#pragma once

#include "plugify_context.hpp"
#include <plugify/language_module.hpp>
#include <plugify/plugin.hpp>
#include <plugify/plugin_manager.hpp>

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
		void Update(DateTime dt) override;

		ModuleHandle FindModule(std::string_view moduleName) const override;
		ModuleHandle FindModuleFromId(UniqueId moduleId) const override;
		ModuleHandle FindModuleFromLang(std::string_view moduleLang) const override;
		ModuleHandle FindModuleFromPath(const fs::path& moduleFilePath) const override;
		std::vector<ModuleHandle> GetModules() const override;

		PluginHandle FindPlugin(std::string_view pluginName) const override;
		PluginHandle FindPluginFromId(UniqueId pluginId) const override;
		PluginHandle FindPluginFromDescriptor(const PluginReferenceDescriptorHandle & pluginDescriptor) const override;
		std::vector<PluginHandle> GetPlugins() const override;

	private:
		using PluginList = std::vector<Plugin>;
		using ModuleList = std::vector<Module>;
		using VisitedPluginMap = std::unordered_map<std::string, std::pair<bool, bool>>;

		void DiscoverAllModulesAndPlugins();
		bool PartitionLocalPackages();
		void LoadRequiredLanguageModules();
		void LoadAndStartAvailablePlugins();
		void TerminateAllPlugins();
		void TerminateAllModules();

		static void SortPluginsByDependencies(const std::string& pluginName, PluginList& sourceList, PluginList& targetList);
		static bool HasCyclicDependencies(PluginList& plugins);
		static bool IsCyclic(const Plugin& plugin, PluginList& plugins, VisitedPluginMap& visitedPlugins);

	private:
		ModuleList _allModules;
		PluginList _allPlugins;
		bool _inited{ false };
	};
}
