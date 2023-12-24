#pragma once

#include <string>
#include <string_view>
#include <filesystem>

namespace wizard {
    class IPlugin;
    struct PluginReferenceDescriptor;

    // Plugin manager provided to user, which implemented in core
    class WIZARD_API IPluginManager {
    protected:
        ~IPluginManager() = default;

    public:
        virtual std::shared_ptr<IPlugin> FindPlugin(const std::string& pluginName) = 0;
        virtual std::shared_ptr<IPlugin> FindPlugin(std::string_view pluginName) = 0;

        virtual std::shared_ptr<IPlugin> FindPluginFromId(uint64_t pluginId) = 0;
        virtual std::shared_ptr<IPlugin> FindPluginFromPath(const std::filesystem::path& pluginFilePath) = 0;
        virtual std::shared_ptr<IPlugin> FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) = 0;

        virtual std::vector<std::shared_ptr<IPlugin>> GetPlugins() = 0;

        virtual bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) = 0;
        virtual bool GetPluginDependencies_FromFilePath(const std::filesystem::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) = 0;
        virtual bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) = 0;
    };

}
