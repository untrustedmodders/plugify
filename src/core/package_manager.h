#pragma once

#include "wizard_context.h"
#include <wizard/package_manager.h>
#include <wizard/package.h>

namespace wizard {
	class HTTPDownloader;
	class PackageManager : public IPackageManager, public WizardContext {
	public:
		explicit PackageManager(std::weak_ptr<IWizard> wizard);
		~PackageManager();

	public:
		/** IPackageManager interface */
		bool Initialize();
		void Terminate();

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

		void SnapshotPackages(const fs::path& manifestFilePath, bool prettify);

		bool HasMissedPackages() { return !_missedPackages.empty(); }
		bool HasConflictedPackages() { return !_conflictedPackages.empty(); }
		void InstallMissedPackages();
		void UninstallConflictedPackages();

		LocalPackageOpt FindLocalPackage(const std::string& packageName);
		RemotePackageOpt FindRemotePackage(const std::string& packageName);

		std::vector<LocalPackageRef> GetLocalPackages();
		std::vector<RemotePackageRef> GetRemotePackages();

	public:
		static bool IsSupportsPlatform(std::span<const std::string> supportedPlatforms) {
			return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), WIZARD_PLATFORM) != supportedPlatforms.end();
		}

	private:
		void LoadLocalPackages();
		void LoadRemotePackages();
		void FindDependencies();

		void Request(const std::function<void()>& action, std::string_view function);

		bool UpdatePackage(const LocalPackage& package, std::optional<int32_t> requiredVersion = {});
		bool InstallPackage(const RemotePackage& package, std::optional<int32_t> requiredVersion = {});
		bool UninstallPackage(const LocalPackage& package, bool remove = true);

		[[nodiscard]] bool DownloadPackage(const Package& package, const PackageVersion& version) const;
		static std::string ExtractPackage(std::span<const uint8_t> packageData, const fs::path& extractPath, std::string_view descriptorExt);

		/**
		 * Retrieves the verified packages list from the central authority.
		 *
		 * The Wizard auto-downloading feature does not allow automatically installing
		 * all packages for various (notably security) reasons; packages that are candidate to
		 * auto-downloading are rather listed on a GitHub repository
		 * (https://raw.githubusercontent.com/untrustedpackageders/verified_packages/verified-packages.json),
		 * which this method gets via a HTTP call to load into local state.
		 *
		 * If list fetching fails, local packages list will be initialized as empty, thus
		 * preventing any package from being auto-downloaded.
		 */
		//void FetchPackagesListFromAPI();

		/**
		 * Checks whether a package is verified.
		 *
		 * A package is deemed verified/authorized through a manual validation process that is
		 * described here: https://github.com/untrustedpackageders/verified_packages/README.md;
		 */
		//bool IsPackageAuthorized(const std::string& packageName, int32_t packageVersion) const;

		/**
		 * Tells if a package archive has not been corrupted. (Use after IsPackageAuthorized)
		 *
		 * The package validation procedure includes computing the SHA256 hash of the final
		 * archive, which is stored in the verified packages list. This hash is used by this
		 * very method to ensure the archive downloaded from the Internet is the exact
		 * same that has been manually verified.
		 */
		//bool IsPackageLegit(const std::string& packageName, int32_t packageVersion, std::span<const uint8_t> packageData) const;

	private:
		std::unique_ptr<HTTPDownloader> _httpDownloader;
		std::vector<LocalPackage> _localPackages;
		std::vector<RemotePackage> _remotePackages;
		//VerifiedPackageMap _packages;
		using Dependency = std::pair<RemotePackageRef, std::optional<int32_t>>;
		std::unordered_map<std::string, Dependency> _missedPackages;
		std::vector<LocalPackageRef> _conflictedPackages;
	};
}