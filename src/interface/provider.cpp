#include <core/provider.hpp>
#include <plugify/api/module.hpp>
#include <plugify/api/plugin.hpp>
#include <plugify/api/provider.hpp>

using namespace plugify;

void ProviderHandle::Log(std::string_view msg, Severity severity) const {
	_impl->Log(msg, severity);
}

const std::filesystem::path& ProviderHandle::GetBaseDir() const noexcept {
	return _impl->GetBaseDir();
}

const std::filesystem::path& ProviderHandle::GetConfigsDir() const noexcept {
	return _impl->GetConfigsDir();
}

const std::filesystem::path& ProviderHandle::GetDataDir() const noexcept {
	return _impl->GetDataDir();
}

const std::filesystem::path& ProviderHandle::GetLogsDir() const noexcept {
	return _impl->GetLogsDir();
}

bool ProviderHandle::IsPreferOwnSymbols() const noexcept {
	return _impl->IsPreferOwnSymbols();
}

bool ProviderHandle::IsPluginLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
	return _impl->IsPluginLoaded(name, constraint);
}

bool ProviderHandle::IsModuleLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
	return _impl->IsModuleLoaded(name, constraint);
}

PluginHandle ProviderHandle::FindPlugin(std::string_view name) const noexcept {
	return _impl->FindPlugin(name);
}

ModuleHandle ProviderHandle::FindModule(std::string_view name) const noexcept {
	return _impl->FindModule(name);
}

std::shared_ptr<IAssemblyLoader> ProviderHandle::GetAssemblyLoader() const noexcept {
	return _impl->GetAssemblyLoader();
}

std::shared_ptr<IFileSystem> ProviderHandle::GetFileSystem() const noexcept {
	return _impl->GetFileSystem();
}
