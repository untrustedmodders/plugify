#pragma once

#include "context.hpp"
#include <plugify/api/language_module.hpp>
#include <plugify/api/manager.hpp>
#include <plugify/api/plugin.hpp>

namespace plugify {
	class Plugin;
	class Module;
	class Plugify;

	class Manager final : public Context {
	public:
		explicit Manager(Plugify& plugify);
		~Manager();

	public:
		bool Initialize();
		void Terminate();
		bool IsInitialized() const;
		void Update(DateTime dt);

		ModuleHandle FindModule(std::string_view moduleName) const;
		ModuleHandle FindModuleFromId(UniqueId moduleId) const;
		std::vector<ModuleHandle> GetModules() const;

		PluginHandle FindPlugin(std::string_view pluginName) const;
		PluginHandle FindPluginFromId(UniqueId pluginId) const;
		std::vector<PluginHandle> GetPlugins() const;

	private:
		using PluginList = std::vector<Plugin>;
		using ModuleList = std::vector<Module>;

		void DiscoverAllModulesAndPlugins();
		bool PartitionLocalPackages();
		bool TopologicalSortPlugins();
		void LoadRequiredLanguageModules();
		void LoadAndStartAvailablePlugins();
		void TerminateAllPlugins();
		void TerminateAllModules();

	private:
		PluginList _allPlugins;
		ModuleList _allModules;
		bool _inited{ false };
	};
}
