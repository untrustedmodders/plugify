#include "plugin_manager.h"
#include "plugin.h"
#include "module.h"

#include <wizard/plugin_manager.h>
#include <wizard/wizard.h>
#include <utils/file_system.h>
#include <utils/json.h>

using namespace wizard;

PluginManager::PluginManager(std::weak_ptr<IWizard> wizard) : IPluginManager(*this), WizardContext(std::move(wizard)) {
	auto debugStart = DateTime::Now();

	DiscoverAllModules();
	DiscoverAllPlugins();
	LoadRequiredLanguageModules();
	LoadAndStartAvailablePlugins();

	WZ_LOG_DEBUG("PluginManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

PluginManager::~PluginManager() {
	TerminateAllPlugins();
	_allPlugins.clear();
	_allModules.clear();
}

void PluginManager::DiscoverAllPlugins() {
	// TODO: assert(_allPlugins.empty());

	//PluginSystem::GetAdditionalPluginPaths(pluginDiscoveryPaths);
	ReadAllPluginsDescriptors();

	PluginList sortedPlugins;
	sortedPlugins.reserve(_allPlugins.size());
	while (!_allPlugins.empty()) {
		SortPluginsByDependencies(_allPlugins.back()->GetName(), _allPlugins, sortedPlugins);
	}

	_allPlugins = std::move(sortedPlugins);

	WZ_LOG_VERBOSE("Plugins order after topological sorting: ");
	for (const auto& plugin : _allPlugins) {
		WZ_LOG_VERBOSE("{} - {}", plugin->GetName(), plugin->GetFriendlyName());
	}
}

template<typename Cnt, typename Pr = std::equal_to<typename Cnt::value_type>>
bool RemoveDuplicates(Cnt& cnt, Pr cmp = Pr()) {
	auto size = std::size(cnt);
	Cnt result;
	result.reserve(size);

	std::copy_if(
		std::make_move_iterator(std::begin(cnt)),
		std::make_move_iterator(std::end(cnt)),
		std::back_inserter(result),
		[&](const typename Cnt::value_type& what) {
			return std::find_if(std::begin(result), std::end(result), [&](const typename Cnt::value_type& existing) {
				return cmp(what, existing);
			}) == std::end(result);
		}
	);

	cnt = std::move(result);
	return std::size(cnt) != size;
}

void PluginManager::ReadAllPluginsDescriptors() {
	// TODO: Load .wpluginmanifest here

	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	bool strictMode = wizard->GetConfig().strictMode;

	FileSystem::ReadDirectory(wizard->GetConfig().baseDir / "plugins", [&](const fs::path& path, int depth) {
		// TODO: Add read from zip (zip should be on 1st depth)
		if (depth != 1)
			return;

		if (path.extension().string() != Plugin::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();
		WZ_LOG_INFO("Read plugin descriptor for '{}', from '{}'", name, path.string());

		auto json = FileSystem::ReadText(path);
		auto descriptor = glz::read_json<PluginDescriptor>(json);
		if (!descriptor.has_value()) {
			WZ_LOG_ERROR("Plugin descriptor: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
			return;
		}

		if (RemoveDuplicates(descriptor->dependencies)) {
			if (strictMode) {
				WZ_LOG_ERROR("Plugin descriptor: '{}' has multiple dependencies with same name!", name);
				return;
			}
		}

		if (RemoveDuplicates(descriptor->exportedMethods)) {
			if (strictMode) {
				WZ_LOG_ERROR("Plugin descriptor: '{}' has multiple method with same name!", name);
				return;
			}
		}

		if (IsSupportsPlatform(descriptor->supportedPlatforms)) {
			auto pluginAssemblyPath = path.parent_path();
			pluginAssemblyPath /= descriptor->assemblyPath;

			auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&name](const auto& plugin) {
				return plugin->GetName() == name;
			});
			if (it == _allPlugins.end()) {
				size_t index = _allPlugins.size();
				_allPlugins.emplace_back(std::make_unique<Plugin>(index, std::move(name), std::move(pluginAssemblyPath), std::move(*descriptor)));
			} else {
				auto& existingPlugin = *it;

				auto& existingVersion = existingPlugin->GetDescriptor().version;
				if (existingVersion != descriptor->version) {
					WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' plugin, over older version (v{}).", std::max(existingVersion, descriptor->version), name, std::min(existingVersion, descriptor->version));

					if (existingVersion < descriptor->version) {
						auto index = static_cast<size_t>(std::distance(_allPlugins.begin(), it));
						existingPlugin = std::make_unique<Plugin>(index, std::move(name), std::move(pluginAssemblyPath), std::move(*descriptor));
					}
				} else {
					WZ_LOG_WARNING("The same version (v{}) of plugin '{}' exists at '{}' and '{}' - second location will be ignored.", existingVersion, name, existingPlugin->GetFilePath().string(), path.string());
				}
			}
		}
	});
}

void PluginManager::DiscoverAllModules() {
	// TODO: Mount modules zips
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	FileSystem::ReadDirectory(wizard->GetConfig().baseDir / "modules", [&](const fs::path& path, int depth) {
		if (depth != 1)
			return;

		if (path.extension().string() != Module::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();
		WZ_LOG_INFO("Read module descriptor for '{}', from '{}'", name, path.string());

		auto json = FileSystem::ReadText(path);
		auto descriptor = glz::read_json<LanguageModuleDescriptor>(json);
		if (!descriptor.has_value()) {
			WZ_LOG_ERROR("Module descriptor: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
			return;
		}

		if (IsSupportsPlatform(descriptor->supportedPlatforms)) {

			// Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.

			auto moduleBinaryPath = path.parent_path();
			moduleBinaryPath /= "bin";
			moduleBinaryPath /= std::format(WIZARD_MODULE_PREFIX "{}" WIZARD_MODULE_SUFFIX, name);

			std::string lang{ descriptor->language };

			auto it = std::find_if(_allModules.begin(), _allModules.end(), [&lang](const auto& plugin) {
				return plugin->GetLanguage() == lang;
			});
			if (it == _allModules.end()) {
				_allModules.emplace_back(std::make_unique<Module>(std::move(name), std::move(lang), std::move(moduleBinaryPath), std::move(*descriptor)));
			} else {
				auto& existingModule = *it;

				auto& existingVersion = existingModule->GetDescriptor().version;
				if (existingVersion != descriptor->version) {
					WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' module, over older version (v{}).", std::max(existingVersion, descriptor->version), name, std::min(existingVersion, descriptor->version));

					if (existingVersion < descriptor->version) {
						existingModule = std::make_unique<Module>(std::move(name), std::move(lang), std::move(moduleBinaryPath), std::move(*descriptor));
					}
				} else {
					WZ_LOG_WARNING("The same version (v{}) of module '{}' exists at '{}' and '{}' - second location will be ignored.", existingVersion, name, existingModule->GetFilePath().string(), path.string());
				}
			}
		}
	});
}

void PluginManager::LoadRequiredLanguageModules() {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	std::unordered_set<std::string> modules;
	modules.reserve(_allModules.size());

	for (const auto& plugin : _allPlugins) {
		const auto& lang = plugin->GetDescriptor().languageModule.name;
		auto it = std::find_if(_allModules.begin(), _allModules.end(), [&lang](const auto& plugin) {
			return plugin->GetLanguage() == lang;
		});
		if (it == _allModules.end()) {
			plugin->SetError(std::format("Language module: '{}' missing for plugin: '{}'!", lang, plugin->GetFriendlyName()));
			continue;
		}
		auto& module = *it;
		plugin->SetModule(*module);
		modules.insert(module->GetLanguage());
	}

	for (const auto& module : _allModules) {
		if (module->GetDescriptor().forceLoad || modules.contains(module->GetLanguage())) {
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
			for (const auto&  module : _allModules) {
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
	for (auto i = static_cast<int64_t>(_allPlugins.size() - 1); i >= 0; --i) {
		const auto& plugin = _allPlugins[static_cast<uint64_t>(i)];
		if (plugin->GetState() == PluginState::Running) {
			plugin->GetModule().EndPlugin(*plugin);
		}
	}

	for (auto i = static_cast<int64_t>(_allPlugins.size() - 1); i >= 0; --i) {
		auto& plugin = _allPlugins[static_cast<uint64_t>(i)];
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

ModuleRef PluginManager::FindModule(const std::string& moduleName) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleName](const auto& module) {
		return module->GetName() == moduleName;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleRef PluginManager::FindModule(std::string_view moduleName) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleName](const auto& module) {
		return module->GetName() == moduleName;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleRef PluginManager::FindModuleFromLang(const std::string& moduleLang) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleLang](const auto& module) {
		return module->GetLanguage() == moduleLang;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleRef PluginManager::FindModuleFromPath(const std::filesystem::path& moduleFilePath) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleFilePath](const auto& module) {
		return module->GetFilePath() == moduleFilePath;
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

ModuleRef PluginManager::FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor) {
	auto it = std::find_if(_allModules.begin(), _allModules.end(), [&moduleDescriptor](const auto& module) {
		return module->GetName() == moduleDescriptor.name && (!moduleDescriptor.requestedVersion || module->GetDescriptor().version == moduleDescriptor.requestedVersion);
	});
	if (it != _allModules.end())
		return *(*it);
	return {};
}

std::vector<std::reference_wrapper<const IModule>> PluginManager::GetModules() {
	std::vector<std::reference_wrapper<const IModule>> modules;
	modules.reserve(_allModules.size());
	for (const auto& module : _allModules)  {
		modules.emplace_back(*module);
	}
	return modules;
}

PluginRef PluginManager::FindPlugin(const std::string& pluginName) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
		return plugin->GetName() == pluginName;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginRef PluginManager::FindPlugin(std::string_view pluginName) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
		return plugin->GetName() == pluginName;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginRef PluginManager::FindPluginFromId(uint64_t pluginId) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginId](const auto& plugin) {
		return plugin->GetId() == pluginId;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginRef PluginManager::FindPluginFromPath(const fs::path& pluginFilePath) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginFilePath](const auto& plugin) {
		return plugin->GetFilePath() == pluginFilePath;
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

PluginRef PluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) {
	auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginDescriptor](const auto& plugin) {
		return plugin->GetName() == pluginDescriptor.name && (!pluginDescriptor.requestedVersion || plugin->GetDescriptor().version == pluginDescriptor.requestedVersion);
	});
	if (it != _allPlugins.end())
		return *(*it);
	return {};
}

std::vector<std::reference_wrapper<const IPlugin>> PluginManager::GetPlugins() {
	std::vector<std::reference_wrapper<const IPlugin>> plugins;
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

bool PluginManager::GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
	auto plugin = FindPluginFromPath(pluginFilePath);
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