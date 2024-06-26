#include "plugin_manager.h"
#include "package_manager.h"
#include "plugin.h"
#include "module.h"

#include <plugify/plugin_manager.h>
#include <plugify/plugify.h>
#include <utils/json.h>

using namespace plugify;

PluginManager::PluginManager(std::weak_ptr<IPlugify> plugify) : IPluginManager(*this), PlugifyContext(std::move(plugify)) {
}

PluginManager::~PluginManager() {
	Terminate();
}

bool PluginManager::Initialize() {
	if (IsInitialized())
		return false;

	auto debugStart = DateTime::Now();
	DiscoverAllModulesAndPlugins();
	LoadRequiredLanguageModules();
	LoadAndStartAvailablePlugins();
	PL_LOG_DEBUG("PluginManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
	return true;
}

void PluginManager::Terminate() {
	if (!IsInitialized())
		return;

	TerminateAllPlugins();
	
	_allPlugins.clear();
	_allModules.clear();
}

bool PluginManager::IsInitialized() {
	return !_allPlugins.empty() || !_allModules.empty();
}

void PluginManager::DiscoverAllModulesAndPlugins() {
	PL_ASSERT(_allModules.empty(), "Modules already initialized");
	PL_ASSERT(_allPlugins.empty(), "Plugins already initialized");
	
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	if (auto packageManager = plugify->GetPackageManager().lock()) {
		for (const auto& packageRef : packageManager->GetLocalPackages()) {
			const auto& package = packageRef.get();
			if (package.type == "plugin") {
				auto id = static_cast<ptrdiff_t>(_allPlugins.size());
				_allPlugins.emplace_back(std::make_unique<Plugin>(id, package));
			} else {
				auto id = static_cast<ptrdiff_t>(_allModules.size());
				_allModules.emplace_back(std::make_unique<Module>(id, package));
			}
		}
	}
	
	if (_allModules.empty()) {
		PL_LOG_WARNING("Did not find any module. Check base directory path in config: '{}'", plugify->GetConfig().baseDir.string());
		return;
	}
	
	if (_allPlugins.empty()) {
		PL_LOG_WARNING("Did not find any plugin. Check base directory path in config: '{}'", plugify->GetConfig().baseDir.string());
		return;
	}

	PluginList sortedPlugins;
	sortedPlugins.reserve(_allPlugins.size());
	while (!_allPlugins.empty()) {
		SortPluginsByDependencies(_allPlugins.back()->GetName(), _allPlugins, sortedPlugins);
	}

	if (HasCyclicDependencies(sortedPlugins)) {
		PL_LOG_WARNING("Found cyclic dependencies");
	}

	_allPlugins = std::move(sortedPlugins);

	PL_LOG_VERBOSE("Plugins order after topological sorting by dependency: ");
	for (const auto& plugin : _allPlugins) {
		PL_LOG_VERBOSE("{} - {}", plugin->GetName(), plugin->GetFriendlyName());
	}
}

void PluginManager::LoadRequiredLanguageModules() {
	if (_allModules.empty())
		return;
	
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	std::unordered_set<UniqueId> modules;
	modules.reserve(_allModules.size());

	for (const auto& plugin : _allPlugins) {
		const auto& lang = plugin->GetDescriptor().languageModule.name;
		auto it = std::find_if(_allModules.begin(), _allModules.end(), [&lang](const auto& p) {
			return p->GetLanguage() == lang;
		});
		if (it == _allModules.end()) {
			plugin->SetError(std::format("Language module: '{}' missing for plugin: '{}'", lang, plugin->GetFriendlyName()));
			continue;
		}
		auto& module = *it;
		plugin->Initialize(plugify->GetProvider());
		plugin->SetModule(*module);
		modules.emplace(module->GetId());
	}
	
	bool loadedAny = false;

	for (const auto& module : _allModules) {
		if (module->GetDescriptor().forceLoad || modules.contains(module->GetId())) {
			loadedAny |= module->Initialize(plugify->GetProvider());
		}
	}
	
	if (!loadedAny) {
		PL_LOG_WARNING("Did not load any module");
		return;
	}
}

void PluginManager::LoadAndStartAvailablePlugins() {
	if (_allPlugins.empty())
		return;
	
	bool loadedAny = false;
	
	for (const auto& plugin : _allPlugins) {
		if (plugin->GetState() == PluginState::NotLoaded) {
			if (plugin->GetModule().GetState() != ModuleState::Loaded) {
				plugin->SetError(std::format("Language module: '{}' missing", plugin->GetModule().GetFriendlyName()));
				continue;
			}
			std::vector<std::string_view> names;
			for (const auto& descriptor : plugin->GetDescriptor().dependencies) {
				auto dependencyPlugin = FindPlugin(descriptor.name);
				if ((!dependencyPlugin.has_value() || dependencyPlugin->get().GetState() != PluginState::Loaded) && !descriptor.optional) {
					names.emplace_back(descriptor.name);
				}
			}
			if (!names.empty()) {
				std::string error;
				std::format_to(std::back_inserter(error), "'{}", names[0]);
				for (auto it = std::next(names.begin()); it != names.end(); ++it) {
					std::format_to(std::back_inserter(error), "', '{}", *it);
				}
				std::format_to(std::back_inserter(error), "'");
				plugin->SetError(std::format("Not loaded {} dependency plugin(s)", error));
			} else {
				loadedAny |= plugin->GetModule().LoadPlugin(*plugin);
			}
		}
	}
	
	if (!loadedAny) {
		PL_LOG_WARNING("Did not load any plugin");
		return;
	}

	for (const auto& plugin : _allPlugins) {
		if (plugin->GetState() == PluginState::Loaded) {
			for (const auto& module : _allModules) {
				module->MethodExport(*plugin);
			}
		}
	}

	for (const auto& plugin : _allPlugins) {
		if (plugin->GetState() == PluginState::Loaded) {
			plugin->GetModule().StartPlugin(*plugin);
		}
	}
}

void PluginManager::TerminateAllPlugins() {
	if (_allPlugins.empty())
		return;
	
	for (auto it = _allPlugins.rbegin(); it != _allPlugins.rend(); ++it) {
		const auto& plugin = *it;
		if (plugin->GetState() == PluginState::Running) {
			plugin->GetModule().EndPlugin(*plugin);
		}
	}

	for (auto it = _allPlugins.rbegin(); it != _allPlugins.rend(); ++it) {
		const auto& plugin = *it;
		plugin->SetUnloaded();
	}
}

void PluginManager::SortPluginsByDependencies(const std::string& pluginName, PluginList& sourceList, PluginList& targetList) {
	auto it = std::find_if(sourceList.begin(), sourceList.end(), [&pluginName](const auto& plugin) {
		return plugin->GetName() == pluginName;
	});
	if (it != sourceList.end()) {
		auto index = static_cast<size_t>(std::distance(sourceList.begin(), it));
		auto plugin = std::move(sourceList[index]);
		sourceList.erase(it);
		for (const auto& descriptor : plugin->GetDescriptor().dependencies) {
			SortPluginsByDependencies(descriptor.name, sourceList, targetList);
		}
		targetList.emplace_back(std::move(plugin));
	}
}

bool PluginManager::HasCyclicDependencies(PluginList& plugins) {
	// Mark all the vertices as not visited
	// and not part of recursion stack
	VisitedPluginMap data; /* [visited, recursive] */

	// Call the recursive helper function
	// to detect cycle in different DFS trees
	for (const auto& plugin : plugins) {
		if (!data[plugin->GetName()].first && IsCyclic(plugin, plugins, data))
			return true;
	}

	return false;
}

bool PluginManager::IsCyclic(const std::unique_ptr<Plugin>& plugin, PluginList& plugins, VisitedPluginMap& visitedPlugins) {
	auto& [visited, recursive] = visitedPlugins[plugin->GetName()];
	if (!visited) {
		// Mark the current node as visited
		// and part of recursion stack
		visited = true;
		recursive = true;

		// Recur for all the vertices adjacent to this vertex
		for (const auto& descriptor : plugin->GetDescriptor().dependencies) {
			const auto& name = descriptor.name;

			auto it = std::find_if(plugins.begin(), plugins.end(), [&name](const auto& p) {
				return p->GetName() == name;
			});

			if (it != plugins.end()) {
				const auto& [vis, rec] = visitedPlugins[name];
				if ((!vis && IsCyclic(*it, plugins, visitedPlugins)) || rec)
					return true;
			}
		}
	}

	// Remove the vertex from recursion stack
	recursive = false;
	return false;
}

ModuleOpt PluginManager::FindModule(const std::string& moduleName) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleName](const auto& module) {
		return module->GetName() == moduleName;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModule(std::string_view moduleName) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleName](const auto& module) {
		return module->GetName() == moduleName;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModuleFromId(UniqueId moduleId) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleId](const auto& module) {
		return module->GetId() == moduleId;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModuleFromLang(const std::string& moduleLang) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleLang](const auto& module) {
		return module->GetLanguage() == moduleLang;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModuleFromPath(const fs::path& moduleFilePath) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleFilePath](const auto& module) {
		return module->GetFilePath() == moduleFilePath;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

std::vector<ModuleRef> PluginManager::GetModules() {
	std::vector<ModuleRef> modules;
	modules.reserve(_allModules.size());
	for (const auto& module : _allModules)  {
		modules.emplace_back(*module);
	}
	return modules;
}

PluginOpt PluginManager::FindPlugin(const std::string& pluginName) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
		return plugin->GetName() == pluginName;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginOpt PluginManager::FindPlugin(std::string_view pluginName) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
		return plugin->GetName() == pluginName;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginOpt PluginManager::FindPluginFromId(UniqueId pluginId) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginId](const auto& plugin) {
		return plugin->GetId() == pluginId;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginOpt PluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginDescriptor](const auto& plugin) {
		return plugin->GetName() == pluginDescriptor.name && (!pluginDescriptor.requestedVersion || plugin->GetDescriptor().version == pluginDescriptor.requestedVersion);
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

std::vector<PluginRef> PluginManager::GetPlugins() {
	std::vector<PluginRef> plugins;
	plugins.reserve(_allPlugins.size());
	for (const auto& plugin : _allPlugins)  {
		plugins.emplace_back(*plugin);
	}
	return plugins;
}

bool PluginManager::GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
	auto plugin = FindPlugin(pluginName);
	if (plugin.has_value()) {
		pluginDependencies = plugin->get().GetDescriptor().dependencies;
		return true;
	}
	return false;
}

bool PluginManager::GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
	auto plugin = FindPluginFromDescriptor(pluginDescriptor);
	if (plugin.has_value()) {
		pluginDependencies = plugin->get().GetDescriptor().dependencies;
		return true;
	}
	return false;
}