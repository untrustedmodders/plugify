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
		void LoadLocalPackages();
		void LoadRemotePackages();

		void InstallPackage(const std::string& packageName);
		void InstallPackages(std::span<const std::string> packageNames);
		void InstallAllPackages(const fs::path& manifestFilePath, bool reinstall);
		void InstallAllPackages(const std::string& manifestUrl, bool reinstall);

		void UpdatePackage(const std::string& packageName);
		void UpdatePackages(std::span<const std::string> packageNames);
		void UpdateAllPackages();

		void UninstallPackage(const std::string& packageName);
		void UninstallPackages(std::span<const std::string> packageNames);
		void UninstallAllPackages();

		void SnapshotPackages(const fs::path& manifestFilePath, bool prettify) const;

	private:
		PackageManager& _impl;
	};
}