#include "plugin_manager.h"
#include "plugin.h"
#include "module.h"

#include "utils/file_system.h"

using namespace wizard;

PluginManager::PluginManager() {
    DiscoverAllPlugins();
    DiscoverAllModules();
    LoadRequiredLanguageModules();
}

PluginManager::~PluginManager() {
    // NOTE: All plugins and modules should be cleaned up or abandoned by this point
}

void PluginManager::DiscoverAllPlugins() {
    assert(allPlugins.empty());

    //PluginSystem::GetAdditionalPluginPaths(pluginDiscoveryPaths);
    ReadAllPlugins();

    PluginList sortedPlugins;
    sortedPlugins.reserve(allPlugins.size());
    while (!allPlugins.empty()) {
        SortPluginsByDependencies(allPlugins.back()->GetName(), allPlugins, sortedPlugins);
    }
    allPlugins = std::move(sortedPlugins);
}

void PluginManager::ReadAllPlugins() {
    // TODO: Load .wpluginmanifest here
    std::vector<fs::path> pluginsFilePaths = FileSystem::GetFiles(Paths::ModulesDir(), true, PluginDescriptor::FileExtension);
}

void PluginManager::DiscoverAllModules() {
    assert(allModules.empty());

    std::vector<fs::path> modulesFilePaths = FileSystem::GetFiles(Paths::ModulesDir(), true, LanguageModuleDescriptor::FileExtension);

    for (const auto& path : modulesFilePaths) {
        LanguageModuleDescriptor languageModuleDescriptor;
        if (languageModuleDescriptor.Load(path) && languageModuleDescriptor.IsSupportsPlatform(WIZARD_PLATFORM)) {
            // Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.

            std::string name{ path.filename().replace_extension().string() };
            if (allModules.contains(name)) {
                WIZARD_LOG("Cannot load module. '" + name + "' already was loaded.", ErrorLevel::WARN);
                continue;
            }

            fs::path moduleBinaryPath{ path.parent_path() };
            moduleBinaryPath /= "bin";
            moduleBinaryPath /= WIZARD_MODULE_PREFIX;
            moduleBinaryPath += name;
            moduleBinaryPath += WIZARD_MODULE_SUFFIX;

            if (fs::exists(moduleBinaryPath) && fs::is_regular_file(moduleBinaryPath)) {
                allModules.emplace(name, std::make_shared<Module>(moduleBinaryPath, languageModuleDescriptor));
            } else {
                WIZARD_LOG("Module binary '" + moduleBinaryPath.string() + "' not exist!.", ErrorLevel::WARN);
            }
        }
    }
}

void PluginManager::LoadRequiredLanguageModules() {
    std::set<std::string> languageModules;
    for (const auto& plugin : allPlugins) {
        languageModules.insert(plugin->GetDescriptor().languageModule.name);
    }

    // TODO: Initialize modules here
}

void PluginManager::SortPluginsByDependencies(const std::string& pluginName, PluginList& sourceList, PluginList& targetList) {
    auto it = std::find_if(sourceList.begin(), sourceList.end(), [&pluginName](const auto& plugin) {
        return plugin->GetName() == pluginName;
    });
    if (it != sourceList.end()) {
        auto index = static_cast<size_t>(std::distance(sourceList.begin(), it));
        auto plugin = sourceList[index];
        sourceList.erase(it);
        for (const auto& pluginDescriptor : plugin->GetDescriptor().dependencies) {
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
        for (const auto& pluginDescriptor : plugin->GetDescriptor().dependencies) {
            const auto& name = pluginDescriptor.name;

            auto it = std::find_if(plugins.begin(), plugins.end(), [&name](const auto& p) {
                return p->GetName() == name;
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
        return plugin->GetName() == pluginName;
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPlugin(std::string_view pluginName) {
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginName](const auto& plugin) {
        return plugin->GetName() == pluginName;
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromId(uint64_t pluginId) {
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginId](const auto& plugin) {
        return plugin->GetId() == pluginId;
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromPath(const fs::path& pluginFilePath) {
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginFilePath](const auto& plugin) {
        return plugin->GetDescriptorFilePath() == pluginFilePath;
    });
    return it != allPlugins.end() ? *it : nullptr;
}

std::shared_ptr<IPlugin> PluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) {
    auto it = std::find_if(allPlugins.begin(), allPlugins.end(), [&pluginDescriptor](const auto& plugin) {
        return plugin->GetName() == pluginDescriptor.name && (!pluginDescriptor.requestedVersion || plugin->GetDescriptor().version == pluginDescriptor.requestedVersion);
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

void PluginManager::RefreshPluginsList() {
    // TODO:
}

IPluginManager& IPluginManager::Get() {
    static PluginManager pluginManager;
    return pluginManager;
}