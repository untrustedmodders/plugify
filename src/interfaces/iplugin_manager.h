#pragma once

#include "iplugin.h"

namespace wizard {
    class PluginReferenceDescriptor;

    /**
     * Manager oversees and manages both loaded and unloaded code and content extensions.
     */
    class WIZARD_API IPluginManager {
    public:
        virtual ~IPluginManager() = default;

        /**
         * Finds information for a plugin.
         * @return Pointer to the plugin's information or nullptr.
         */
        virtual std::shared_ptr<IPlugin> FindPlugin(const std::string& pluginName) = 0;
        virtual std::shared_ptr<IPlugin> FindPlugin(std::string_view pluginName) = 0;

        virtual std::shared_ptr<IPlugin> FindPluginFromId(uint64_t pluginId) = 0;
        virtual std::shared_ptr<IPlugin> FindPluginFromPath(const fs::path& pluginFilePath) = 0;
        virtual std::shared_ptr<IPlugin> FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) = 0;

        /**
         * Gets an array of all the loaded plugins.
         * @return Array of the loaded plugins.
         */
        virtual std::vector<std::shared_ptr<IPlugin>> GetPlugins() = 0;

        /**
         * Tries to get a list of plugin dependencies for a given plugin. Returns false if the plugin provided was not found.
         */
        virtual bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) = 0;
        virtual bool GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) = 0;
        virtual bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) = 0;

    public:
        /**
         * Access singleton instance.
         * @return Reference to the singleton object.
         */
        static IPluginManager& Get();
    };

}
