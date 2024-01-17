#include <wizard/package_manager.h>
#include <core/package_manager.h>

using namespace wizard;

IPackageManager::IPackageManager(PackageManager& impl) : _impl{impl} {
}

bool IPackageManager::Initialize() const {
	return _impl.Initialize();
}

void IPackageManager::Terminate() const {
	_impl.Terminate();
}

bool IPackageManager::IsInitialized() const {
	return _impl.IsInitialized();
}

void IPackageManager::InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion) const {
	_impl.InstallPackage(packageName, requiredVersion);
}

void IPackageManager::InstallPackages(std::span<const std::string> packageNames) const {
	_impl.InstallPackages(packageNames);
}

void IPackageManager::InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) const {
	_impl.InstallAllPackages(manifestFilePath, reinstall);
}

void IPackageManager::InstallAllPackages(const std::string& manifestUrl, bool reinstall) const {
	_impl.InstallAllPackages(manifestUrl, reinstall);
}

void IPackageManager::UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion) const {
	_impl.UpdatePackage(packageName, requiredVersion);
}

void IPackageManager::UpdatePackages(std::span<const std::string> packageNames) const {
	_impl.UpdatePackages(packageNames);
}

void IPackageManager::UpdateAllPackages() const {
	_impl.UpdateAllPackages();
}

void IPackageManager::UninstallPackage(const std::string& packageName) const {
	_impl.UninstallPackage(packageName);
}

void IPackageManager::UninstallPackages(std::span<const std::string> packageNames) const {
	_impl.UninstallPackages(packageNames);
}

void IPackageManager::UninstallAllPackages() const {
	_impl.UninstallAllPackages();
}

void IPackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) const {
	return _impl.SnapshotPackages(manifestFilePath, prettify);
}

bool IPackageManager::HasMissedPackages() const {
	return _impl.HasMissedPackages();
}

bool IPackageManager::HasConflictedPackages() const{
	return _impl.HasConflictedPackages();
}

void IPackageManager::InstallMissedPackages() const {
	return _impl.InstallMissedPackages();
}

void IPackageManager::UninstallConflictedPackages() const {
	return _impl.UninstallConflictedPackages();
}

LocalPackageOpt IPackageManager::FindLocalPackage(const std::string& packageName) const {
	return _impl.FindLocalPackage(packageName);
}

RemotePackageOpt IPackageManager::FindRemotePackage(const std::string& packageName) const {
	return _impl.FindRemotePackage(packageName);
}

std::vector<LocalPackageRef> IPackageManager::GetLocalPackages() const {
	return _impl.GetLocalPackages();
}

std::vector<RemotePackageRef> IPackageManager::GetRemotePackages() const {
	return _impl.GetRemotePackages();
}