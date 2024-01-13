#include "plugin_manager.h"
#include "package_manager.h"
#include "plugin.h"
#include "module.h"

#include <wizard/plugin_manager.h>
#include <wizard/wizard.h>
#include <utils/json.h>

using namespace wizard;

PluginManager::PluginManager(std::weak_ptr<IWizard> wizard) : IPluginManager(*this), WizardContext(std::move(wizard)) {
}

PluginManager::~PluginManager() {
	Terminate_();
}

bool PluginManager::Initialize_() {
	if (!_allPlugins.empty() || !_allModules.empty())
		return false;

	auto debugStart = DateTime::Now();
	DiscoverAllModulesAndPlugins();
	LoadRequiredLanguageModules();
	LoadAndStartAvailablePlugins();
	WZ_LOG_DEBUG("PluginManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
	return true;
}

void PluginManager::Terminate_() {
	TerminateAllPlugins();
	_allPlugins.clear();
	_allModules.clear();
}

void PluginManager::DiscoverAllModulesAndPlugins() {
	WZ_ASSERT(_allModules.empty(), "Modules already initialized");
	WZ_ASSERT(_allPlugins.empty(), "Plugins already initialized");
	auto wizard = _wizard.lock();
	WZ_ASSERT(wizard);

	if (auto packageManager = wizard->GetPackageManager().lock()) {
		for (const auto& packageRef : packageManager->GetLocalPackages()) {
			const auto& package = packageRef.get();
			if (package.type == "plugin") {
				size_t id = _allPlugins.size();
				_allPlugins.emplace_back(std::make_unique<Plugin>(id, package));
			} else {
				size_t id = _allModules.size();
				_allModules.emplace_back(std::make_unique<Module>(id, package));
			}
		}
	}

	PluginList sortedPlugins;
	sortedPlugins.reserve(_allPlugins.size());
	while (!_allPlugins.empty()) {
		SortPluginsByDependencies(_allPlugins.back()->GetName(), _allPlugins, sortedPlugins);
	}

	_allPlugins = std::move(sortedPlugins);

	WZ_LOG_VERBOSE("Plugins order after topological sorting by dependency: ");
	for (const auto& plugin : _allPlugins) {
		WZ_LOG_VERBOSE("{} - {}", plugin->GetName(), plugin->GetFriendlyName());
	}
}

void PluginManager::LoadRequiredLanguageModules() {
	auto wizard = _wizard.lock();
	WZ_ASSERT(wizard);

	std::unordered_set<UniqueId> modules;
	modules.reserve(_allModules.size());

	for (const auto& plugin : _allPlugins) {
		const auto& lang = plugin->GetDescriptor().languageModule.name;
		auto it = std::find_if(_allModules.begin(), _allModules.end(), [&lang](const auto& plugin) {
			return plugin->GetLanguage() == lang;
		});
		if (it == _allModules.end()) {
			plugin->SetError(std::format("Language module: '{}' missing for plugin: '{}'", lang, plugin->GetFriendlyName()));
			continue;
		}
		auto& module = *it;
		plugin->SetModule(*module);
		modules.emplace(module->GetId());
	}

	for (const auto& module : _allModules) {
		if (module->GetDescriptor().forceLoad || modules.contains(module->GetId())) {
			module->Initialize(wizard->GetProvider());
		}
	}
}

void PluginManager::LoadAndStartAvailablePlugins() {
	for (const auto& plugin : _allPlugins) {
		if (plugin->GetState() == PluginState::NotLoaded) {
			std::vector<std::string_view> names;
			for (const auto& descriptor : plugin->GetDescriptor().dependencies) {
				auto dependencyPlugin = FindPlugin(descriptor.name);
				if ((!dependencyPlugin.has_value() || dependencyPlugin->get().GetState() != PluginState::Loaded) && !descriptor.optional) {
					names.emplace_back(descriptor.name);
				}
			}
			if (!names.empty()) {
				std::ostringstream error;
				error << "'" << names[0];
				for (auto it = std::next(names.begin()); it != names.end(); ++it) {
					error << "', '" << *it;
				}
				error << "'";
				plugin->SetError(std::format("Not loaded {} dependency plugin(s)", error.str()));
			} else {
				plugin->GetModule().LoadPlugin(*plugin);
			}
		}
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

ModuleOpt PluginManager::FindModule_(const std::string& moduleName) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleName](const auto& module) {
		return module->GetName() == moduleName;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModule_(std::string_view moduleName) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleName](const auto& module) {
		return module->GetName() == moduleName;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModuleFromId_(UniqueId moduleId) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleId](const auto& module) {
		return module->GetId() == moduleId;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModuleFromLang_(const std::string& moduleLang) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleLang](const auto& module) {
		return module->GetLanguage() == moduleLang;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModuleFromPath_(const std::filesystem::path& moduleFilePath) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleFilePath](const auto& module) {
		return module->GetFilePath() == moduleFilePath;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleOpt PluginManager::FindModuleFromDescriptor_(const PluginReferenceDescriptor& moduleDescriptor) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleDescriptor](const auto& module) {
		return module->GetName() == moduleDescriptor.name && (!moduleDescriptor.requestedVersion || module->GetDescriptor().version == moduleDescriptor.requestedVersion);
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

std::vector<ModuleRef> PluginManager::GetModules_() const {
	std::vector<ModuleRef> modules;
	modules.reserve(_allModules.size());
	for (const auto& module : _allModules)  {
		modules.emplace_back(*module);
	}
	return modules;
}

PluginOpt PluginManager::FindPlugin_(const std::string& pluginName) const {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
		return plugin->GetName() == pluginName;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginOpt PluginManager::FindPlugin_(std::string_view pluginName) const {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
		return plugin->GetName() == pluginName;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginOpt PluginManager::FindPluginFromId_(UniqueId pluginId) const {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginId](const auto& plugin) {
		return plugin->GetId() == pluginId;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginOpt PluginManager::FindPluginFromPath_(const fs::path& pluginFilePath) const {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginFilePath](const auto& plugin) {
		return plugin->GetFilePath() == pluginFilePath;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginOpt PluginManager::FindPluginFromDescriptor_(const PluginReferenceDescriptor& pluginDescriptor) const {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginDescriptor](const auto& plugin) {
		return plugin->GetName() == pluginDescriptor.name && (!pluginDescriptor.requestedVersion || plugin->GetDescriptor().version == pluginDescriptor.requestedVersion);
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

std::vector<PluginRef> PluginManager::GetPlugins_() const {
	std::vector<PluginRef> plugins;
	plugins.reserve(_allPlugins.size());
	for (const auto& plugin : _allPlugins)  {
		plugins.emplace_back(*plugin);
	}
	return plugins;
}

bool PluginManager::GetPluginDependencies_(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
	auto plugin = FindPlugin(pluginName);
	if (plugin.has_value()) {
		pluginDependencies = plugin->get().GetDescriptor().dependencies;
		return true;
	}
	return false;
}

bool PluginManager::GetPluginDependencies_FromFilePath_(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
	auto plugin = FindPluginFromPath(pluginFilePath);
	if (plugin.has_value()) {
		pluginDependencies = plugin->get().GetDescriptor().dependencies;
		return true;
	}
	return false;
}

bool PluginManager::GetPluginDependencies_FromDescriptor_(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
	auto plugin = FindPluginFromDescriptor(pluginDescriptor);
	if (plugin.has_value()) {
		pluginDependencies = plugin->get().GetDescriptor().dependencies;
		return true;
	}
	return false;
}