#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <plugify_export.h>

namespace plugify {
	class PackageManager;
	struct LocalPackage;
	struct RemotePackage;

	using LocalPackageRef = std::reference_wrapper<const LocalPackage>;
	using RemotePackageRef = std::reference_wrapper<const RemotePackage>;
	using LocalPackageOpt = std::optional<LocalPackageRef>;
	using RemotePackageOpt = std::optional<RemotePackageRef>;

	// Package manager provided to user, which implemented in core
	class PLUGIFY_API IPackageManager {
	protected:
		explicit IPackageManager(PackageManager& impl);
		~IPackageManager() = default;

	public:
		bool Initialize() const;
		void Terminate() const;
		bool IsInitialized() const;

		void InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion = {}) const;
		void InstallPackages(std::span<const std::string> packageNames) const;
		void InstallAllPackages(const std::filesystem::path& manifestFilePath, bool reinstall) const;
		void InstallAllPackages(const std::string& manifestUrl, bool reinstall) const;

		void UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion = {}) const;
		void UpdatePackages(std::span<const std::string> packageNames) const;
		void UpdateAllPackages() const;

		void UninstallPackage(const std::string& packageName) const;
		void UninstallPackages(std::span<const std::string> packageNames) const;
		void UninstallAllPackages() const;

		void SnapshotPackages(const std::filesystem::path& manifestFilePath, bool prettify) const;

		bool HasMissedPackages() const;
		bool HasConflictedPackages() const;
		void InstallMissedPackages() const;
		void UninstallConflictedPackages() const;

		LocalPackageOpt FindLocalPackage(const std::string& packageName) const;
		RemotePackageOpt FindRemotePackage(const std::string& packageName) const;

		std::vector<LocalPackageRef> GetLocalPackages() const;
		std::vector<RemotePackageRef> GetRemotePackages() const;

	private:
		PackageManager& _impl;
	};
}