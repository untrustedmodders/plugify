#include <wizard/package_manager.h>
#include <core/package_manager.h>

using namespace wizard;

IPackageManager::IPackageManager(PackageManager& impl) : _impl{impl} {
}

void IPackageManager::SnapshotPackages(const fs::path& filepath, bool prettify) {
	return _impl.SnapshotPackages(filepath, prettify);
}