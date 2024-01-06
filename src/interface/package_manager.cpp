#include <wizard/package_manager.h>
#include <core/package_manager.h>

using namespace wizard;

IPackageManager::IPackageManager(PackageManager& impl) : _impl{impl} {
}

void IPackageManager::UpdatePackages() {
	return _impl.UpdatePackages();
}

void IPackageManager::InstallPackages(const fs::path& manifestFilePath) {
	return _impl.InstallPackages(manifestFilePath);
}

void IPackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) {
	return _impl.SnapshotPackages(manifestFilePath, prettify);
}