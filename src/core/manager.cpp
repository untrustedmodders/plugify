#include "plugify.hpp"
#include "manager.hpp"
#include "module.hpp"
#include "plugin.hpp"
#include "plugin_manifest.hpp"
#include "module_manifest.hpp"

#include <plugify/api/dependency.hpp>
#include <plugify/api/manager.hpp>
#include <plugify/api/plugify.hpp>
#include <util/json.hpp>

using namespace plugify;

Manager::Manager(Plugify& plugify) : Context(plugify) {
}

Manager::~Manager() {
	Terminate();
}

bool Manager::Initialize() {
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

void Manager::Terminate() {
	if (!IsInitialized())
		return;

	TerminateAllPlugins();
	TerminateAllModules();

	_inited = false;
}

bool Manager::IsInitialized() const {
	return _inited;
}

void Manager::Update(DateTime dt) {
	if (!IsInitialized())
		return;

	for (auto& module : _modules) {
		module.Update(dt);
	}

	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::Running) {
			plugin.GetModule()->UpdatePlugin(plugin, dt);
		}
	}
}

bool Manager::TopologicalSortPlugins() {
	if (_plugins.empty())
		return true; // Nothing to sort

	std::unordered_map<std::string, Plugin> pluginMap;
	std::unordered_map<std::string, size_t> inDegree;
	std::unordered_map<std::string, std::vector<std::string>> adjList;

	pluginMap.reserve(_plugins.size());
	inDegree.reserve(_plugins.size());
	adjList.reserve(_plugins.size());

	// Step 1: Build node map and initialize in-degrees
	for (Plugin& plugin : _plugins) {
		const std::string& name = plugin.GetName();
		inDegree.emplace(name, 0);
		pluginMap.emplace(name, std::move(plugin));
	}
	_plugins.clear();

	// Step 2: Build dependency graph
	for (const auto& [name, plugin] : pluginMap) {
		if (const auto& deps = plugin.GetManifest().dependencies) {
			for (const auto& dep : *deps) {
				if (pluginMap.contains(dep->name)) {
					adjList[dep->name].emplace_back(name);
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
		_plugins.emplace_back(std::move(std::get<Plugin>(*it)));
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
			_plugins.emplace_back(std::move(plugin));
		}

		return false;  // Cycles were found
	}

	return true;  // No cycles
}

void Manager::DiscoverAllModulesAndPlugins() {
	PL_ASSERT(_allModules.empty() && "Modules already initialized");
	PL_ASSERT(_allPlugins.empty() && "Plugins already initialized");

	if (!PartitionLocalPackages())
		return;

	if (!TopologicalSortPlugins()) {
		PL_LOG_VERBOSE("Cyclic dependencies found during plugin sorting.");
	}

	PL_LOG_VERBOSE("Plugins order after topological sorting by dependency: ");
	for (const auto& plugin : _plugins) {
		PL_LOG_VERBOSE("--> {}", plugin.GetName());
	}
}

template<typename T>
Manager::ManifestResult Manager::ReadManifest(const fs::path& path, ManifestType type) {
	auto fs = _plugify.GetFileSystem();
	auto json = fs->ReadTextFile(path);
	if (!json) {
		//return std::unexpected(json.error());
	}

	auto dest = glz::read_jsonc<std::shared_ptr<T>>(json);
	if (!dest.has_value()) {
		return std::unexpected(glz::format_error(dest.error(), json));
	}
	auto& manifest = *dest;
	manifest->path = path;
	manifest->type = type;

	if (!SupportsPlatform(manifest->platforms)) {
		return std::unexpected(std::string());
	}

	auto errors = manifest->Validate();
	if (!errors.empty()) {
		return std::unexpected(plg::join(errors, ", "));
	}

	return std::static_pointer_cast<Manifest>(std::move(manifest));
}

Manager::ManifestList Manager::FindLocalPackages() {
	std::unordered_map<std::string, ManifestReader> manifestLoaders = {
		{".pmodule", [](const fs::path& path) { return ReadManifest<ModuleManifest>(path, ManifestType::LanguageModule); }},
		{".pplugin", [](const fs::path& path) { return ReadManifest<PluginManifest>(path, ManifestType::Plugin); }}
		// Easy to add more types here
	};

	auto reader = _plugify.GetAssemblyLoader();

	ManifestList manifests;
	ScanDirectory(_plugify.GetConfig().baseDir, [&](const fs::path& path, int depth) {
		if (depth != 1)
			return;

		auto extension = path.extension().string();
		std::ranges::transform(extension, extension.begin(), ::tolower);
		auto itl = manifestLoaders.find(extension);
		if (itl == manifestLoaders.end())
			return;

		auto result = std::get<ManifestReader>(*itl)(path);
		if (!result) {
			if (!result.error().empty()) {
				PL_LOG_ERROR("Package: '{}' has error(s): {}", path.string(), result.error());
			}
			return;
		}
		auto& manifest = *result;

		auto it = std::ranges::find(manifests, manifest->name, &Manifest::name);
		if (it == manifests.end()) {
			manifests.emplace_back(std::move(manifest));
		} else {
			auto& existingManifest = *it;

			auto& existingVersion = existingManifest->version;
			if (existingVersion != manifest->version) {
				PL_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, manifest->version), manifest->name, std::min(existingVersion, manifest->version));

				if (existingVersion < manifest->version) {
					existingManifest = std::move(manifest);
				}
			} else {
				PL_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' - second location will be ignored.", existingVersion, manifest->name, manifest->path.string());
			}
		}
	}, 3);

	return manifests;
}

bool Manager::PartitionLocalPackages() {
	auto manifests = FindLocalPackages();
	if (manifests.empty()) {
		PL_LOG_WARNING("Did not find any package.");
		return false;
	}

	auto count = manifests.size();
	auto pluginCount = static_cast<size_t>(std::ranges::count(manifests, ManifestType::Plugin, &Manifest::type));
	_plugins.reserve(pluginCount);
	_modules.reserve(count - pluginCount);

	const auto& config = _plugify.GetConfig();
	for (auto& manifest : manifests) {
		BasePaths paths {
				.base = manifest->path.parent_path(),
				.configs = config.baseDir / config.configsDir,
				.data = config.baseDir / config.dataDir,
				.logs = config.baseDir / config.logsDir,
		};

		if (manifest->type == ManifestType::Plugin) {
			_plugins.emplace_back(static_cast<UniqueId>(_plugins.size()), std::move(paths), std::move(manifest));
		} else {
			_modules.emplace_back(static_cast<UniqueId>(_modules.size()), std::move(paths), std::move(manifest));
		}
	}

	if (_modules.empty()) {
		PL_LOG_WARNING("Did not find any module.");
		return false;
	}

	if (_plugins.empty()) {
		PL_LOG_WARNING("Did not find any plugin.");
		return false;
	}

	return true;
}

void Manager::LoadRequiredLanguageModules() {
	if (_modules.empty())
		return;

	std::unordered_set<UniqueId> modules;
	modules.reserve(_modules.size());

	for (auto& plugin : _plugins) {
		const auto& [name, constraits, optional] = plugin.GetManifest().language;
		auto it = std::ranges::find(_modules, name, &Module::GetLanguage);
		if (it == _modules.end()) {
			plugin.SetError(std::format("Language module: '{}' missing for plugin: '{}'", name, plugin.GetName()));
			continue;
		}
		auto& module = *it;
		plugin.Initialize(_plugify);
		plugin.SetModule(module);
		modules.emplace(module.GetId());
	}

	bool loadedAny = false;

	for (auto& module : _modules) {
		if (module.GetManifest().forceLoad.value_or(false) || modules.contains(module.GetId())) {
			loadedAny |= module.Initialize(_plugify);
		}
	}
	
	if (!loadedAny) {
		PL_LOG_WARNING("Did not load any module");
		return;
	}
}

void Manager::LoadAndStartAvailablePlugins() {
	if (_plugins.empty())
		return;
	
	bool loadedAny = false;
	
	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::NotLoaded) {
			if (plugin.GetModule()->GetState() != ModuleState::Loaded) {
				plugin.SetError(std::format("Language module: '{}' missing", plugin.GetModule()->GetName()));
				continue;
			}
			std::vector<std::string_view> names;
			if (const auto& dependencies = plugin.GetManifest().dependencies) {
				for (const auto& dependency: *dependencies) {
					auto dependencyPlugin = FindPlugin(dependency->name);
					if ((!dependencyPlugin || dependencyPlugin.GetState() != PluginState::Loaded) && !dependency->optional.value_or(false)) {
						names.emplace_back(dependency->name);
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

	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::Loaded) {
			for (const auto& module : _modules) {
				module.MethodExport(plugin);
			}
		}
	}

	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::Loaded) {
			plugin.GetModule()->StartPlugin(plugin);
		}
	}
}

void Manager::TerminateAllPlugins() {
	if (_plugins.empty())
		return;
	
	for (auto& plugin : std::ranges::reverse_view(_plugins)) {
		if (plugin.GetState() == PluginState::Running) {
			plugin.GetModule()->EndPlugin(plugin);
		}
	}

	for (auto& plugin : std::ranges::reverse_view(_plugins)) {
		plugin.Terminate();
	}

	_plugins.clear();
}

void Manager::TerminateAllModules() {
	if (_modules.empty())
		return;

	for (auto& module : std::ranges::reverse_view(_modules)) {
		module.Terminate();
	}

	_modules.clear();
}

ModuleHandle Manager::FindModule(std::string_view moduleName) const {
	auto it = std::ranges::find(_modules, moduleName, &Module::GetName);
	if (it != _modules.end())
		return *it;
	return {};
}

ModuleHandle Manager::FindModuleFromId(UniqueId moduleId) const {
	auto it = std::ranges::find(_modules, moduleId, &Module::GetId);
	if (it != _modules.end())
		return *it;
	return {};
}

std::vector<ModuleHandle> Manager::GetModules() const {
	std::vector<ModuleHandle> modules;
	modules.reserve(_modules.size());
	for (const auto& module : _modules)  {
		modules.emplace_back(module);
	}
	return modules;
}

PluginHandle Manager::FindPlugin(std::string_view pluginName) const {
	auto it = std::ranges::find(_plugins, pluginName, &Plugin::GetName);
	if (it != _plugins.end())
		return *it;
	return {};
}

PluginHandle Manager::FindPluginFromId(UniqueId pluginId) const {
	auto it = std::ranges::find(_plugins, pluginId, &Plugin::GetId);
	if (it != _plugins.end())
		return *it;
	return {};
}

std::vector<PluginHandle> Manager::GetPlugins() const {
	std::vector<PluginHandle> plugins;
	plugins.reserve(_plugins.size());
	for (const auto& plugin : _plugins)  {
		plugins.emplace_back(plugin);
	}
	return plugins;
}
