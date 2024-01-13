#include <wizard/plugin_manager.h>
#include <core/plugin_manager.h>

using namespace wizard;

IPluginManager::IPluginManager(PluginManager& impl) : _impl{impl} {
}

ModuleRef IPluginManager::FindModule(const std::string& moduleName) const {
    return _impl.FindModule_(moduleName);
}

ModuleRef IPluginManager::FindModule(std::string_view moduleName) const {
    return _impl.FindModule_(moduleName);
}

ModuleRef IPluginManager::FindModuleFromId(UniqueId moduleId) const {
    return _impl.FindModuleFromId_(moduleId);
}

ModuleRef IPluginManager::FindModuleFromLang(const std::string& moduleLang) const {
    return _impl.FindModuleFromLang_(moduleLang);
}

ModuleRef IPluginManager::FindModuleFromPath(const fs::path& moduleFilePath) const {
    return _impl.FindModuleFromPath_(moduleFilePath);
}

ModuleRef IPluginManager::FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor) const {
    return _impl.FindModuleFromDescriptor_(moduleDescriptor);
}

std::vector<std::reference_wrapper<const IModule>> IPluginManager::GetModules() const {
    return _impl.GetModules_();
}

PluginRef IPluginManager::FindPlugin(const std::string& pluginName) const {
    return _impl.FindPlugin_(pluginName);
}

PluginRef IPluginManager::FindPlugin(std::string_view pluginName) const {
    return _impl.FindPlugin_(pluginName);
}

PluginRef IPluginManager::FindPluginFromId(UniqueId pluginId) const {
    return _impl.FindPluginFromId_(pluginId);
}

PluginRef IPluginManager::FindPluginFromPath(const fs::path& pluginFilePath) const {
    return _impl.FindPluginFromPath_(pluginFilePath);
}

PluginRef IPluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) const {
    return _impl.FindPluginFromDescriptor_(pluginDescriptor);
}

std::vector<std::reference_wrapper<const IPlugin>> IPluginManager::GetPlugins() const {
    return _impl.GetPlugins_();
}

bool IPluginManager::GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
    return _impl.GetPluginDependencies_(pluginName, pluginDependencies);
}

bool IPluginManager::GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
    return _impl.GetPluginDependencies_FromFilePath_(pluginFilePath, pluginDependencies);
}

bool IPluginManager::GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
    return _impl.GetPluginDependencies_FromDescriptor_(pluginDescriptor, pluginDependencies);
}
