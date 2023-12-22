#include "plugin_manager.h"
#include "plugin.h"

using namespace wizard;

PluginManager::PluginManager() {
    DiscoverAllPlugins();
    LoadRequiredLanguageModules();
}

PluginManager::~PluginManager() {
    // NOTE: All plugins and modules should be cleaned up or abandoned by this point
}

void PluginManager::DiscoverAllPlugins() {
    assert(allPlugins.empty());

    //PluginSystemDefs::GetAdditionalPluginPaths(pluginDiscoveryPaths);
    ReadAllPlugins();

    PluginList sortedPlugins;
    sortedPlugins.reserve(allPlugins.size());
    while (!allPlugins.empty()) {
        SortPluginsByDependencies(allPlugins.back()->m_name, allPlugins, sortedPlugins);
    }
    allPlugins = std::move(sortedPlugins);
}

void PluginManager::ReadAllPlugins() {
    // Find any plugin manifest files. These give us the plugin list (and their descriptors) without needing to scour the directory tree.
    std::vector<fs::path> manifestFilePaths;
    //FindPluginManifestsInDirectory(Paths::PluginsDir(), manifestFilePaths);

    // track child plugins that don't want to go into main plugin set
    std::vector<std::shared_ptr<Plugin>> childPlugins;

    // If we didn't find any manifests, do a recursive search for plugins
    if (manifestFilePaths.empty()) {
        WIZARD_LOG("No *.wpluginmanifest files found, looking for *.wplugin files instead.", ErrorLevel::INFO);
    }
    else {
        // TODO: Load & download plugins from manifest
    }
}

void PluginManager::LoadRequiredLanguageModules() {
    std::set<std::string> languageModules;
    for (const auto& plugin : allPlugins) {
        languageModules.insert(plugin->m_descriptor.languageModule.name);
    }

    // Find any module files. These give us the plugin list (and their descriptors) without needing to scour the directory tree.
    /*std::vector<fs::path> modulesFilePaths;
    FindLanguageModulesInDirectory(Paths::ModulesDir(), modulesFilePaths);

    if (!modulesFilePaths.empty()) {
        for (const auto& path : modulesFilePaths)  {
        }
    }*/
}

void PluginManager::SortPluginsByDependencies(const std::string& pluginName, PluginList& sourceList, PluginList& targetList) {
    auto it = std::find_if(sourceList.begin(), sourceList.end(), [&pluginName](const auto& plugin) {
        return plugin->m_name == pluginName;
    });
    if (it != sourceList.end()) {
        auto index = static_cast<size_t>(std::distance(sourceList.begin(), it));
        auto plugin = sourceList[index];
        sourceList.erase(it);
        for (const auto& pluginDescriptor : plugin->m_descriptor.dependencies) {
            SortPluginsByDependencies(pluginDescriptor.name, sourceList, targetList);
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
        if (!data[plugin->m_name].first && IsCyclic(plugin, plugins, data))
            return true;
    }

    return false;
}

bool PluginManager::IsCyclic(const std::shared_ptr<Plugin>& plugin, PluginList& plugins, VisitedPluginMap& visitedPlugins) {
    auto& [visited, recursive] = visitedPlugins[plugin->m_name];
    if (!visited) {
        // Mark the current node as visited
        // and part of recursion stack
        visited = true;
        recursive = true;

        // Recur for all the vertices adjacent to this vertex
        for (const auto& pluginDescriptor : plugin->m_descriptor.dependencies) {
            auto& name = pluginDescriptor.name;

            auto it = std::find_if(plugins.begin(), plugins.end(), [&name](const auto& p) {
                return p->m_name == name;
            });

            if (it != plugins.end()) {
                auto [vis, rec] = visitedPlugins[name];
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
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginName](const auto& plugin) {
        return plugin->m_name == pluginName;
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPlugin(std::string_view pluginName) {
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginName](const auto& plugin) {
        return plugin->m_name == pluginName;
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromId(uint64_t pluginId) {
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginId](const auto& plugin) {
        return plugin->m_id == pluginId;
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromPath(const fs::path& pluginFilePath) {
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginFilePath](const auto& plugin) {
        return plugin->m_filePath == pluginFilePath;
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) {
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginDescriptor](const auto& plugin) {
        return plugin->m_name == pluginDescriptor.name && (!pluginDescriptor.requestedVersion || plugin->m_descriptor.version == pluginDescriptor.requestedVersion);
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::vector<std::shared_ptr<IPlugin>> PluginManager::GetPlugins() {
    std::vector<std::shared_ptr<IPlugin>> plugins;
    plugins.reserve(allPlugins.size());
    for (const auto& plugin : allPlugins)  {
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

IPluginManager& IPluginManager::Get() {
    static PluginManager pluginManager;
    return pluginManager;
}