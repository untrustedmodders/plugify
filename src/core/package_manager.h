#pragma once

#include "wizard_context.h"
#include "package.h"
#include "package_downloader.h"
#include <wizard/package_manager.h>

namespace wizard {
	using LocalPackageRef = std::optional<std::reference_wrapper<LocalPackage>>;
	using RemotePackageRef = std::optional<std::reference_wrapper<RemotePackage>>;

	class PackageManager : public IPackageManager, public WizardContext {
	public:
		explicit PackageManager(std::weak_ptr<IWizard> wizard);
		~PackageManager();

		void LoadLocalPackages();
		void LoadRemotePackages();

		void InstallPackage(const std::string& packageName);
		void InstallPackages(std::span<const std::string> packageNames);
		void InstallAllPackages(const fs::path& manifestFilePath, bool reinstall);

		void UpdatePackage(const std::string& packageName);
		void UpdatePackages(std::span<const std::string> packageNames);
		void UpdateAllPackages();

		void UninstallPackage(const std::string& packageName);
		void UninstallPackages(std::span<const std::string> packageNames);
		void UninstallAllPackages();

		void SnapshotPackages(const fs::path& manifestFilePath, bool prettify);

	private:
		LocalPackageRef FindLocalPackage(const std::string& packageName);
		RemotePackageRef FindRemotePackage(const std::string& packageName);

		void UpdatePackage(const LocalPackage& package);
		void InstallPackage(const RemotePackage& package);
		void UninstallPackage(const LocalPackage& package);

		void DoPackage(const std::function<void()>& body);

	private:
		//PackageDownloader _downloader;
		std::unordered_map<std::string, LocalPackage> _localPackages;
		std::unordered_map<std::string, RemotePackage> _remotePackages;
	};
}