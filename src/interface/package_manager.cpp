#include <wizard/package_manager.h>
#include <core/package_manager.h>

using namespace wizard;

IPackageManager::IPackageManager(PackageManager& impl) : _impl{impl} {
}

bool IPackageManager::Initialize() {
	return _impl.Initialize_();
}

void IPackageManager::Terminate() {
	_impl.Terminate_();
}

void IPackageManager::InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion) {
	_impl.InstallPackage_(packageName, requiredVersion);
}

void IPackageManager::InstallPackages(std::span<const std::string> packageNames) {
	_impl.InstallPackages_(packageNames);
}

void IPackageManager::InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) {
	_impl.InstallAllPackages_(manifestFilePath, reinstall);
}

void IPackageManager::InstallAllPackages(const std::string& manifestUrl, bool reinstall) {
	_impl.InstallAllPackages_(manifestUrl, reinstall);
}

void IPackageManager::UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion) {
	_impl.UpdatePackage_(packageName, requiredVersion);
}

void IPackageManager::UpdatePackages(std::span<const std::string> packageNames) {
	_impl.UpdatePackages_(packageNames);
}

void IPackageManager::UpdateAllPackages() {
	_impl.UpdateAllPackages_();
}

void IPackageManager::UninstallPackage(const std::string& packageName) {
	_impl.UninstallPackage_(packageName);
}

void IPackageManager::UninstallPackages(std::span<const std::string> packageNames) {
	_impl.UninstallPackages_(packageNames);
}

void IPackageManager::UninstallAllPackages() {
	_impl.UninstallAllPackages_();
}

void IPackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) const {
	return _impl.SnapshotPackages_(manifestFilePath, prettify);
}

bool IPackageManager::HasMissedPackages() const {
	return _impl.HasMissedPackages_();
}

bool IPackageManager::HasConflictedPackages() const{
	return _impl.HasConflictedPackages_();
}

void IPackageManager::InstallMissedPackages(){
	return _impl.InstallMissedPackages_();
}

void IPackageManager::UninstallConflictedPackages(){
	return _impl.UninstallConflictedPackages_();
}

LocalPackageOpt IPackageManager::FindLocalPackage(const std::string& packageName) const {
	return _impl.FindLocalPackage_(packageName);
}

RemotePackageOpt IPackageManager::FindRemotePackage(const std::string& packageName) const {
	return _impl.FindRemotePackage_(packageName);
}

std::vector<LocalPackageRef> IPackageManager::GetLocalPackages() const {
	return _impl.GetLocalPackages_();
}

std::vector<RemotePackageRef> IPackageManager::GetRemotePackages() const {
	return _impl.GetRemotePackages_();
}