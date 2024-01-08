#pragma once

#include "package_downloader.h"
#include "wizard_context.h"
#include <wizard/package_manager.h>
#include <wizard/package.h>

namespace wizard {
	using LocalPackageRef = std::optional<std::reference_wrapper<const LocalPackage>>;
	using RemotePackageRef = std::optional<std::reference_wrapper<const RemotePackage>>;

	class PackageManager : public IPackageManager, public WizardContext {
	public:
		explicit PackageManager(std::weak_ptr<IWizard> wizard);
		~PackageManager();

		/** IPackageManager interface */
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
		// Move to IPackageManager
		LocalPackageRef FindLocalPackage(const std::string& packageName) const;
		RemotePackageRef FindRemotePackage(const std::string& packageName) const;

		bool UpdatePackage(const LocalPackage& package);
		bool InstallPackage(const RemotePackage& package);
		bool UninstallPackage(const LocalPackage& package);

		void Request(const std::function<void()>& action);

	private:
		PackageDownloader _downloader;
		std::unordered_map<std::string, LocalPackage> _localPackages;
		std::unordered_map<std::string, RemotePackage> _remotePackages;
	};
}