#pragma once

#include "wizard_context.h"
#include <wizard/plugin_manager.h>
#include <wizard/plugin.h>
#include <wizard/language_module.h>

namespace wizard {
    class Plugin;
    class Module;
    class IWizard;

    class PluginManager final : public IPluginManager, public WizardContext {
    public:
        explicit PluginManager(std::weak_ptr<IWizard> wizard);
        ~PluginManager();

        /** IPluginManager interface */
        std::shared_ptr<IModule> FindModule(const std::string& moduleName);
        std::shared_ptr<IModule> FindModule(std::string_view moduleName);
        std::shared_ptr<IModule> FindModuleFromLang(const std::string& moduleLang);
        std::shared_ptr<IModule> FindModuleFromPath(const std::filesystem::path& moduleFilePath);
        std::shared_ptr<IModule> FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor);
        std::vector<std::shared_ptr<IModule>> GetModules();

        std::shared_ptr<IPlugin> FindPlugin(const std::string& pluginName);
        std::shared_ptr<IPlugin> FindPlugin(std::string_view pluginName);
        std::shared_ptr<IPlugin> FindPluginFromId(uint64_t pluginId);
        std::shared_ptr<IPlugin> FindPluginFromPath(const fs::path& pluginFilePath);
        std::shared_ptr<IPlugin> FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor);
        std::vector<std::shared_ptr<IPlugin>> GetPlugins();
        bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies);
        bool GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies);
        bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies);

    private:
        using PluginList = std::vector<std::shared_ptr<Plugin>>;
        using ModuleMap = std::unordered_map<std::string, std::shared_ptr<Module>>;
        using VisitedPluginMap = std::unordered_map<std::string, std::pair<bool, bool>>;

        void DiscoverAllPlugins();
        void DiscoverAllModules();
        void ReadAllPluginsDescriptors();

        void LoadRequiredLanguageModules();
        void LoadAndStartAvailablePlugins();
        void TerminateAllPlugins();

        static void SortPluginsByDependencies(const std::string& pluginName, PluginList& sourceList, PluginList& targetList);
        static bool HasCyclicDependencies(PluginList& plugins);
        static bool IsCyclic(const std::shared_ptr<Plugin>& plugin, PluginList& plugins, VisitedPluginMap& visitedPlugins);

        static bool IsSupportsPlatform(std::span<const std::string> supportedPlatforms) {
            return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), WIZARD_PLATFORM) != supportedPlatforms.end();
        }

    private:
        PluginList _allPlugins;
        ModuleMap _allModules;
    };
}