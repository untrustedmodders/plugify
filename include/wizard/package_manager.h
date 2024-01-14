#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <wizard_export.h>

namespace wizard {
	class PackageManager;
	struct LocalPackage;
	struct RemotePackage;

	using LocalPackageRef = std::reference_wrapper<const LocalPackage>;
	using RemotePackageRef = std::reference_wrapper<const RemotePackage>;
	using LocalPackageOpt = std::optional<LocalPackageRef>;
	using RemotePackageOpt = std::optional<RemotePackageRef>;

	// Package manager provided to user, which implemented in core
	class WIZARD_API IPackageManager {
	protected:
		explicit IPackageManager(PackageManager& impl);
		~IPackageManager() = default;

	public:
		bool Initialize();
		void Terminate();

		void InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion = {});
		void InstallPackages(std::span<const std::string> packageNames);
		void InstallAllPackages(const std::filesystem::path& manifestFilePath, bool reinstall);
		void InstallAllPackages(const std::string& manifestUrl, bool reinstall);

		void UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion = {});
		void UpdatePackages(std::span<const std::string> packageNames);
		void UpdateAllPackages();

		void UninstallPackage(const std::string& packageName);
		void UninstallPackages(std::span<const std::string> packageNames);
		void UninstallAllPackages();

		void SnapshotPackages(const std::filesystem::path& manifestFilePath, bool prettify) const;

		bool HasMissedPackages() const;
		bool HasConflictedPackages() const;
		void InstallMissedPackages();
		void UninstallConflictedPackages();

		LocalPackageOpt FindLocalPackage(const std::string& packageName) const;
		RemotePackageOpt FindRemotePackage(const std::string& packageName) const;

		std::vector<LocalPackageRef> GetLocalPackages() const;
		std::vector<RemotePackageRef> GetRemotePackages() const;

	private:
		PackageManager& _impl;
	};
}