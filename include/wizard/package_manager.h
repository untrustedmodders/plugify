#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <wizard_export.h>

namespace wizard {
	class PackageManager;
	class LocalPackage;
	class RemotePackage;

	using LocalPackageRef = std::optional<std::reference_wrapper<const LocalPackage>>;
	using RemotePackageRef = std::optional<std::reference_wrapper<const RemotePackage>>;

	// Package manager provided to user, which implemented in core
	class WIZARD_API IPackageManager {
	protected:
		explicit IPackageManager(PackageManager& impl);
		~IPackageManager() = default;

	public:
		void InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion = {});
		void InstallPackages(std::span<const std::string> packageNames);
		void InstallAllPackages(const fs::path& manifestFilePath, bool reinstall);
		void InstallAllPackages(const std::string& manifestUrl, bool reinstall);

		void UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion = {});
		void UpdatePackages(std::span<const std::string> packageNames);
		void UpdateAllPackages();

		void UninstallPackage(const std::string& packageName);
		void UninstallPackages(std::span<const std::string> packageNames);
		void UninstallAllPackages();

		void SnapshotPackages(const fs::path& manifestFilePath, bool prettify) const;

		LocalPackageRef FindLocalPackage(const std::string& packageName) const;
		RemotePackageRef FindRemotePackage(const std::string& packageName) const;

		std::vector<std::reference_wrapper<const LocalPackage>> GetLocalPackages() const;
		std::vector<std::reference_wrapper<const RemotePackage>> GetRemotePackages() const;

	private:
		PackageManager& _impl;
	};
}