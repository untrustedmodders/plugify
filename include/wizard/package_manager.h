#pragma once

#include <filesystem>
#include <wizard_export.h>

namespace wizard {
	class PackageManager;

	// Package manager provided to user, which implemented in core
	class WIZARD_API IPackageManager {
	protected:
		explicit IPackageManager(PackageManager& impl);
		~IPackageManager() = default;

	public:
		void UpdatePackages();
		void InstallPackages(const std::filesystem::path& manifestFilePath, bool reinstall);
		void SnapshotPackages(const std::filesystem::path& manifestFilePath, bool prettify);

	private:
		PackageManager& _impl;
	};
}