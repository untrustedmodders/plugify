#include <core/plugify.hpp>
#include <plugify/api/plugify.hpp>

using namespace plugify;

bool PlugifyHandle::Initialize(const fs::path& rootDir) noexcept {
	return const_cast<Plugify*>(_impl)->Initialize(rootDir);
}

void PlugifyHandle::Terminate() noexcept {
	const_cast<Plugify*>(_impl)->Terminate();
}

bool PlugifyHandle::IsInitialized() const noexcept {
	return _impl->IsInitialized();
}

void PlugifyHandle::Update() noexcept {
	const_cast<Plugify*>(_impl)->Update();
}

void PlugifyHandle::SetLogger(std::shared_ptr<ILogger> logger) const noexcept {
	_impl->SetLogger(std::move(logger));
}

void PlugifyHandle::Log(std::string_view msg, Severity severity) const noexcept {
	_impl->Log(msg, severity);
}

ProviderHandle PlugifyHandle::GetProvider() const noexcept {
	return _impl->GetProvider();
}

ManagerHandle PlugifyHandle::GetManager() const noexcept {
	return _impl->GetManager();
}

const Config& PlugifyHandle::GetConfig() const noexcept {
	return _impl->GetConfig();
}

Version PlugifyHandle::GetVersion() const noexcept {
	return _impl->GetVersion();
}

void PlugifyHandle::SetAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) noexcept {
	const_cast<Plugify*>(_impl)->SetAssemblyLoader(std::move(loader));
}

std::shared_ptr<IAssemblyLoader> PlugifyHandle::GetAssemblyLoader() const noexcept {
	return _impl->GetAssemblyLoader();
}

void PlugifyHandle::SetFileSystem(std::shared_ptr<IFileSystem> reader) noexcept {
	const_cast<Plugify*>(_impl)->SetFileSystem(std::move(reader));
}

std::shared_ptr<IFileSystem> PlugifyHandle::GetFileSystem() const noexcept {
	return _impl->GetFileSystem();
}
