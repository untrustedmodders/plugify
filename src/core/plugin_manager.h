#pragma once

#include <wizard/plugin_manager.h>
#include <wizard/plugin.h>

namespace wizard {
    class Plugin;

    /**
     * PluginManager manages available code and content extensions (both loaded and not loaded.)
     */
    class PluginManager final : public IPluginManager {
    public:
        /** Constructor */
        PluginManager();

        /** Destructor */
        ~PluginManager() override;

        /** IPluginManager interface */
        void RefreshPluginsList() override;
        std::shared_ptr<IPlugin> FindPlugin(const std::string& pluginName) override;
        std::shared_ptr<IPlugin> FindPlugin(std::string_view pluginName) override;
        std::shared_ptr<IPlugin> FindPluginFromId(uint64_t pluginId) override;
        std::shared_ptr<IPlugin> FindPluginFromPath(const fs::path& pluginFilePath) override;
        std::shared_ptr<IPlugin> FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) override;
        std::vector<std::shared_ptr<IPlugin>> GetPlugins() override;
        bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) override;
        bool GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) override;
        bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) override;

    private:
        using PluginList = std::vector<std::shared_ptr<Plugin>>;
        using VisitedPluginMap = std::unordered_map<std::string, std::pair<bool, bool>>;

        /** Initiates a search for all plugins stored on the server, constructing an array of plugin objects without loading any of them.
            This process occurs upon the initial access of the plugin manager singleton. */
        void DiscoverAllPlugins();

        /** Reads all plugin descriptors. */
        void ReadAllPlugins();

        /** */
        void LoadRequiredLanguageModules();

        /** */
        static void SortPluginsByDependencies(const std::string& pluginName, PluginList& sourceList, PluginList& targetList);

        /** */
        static bool HasCyclicDependencies(PluginList& plugins);

        /** */
        static bool IsCyclic(const std::shared_ptr<Plugin>& plugin, PluginList& plugins, VisitedPluginMap& visitedPlugins);

    private:
        /** All of the plugins that we know about */
        PluginList allPlugins;
    };
}