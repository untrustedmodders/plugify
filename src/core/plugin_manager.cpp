#include "plugin_manager.h"
#include "plugin.h"
#include "module.h"

#include "utils/file_system.h"

using namespace wizard;

PluginManager::PluginManager() {
    DiscoverAllModules();
    DiscoverAllPlugins();
    LoadRequiredLanguageModules();
    LoadAndStartAvailablePlugins();
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

    WIZARD_LOG("Plugins order after topological sorting: ", ErrorLevel::INFO);
    for ([[maybe_unused]] const auto& plugin : _allPlugins) {
        WIZARD_LOG(plugin->GetName() + " - " + plugin->GetFriendlyName(), ErrorLevel::INFO);
    }
}

void PluginManager::ReadAllPluginsDescriptors() {
    // TODO: Load .wpluginmanifest here

    std::vector<fs::path> pluginsFilePaths = FileSystem::GetFiles(Paths::PluginsDir(), true, PluginDescriptor::kFileExtension);

    for (const auto& path : pluginsFilePaths) {
        std::string name{ path.filename().replace_extension().string() };
        WIZARD_LOG("Read module descriptor for " + name + ", from '" + path.string() + "'", ErrorLevel::INFO);

        PluginDescriptor descriptor;
        if (descriptor.Load(path) && descriptor.IsSupportsPlatform(WIZARD_PLATFORM)) {
            fs::path pluginAssemblyPath{ path.parent_path() };
            pluginAssemblyPath /= descriptor.assemblyPath;

            if (!fs::exists(pluginAssemblyPath) || !fs::is_regular_file(pluginAssemblyPath)) {
                WIZARD_LOG("Plugin assembly '" + pluginAssemblyPath.string() + "' not exist!.", ErrorLevel::ERROR);
                continue;
            }

            auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&name](const auto& plugin) {
                return plugin->GetName() == name;
            });
            if (it == _allPlugins.end()) {
                size_t index = _allPlugins.size();
                _allPlugins.emplace_back(std::make_shared<Plugin>(index, std::move(name), std::move(pluginAssemblyPath), std::move(descriptor)));
            } else {
                const auto& existingPlugin = *it;

                int32_t existingVersion = existingPlugin->GetDescriptor().version;
                if (existingVersion != descriptor.version) {
                    WIZARD_LOG("By default, prioritizing newer version (v" + std::to_string(std::max(existingVersion, descriptor.version)) + ") of '" + name + "' plugin, over older version (v" + std::to_string(std::min(existingVersion, descriptor.version)) + ").", ErrorLevel::WARN);

                    if (existingVersion < descriptor.version) {
                        auto index = static_cast<size_t>(std::distance(_allPlugins.begin(), it));
                        _allPlugins[index] = std::make_shared<Plugin>(index, std::move(name), std::move(pluginAssemblyPath), std::move(descriptor));
                    }
                } else {
                    WIZARD_LOG("The same version (v" + std::to_string(existingVersion) + ") of plugin '"+ name + "' exists at '" + existingPlugin->GetFilePath().string() + "' and '" + path.string() + "' - second location will be ignored.", ErrorLevel::WARN);
                }
            }
        }
    }
}

void PluginManager::DiscoverAllModules() {
    // TODO: Mount modules zips
    // TODO: assert(_allModules.empty());

    std::vector<fs::path> modulesFilePaths = FileSystem::GetFiles(Paths::ModulesDir(), true, LanguageModuleDescriptor::kFileExtension);

    for (const auto& path : modulesFilePaths) {
        std::string name{ path.filename().replace_extension().string() };
        WIZARD_LOG("Read module descriptor for " + name + ", from " + path.string(), ErrorLevel::INFO);

        LanguageModuleDescriptor descriptor;
        if (descriptor.Load(path) && descriptor.IsSupportsPlatform(WIZARD_PLATFORM)) {

            // Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.

            fs::path moduleBinaryPath{ path.parent_path() };
            moduleBinaryPath /= "bin";
            moduleBinaryPath /= WIZARD_MODULE_PREFIX;
            moduleBinaryPath += name;
            moduleBinaryPath += WIZARD_MODULE_SUFFIX;

            if (!fs::exists(moduleBinaryPath) || !fs::is_regular_file(moduleBinaryPath)) {
                WIZARD_LOG("Module binary '" + moduleBinaryPath.string() + "' not exist!.", ErrorLevel::ERROR);
                continue;
            }

            auto it = _allModules.find(name);
            if (it == _allModules.end()) {
                _allModules.emplace(std::move(name), std::make_shared<Module>(std::move(moduleBinaryPath), std::move(descriptor)));
            } else {
                const auto& existingModule = it->second;

                int32_t existingVersion = existingModule->GetDescriptor().version;
                if (existingVersion != descriptor.version) {
                    WIZARD_LOG("By default, prioritizing newer version (v" + std::to_string(std::max(existingVersion, descriptor.version)) + ") of '" + name + "' module, over older version (v" + std::to_string(std::min(existingVersion, descriptor.version)) + ").", ErrorLevel::WARN);

                    if (existingVersion < descriptor.version) {
                        _allModules[std::move(name)] = std::make_shared<Module>(std::move(moduleBinaryPath), std::move(descriptor));
                    }
                } else {
                    WIZARD_LOG("The same version (v" + std::to_string(existingVersion) + ") of module '" + name + "' exists at '" + existingModule->GetFilePath().string() + "' and '" + path.string() + "' - second location will be ignored.", ErrorLevel::WARN);
                }
            }
        }
    }
}

void PluginManager::LoadRequiredLanguageModules() {
    std::set<std::shared_ptr<Module>> modules;
    for (const auto& plugin : _allPlugins) {
        const auto& name = plugin->GetDescriptor().languageModule.name;
        auto it = _allModules.find(name);
        if (it == _allModules.end()) {
            plugin->SetError("Language module: '" + name + "' missing!");
            continue;
        }
        auto& module = it->second;
        plugin->SetModule(module);
        modules.insert(module);
    }

    for (const auto& [_, languageModule] : _allModules) {
        if (languageModule->GetDescriptor().forceLoad || modules.contains(languageModule)) {
            languageModule->Initialize();
        }
    }
}

void PluginManager::LoadAndStartAvailablePlugins() {
    for (const auto& plugin : _allPlugins) {
        if (plugin->GetState() == PluginState::NotLoaded) {
            plugin->GetModule()->LoadPlugin(plugin);
        }
    }

    for (const auto& plugin : _allPlugins) {
        if (plugin->GetState() == PluginState::Loaded) {
            plugin->GetModule()->StartPlugin(plugin);
        }
    }
}

void PluginManager::TerminateAllPlugins() {
    for (const auto& plugin :  _allPlugins | std::views::reverse) {
        if (plugin->GetState() == PluginState::Running) {
            plugin->GetModule()->EndPlugin(plugin);
        }
    }

    for (auto& plugin : _allPlugins | std::views::reverse) {
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
        targetList.push_back(plugin);
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

bool PluginManager::IsCyclic(const std::shared_ptr<Plugin>& plugin, PluginList& plugins, VisitedPluginMap& visitedPlugins) {
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

std::shared_ptr<IPlugin> PluginManager::FindPlugin(const std::string& pluginName) {
    auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
        return plugin->GetName() == pluginName;
    });
    return it != _allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPlugin(std::string_view pluginName) {
    auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginName](const auto& plugin) {
        return plugin->GetName() == pluginName;
    });
    return it != _allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromId(uint64_t pluginId) {
    auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginId](const auto& plugin) {
        return plugin->GetId() == pluginId;
    });
    return it != _allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromPath(const fs::path& pluginFilePath) {
    auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginFilePath](const auto& plugin) {
        return plugin->GetFilePath() == pluginFilePath;
    });
    return it != _allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) {
    auto it = std::find_if(_allPlugins.begin(), _allPlugins.end(), [&pluginDescriptor](const auto& plugin) {
        return plugin->GetName() == pluginDescriptor.name && (!pluginDescriptor.requestedVersion || plugin->GetDescriptor().version == pluginDescriptor.requestedVersion);
    });
    return it != _allPlugins.end() ? *it : nullptr;
}

std::vector<std::shared_ptr<IPlugin>> PluginManager::GetPlugins() {
    std::vector<std::shared_ptr<IPlugin>> plugins;
    plugins.reserve(_allPlugins.size());
    for (const auto& plugin : _allPlugins)  {
        plugins.push_back(plugin);
    }
    return plugins;
}

bool PluginManager::GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    auto plugin = FindPlugin(pluginName);
    if (plugin) {
        pluginDependencies = plugin->GetDescriptor().dependencies;
        return true;
    }
    return false;
}

bool PluginManager::GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    auto plugin = FindPluginFromPath(pluginFilePath);
    if (plugin) {
        pluginDependencies = plugin->GetDescriptor().dependencies;
        return true;
    }
    return false;
}

bool PluginManager::GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    auto plugin = FindPluginFromDescriptor(pluginDescriptor);
    if (plugin) {
        pluginDependencies = plugin->GetDescriptor().dependencies;
        return true;
    }
    return false;
}