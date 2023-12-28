#include <wizard/plugin_manager.h>
#include <core/plugin_manager.h>

using namespace wizard;

IPluginManager::IPluginManager(PluginManager& impl) : _impl{impl} {
}

std::shared_ptr<IPlugin> IPluginManager::FindPlugin(const std::string& pluginName) {
    return _impl.FindPlugin(pluginName);
}

std::shared_ptr<IPlugin> IPluginManager::FindPlugin(std::string_view pluginName) {
    return _impl.FindPlugin(pluginName);
}

std::shared_ptr<IPlugin> IPluginManager::FindPluginFromId(uint64_t pluginId) {
    return _impl.FindPluginFromId(pluginId);
}

std::shared_ptr<IPlugin> IPluginManager::FindPluginFromPath(const std::filesystem::path& pluginFilePath) {
    return _impl.FindPluginFromPath(pluginFilePath);
}

std::shared_ptr<IPlugin> IPluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) {
    return _impl.FindPluginFromDescriptor(pluginDescriptor);
}

std::vector<std::shared_ptr<IPlugin>> IPluginManager::GetPlugins() {
    return _impl.GetPlugins();
}

bool IPluginManager::GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    return _impl.GetPluginDependencies(pluginName, pluginDependencies);
}

bool IPluginManager::GetPluginDependencies_FromFilePath(const std::filesystem::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    return _impl.GetPluginDependencies_FromFilePath(pluginFilePath, pluginDependencies);
}

bool IPluginManager::GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    return _impl.GetPluginDependencies_FromDescriptor(pluginDescriptor, pluginDependencies);
}
