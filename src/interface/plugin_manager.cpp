#include <wizard/plugin_manager.h>
#include <core/plugin_manager.h>

using namespace wizard;

IPluginManager::IPluginManager(PluginManager& impl) : _impl{impl} {
}

bool IPluginManager::Initialize() const {
	return _impl.Initialize();
}

void IPluginManager::Terminate() const {
	_impl.Terminate();
}

bool IPluginManager::IsInitialized() const {
	return _impl.IsInitialized();
}

ModuleOpt IPluginManager::FindModule(const std::string& moduleName) const {
    return _impl.FindModule(moduleName);
}

ModuleOpt IPluginManager::FindModule(std::string_view moduleName) const {
    return _impl.FindModule(moduleName);
}

ModuleOpt IPluginManager::FindModuleFromId(UniqueId moduleId) const {
    return _impl.FindModuleFromId(moduleId);
}

ModuleOpt IPluginManager::FindModuleFromLang(const std::string& moduleLang) const {
    return _impl.FindModuleFromLang(moduleLang);
}

ModuleOpt IPluginManager::FindModuleFromPath(const fs::path& moduleFilePath) const {
    return _impl.FindModuleFromPath(moduleFilePath);
}

ModuleOpt IPluginManager::FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor) const {
    return _impl.FindModuleFromDescriptor(moduleDescriptor);
}

std::vector<ModuleRef> IPluginManager::GetModules() const {
    return _impl.GetModules();
}

PluginOpt IPluginManager::FindPlugin(const std::string& pluginName) const {
    return _impl.FindPlugin(pluginName);
}

PluginOpt IPluginManager::FindPlugin(std::string_view pluginName) const {
    return _impl.FindPlugin(pluginName);
}

PluginOpt IPluginManager::FindPluginFromId(UniqueId pluginId) const {
    return _impl.FindPluginFromId(pluginId);
}

PluginOpt IPluginManager::FindPluginFromPath(const fs::path& pluginFilePath) const {
    return _impl.FindPluginFromPath(pluginFilePath);
}

PluginOpt IPluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) const {
    return _impl.FindPluginFromDescriptor(pluginDescriptor);
}

std::vector<PluginRef> IPluginManager::GetPlugins() const {
    return _impl.GetPlugins();
}

bool IPluginManager::GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
    return _impl.GetPluginDependencies(pluginName, pluginDependencies);
}

bool IPluginManager::GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
    return _impl.GetPluginDependencies_FromFilePath(pluginFilePath, pluginDependencies);
}

bool IPluginManager::GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) const {
    return _impl.GetPluginDependencies_FromDescriptor(pluginDescriptor, pluginDependencies);
}
