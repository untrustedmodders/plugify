#pragma once

#include "package_downloader.h"
#include "wizard_context.h"
#include <wizard/package_manager.h>
#include <wizard/package.h>

namespace wizard {
	class PackageManager : public IPackageManager, public WizardContext {
	public:
		explicit PackageManager(std::weak_ptr<IWizard> wizard);
		~PackageManager();

		/** IPackageManager interface */
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

		static bool IsSupportsPlatform(std::span<const std::string> supportedPlatforms) {
			return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), WIZARD_PLATFORM) != supportedPlatforms.end();
		}

	private:
		void LoadLocalPackages();
		void LoadRemotePackages();
		void ResolveDependencies();

		bool UpdatePackage(const LocalPackage& package, std::optional<int32_t> requiredVersion = {});
		bool InstallPackage(const RemotePackage& package, std::optional<int32_t> requiredVersion = {});
		bool UninstallPackage(const LocalPackage& package);

		void Request(const std::function<void()>& action);

	private:
		PackageDownloader _downloader;
		std::unordered_map<std::string, LocalPackage> _localPackages;
		std::unordered_map<std::string, RemotePackage> _remotePackages;
	};
}