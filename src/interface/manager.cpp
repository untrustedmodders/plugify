#include <core/manager.hpp>
#include <plugify/api/module.hpp>
#include <plugify/api/manager.hpp>

using namespace plugify;

bool ManagerHandle::Initialize() noexcept {
	return const_cast<Manager*>(_impl)->Initialize();
}

void ManagerHandle::Terminate() noexcept {
	const_cast<Manager*>(_impl)->Terminate();
}

bool ManagerHandle::IsInitialized() const noexcept {
	return _impl->IsInitialized();
}

void ManagerHandle::Update(DateTime dt) noexcept {
	const_cast<Manager*>(_impl)->Update(dt);
}

ModuleHandle ManagerHandle::FindModule(std::string_view moduleName) const noexcept {
	return _impl->FindModule(moduleName);
}

ModuleHandle ManagerHandle::FindModuleFromId(UniqueId moduleId) const noexcept {
	return _impl->FindModuleFromId(moduleId);
}

std::vector<ModuleHandle> ManagerHandle::GetModules() const noexcept {
	return _impl->GetModules();
}

PluginHandle ManagerHandle::FindPlugin(std::string_view pluginName) const noexcept {
	return _impl->FindPlugin(pluginName);
}

PluginHandle ManagerHandle::FindPluginFromId(UniqueId pluginId) const noexcept {
	return _impl->FindPluginFromId(pluginId);
}

std::vector<PluginHandle> ManagerHandle::GetPlugins() const noexcept {
	return _impl->GetPlugins();
}
