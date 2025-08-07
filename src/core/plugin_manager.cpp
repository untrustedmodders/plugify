#include "plugin_manager.hpp"
#include "package_manager.hpp"
#include "module.hpp"
#include "plugin.hpp"

#include <plugify/plugify.hpp>
#include <plugify/plugin_descriptor.hpp>
#include <plugify/plugin_manager.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <utils/json.hpp>

using namespace plugify;

PluginManager::PluginManager(std::weak_ptr<IPlugify> plugify) : PlugifyContext(std::move(plugify)) {
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

	_inited = true;

	PL_LOG_DEBUG("PluginManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
	return true;
}

void PluginManager::Terminate() {
	if (!IsInitialized())
		return;

	TerminateAllPlugins();
	TerminateAllModules();

	_inited = false;
}

bool PluginManager::IsInitialized() const {
	return _inited;
}

void PluginManager::Update(DateTime dt) {
	if (!IsInitialized())
		return;

	for (auto& module : _allModules) {
		module.Update(dt);
	}

	for (auto& plugin : _allPlugins) {
		if (plugin.GetState() == PluginState::Running) {
			plugin.GetModule()->UpdatePlugin(plugin, dt);
		}
	}
}

bool PluginManager::TopologicalSortPlugins() {
	if (_allPlugins.empty())
		return true; // Nothing to sort

	std::unordered_map<std::string, Plugin> pluginMap;
	std::unordered_map<std::string, size_t> inDegree;
	std::unordered_map<std::string, std::vector<std::string>> adjList;

	pluginMap.reserve(_allPlugins.size());
	inDegree.reserve(_allPlugins.size());
	adjList.reserve(_allPlugins.size());

	// Step 1: Build node map and initialize in-degrees
	for (Plugin& plugin : _allPlugins) {
		const std::string& name = plugin.GetName();
		inDegree.emplace(name, 0);
		pluginMap.emplace(name, std::move(plugin));
	}
	_allPlugins.clear();

	// Step 2: Build dependency graph
	for (const auto& [name, plugin] : pluginMap) {
		if (const auto& deps = plugin.GetDescriptor().dependencies) {
			for (const auto& dep : *deps) {
				if (pluginMap.contains(dep.name)) {
					adjList[dep.name].emplace_back(name);
					++inDegree[name];
				}
			}
		}
	}

	// Step 3: Collect plugins with no dependencies
	std::queue<std::string> ready;
	for (const auto& [name, degree] : inDegree) {
		if (degree == 0) {
			ready.push(name);
		}
	}

	// Step 4: Kahn's sort
	while (!ready.empty()) {
		const std::string current = std::move(ready.front());
		ready.pop();

		auto it = pluginMap.find(current);
		_allPlugins.emplace_back(std::move(std::get<Plugin>(*it)));
		pluginMap.erase(it);

		for (const auto& dependent : adjList[current]) {
			if (--inDegree[dependent] == 0) {
				ready.push(dependent);
			}
		}
	}

	// Step 5: Handle cyclic nodes
	if (!pluginMap.empty()) {
		for (auto& [name, plugin] : pluginMap) {
			PL_LOG_WARNING("Plugin '{}' is involved in a cyclic dependency and was excluded from the ordered load.", name);
			_allPlugins.emplace_back(std::move(plugin));
		}

		return false;  // Cycles were found
	}

	return true;  // No cycles
}

void PluginManager::DiscoverAllModulesAndPlugins() {
	PL_ASSERT(_allModules.empty() && "Modules already initialized");
	PL_ASSERT(_allPlugins.empty() && "Plugins already initialized");

	if (!PartitionLocalPackages())
		return;

	if (!TopologicalSortPlugins()) {
		PL_LOG_VERBOSE("Cyclic dependencies found during plugin sorting.");
	}

	PL_LOG_VERBOSE("Plugins order after topological sorting by dependency: ");
	for (const auto& plugin : _allPlugins) {
		PL_LOG_VERBOSE("{} - {}", plugin.GetName(), plugin.GetFriendlyName());
	}
}

bool PluginManager::PartitionLocalPackages() {
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	if (auto packageManager = plugify->GetPackageManager().lock()) {
		auto localPackages = packageManager->GetLocalPackages();
		auto count = localPackages.size();

		auto pluginCount = static_cast<size_t>(std::count_if(localPackages.begin(), localPackages.end(), [](const auto& param) {
			return param->type == PackageType::Plugin;
		}));
		_allPlugins.reserve(pluginCount);
		_allModules.reserve(count - pluginCount);

		const auto& config = plugify->GetConfig();
		const BasePaths paths {
				.configs = config.baseDir / config.configsDir,
				.data = config.baseDir / config.dataDir,
				.logs = config.baseDir / config.logsDir,
		};

		for (const auto& package : localPackages) {
			if (package->type == PackageType::Plugin) {
				_allPlugins.emplace_back(static_cast<UniqueId>(_allPlugins.size()), *package, paths);
			} else {
				_allModules.emplace_back(static_cast<UniqueId>(_allModules.size()), *package);
			}
		}
	}

	if (_allModules.empty()) {
		PL_LOG_WARNING("Did not find any module.");
		return false;
	}

	if (_allPlugins.empty()) {
		PL_LOG_WARNING("Did not find any plugin.");
		return false;
	}

	return true;
}

void PluginManager::LoadRequiredLanguageModules() {
	if (_allModules.empty())
		return;
	
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);
	auto provider = plugify->GetProvider().lock();
	PL_ASSERT(provider);

	std::unordered_set<UniqueId> modules;
	modules.reserve(_allModules.size());

	for (auto& plugin : _allPlugins) {
		const auto& [name, version] = plugin.GetDescriptor().languageModule;
		auto it = std::find_if(_allModules.begin(), _allModules.end(), [&name, &version](const auto& p) {
			return p.GetLanguage() == name && (!version || p.GetDescriptor().version >= version);
		});
		if (it == _allModules.end()) {
			plugin.SetError(std::format("Language module: '{}' (v{}) missing for plugin: '{}'", name, version.has_value() ? version->to_string() : "[latest]", plugin.GetFriendlyName()));
			continue;
		}
		auto& module = *it;
		plugin.Initialize(provider);
		plugin.SetModule(module);
		modules.emplace(module.GetId());
	}
	
	bool loadedAny = false;

	for (auto& module : _allModules) {
		if (module.GetDescriptor().forceLoad.value_or(false) || modules.contains(module.GetId())) {
			loadedAny |= module.Initialize(provider);
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
	
	for (auto& plugin : _allPlugins) {
		if (plugin.GetState() == PluginState::NotLoaded) {
			if (plugin.GetModule()->GetState() != ModuleState::Loaded) {
				plugin.SetError(std::format("Language module: '{}' missing", plugin.GetModule()->GetFriendlyName()));
				continue;
			}
			std::vector<std::string_view> names;
			if (const auto& dependencies = plugin.GetDescriptor().dependencies) {
				for (const auto& dependency: *dependencies) {
					auto dependencyPlugin = FindPlugin(dependency.name);
					if ((!dependencyPlugin || dependencyPlugin.GetState() != PluginState::Loaded) && !dependency.optional.value_or(false)) {
						names.emplace_back(dependency.name);
					}
				}
			}
			if (!names.empty()) {
				plugin.SetError(std::format("Not loaded {} dependency plugin(s)", plg::join(names, ", ")));
			} else {
				loadedAny |= plugin.GetModule()->LoadPlugin(plugin);
			}
		}
	}
	
	if (!loadedAny) {
		PL_LOG_WARNING("Did not load any plugin");
		return;
	}

	for (auto& plugin : _allPlugins) {
		if (plugin.GetState() == PluginState::Loaded) {
			for (const auto& module : _allModules) {
				module.MethodExport(plugin);
			}
		}
	}

	for (auto& plugin : _allPlugins) {
		if (plugin.GetState() == PluginState::Loaded) {
			plugin.GetModule()->StartPlugin(plugin);
		}
	}
}

void PluginManager::TerminateAllPlugins() {
	if (_allPlugins.empty())
		return;
	
	for (auto it = _allPlugins.rbegin(); it != _allPlugins.rend(); ++it) {
		auto& plugin = *it;
		if (plugin.GetState() == PluginState::Running) {
			plugin.GetModule()->EndPlugin(plugin);
		}
	}

	for (auto it = _allPlugins.rbegin(); it != _allPlugins.rend(); ++it) {
		auto& plugin = *it;
		plugin.Terminate();
	}

	_allPlugins.clear();
}

void PluginManager::TerminateAllModules() {
	if (_allModules.empty())
		return;

	for (auto it = _allModules.rbegin(); it != _allModules.rend(); ++it) {
		auto& module = *it;
		module.Terminate();
	}

	_allModules.clear();
}

ModuleHandle PluginManager::FindModule(std::string_view moduleName) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleName](const auto& module) {
		return module.GetName() == moduleName;
	});
	if (it != _allModules.end())
		return *it;
	return {};
}

ModuleHandle PluginManager::FindModuleFromId(UniqueId moduleId) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleId](const auto& module) {
		return module.GetId() == moduleId;
	});
	if (it != _allModules.end())
		return *it;
	return {};
}

ModuleHandle PluginManager::FindModuleFromLang(std::string_view moduleLang) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleLang](const auto& module) {
		return module.GetLanguage() == moduleLang;
	});
	if (it != _allModules.end())
		return *it;
	return {};
}

ModuleHandle PluginManager::FindModuleFromPath(const fs::path& moduleFilePath) const {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleFilePath](const auto& module) {
		return module.GetFilePath() == moduleFilePath;
	});
	if (it != _allModules.end())
		return *it;
	return {};
}

std::vector<ModuleHandle> PluginManager::GetModules() const {
	std::vector<ModuleHandle> modules;
	modules.reserve(_allModules.size());
	for (const auto& module : _allModules)  {
		modules.emplace_back(module);
	}
	return modules;
}

PluginHandle PluginManager::FindPlugin(std::string_view pluginName) const {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
		return plugin.GetName() == pluginName;
	});
	if (it != _allPlugins.end())
		return *it;
	return {};
}

PluginHandle PluginManager::FindPluginFromId(UniqueId pluginId) const {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginId](const auto& plugin) {
		return plugin.GetId() == pluginId;
	});
	if (it != _allPlugins.end())
		return *it;
	return {};
}

PluginHandle PluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptorHandle& pluginDescriptor) const {
	auto name = pluginDescriptor.GetName();
	auto version = pluginDescriptor.GetVersion();
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&name, &version](const auto& plugin) {
		return plugin.GetName() == name && (!version || plugin.GetDescriptor().version >= version);
	});
	if (it != _allPlugins.end())
		return *it;
	return {};
}

std::vector<PluginHandle> PluginManager::GetPlugins() const {
	std::vector<PluginHandle> plugins;
	plugins.reserve(_allPlugins.size());
	for (const auto& plugin : _allPlugins)  {
		plugins.emplace_back(plugin);
	}
	return plugins;
}
