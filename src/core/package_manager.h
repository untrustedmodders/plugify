#pragma once

#include "plugify_context.h"
#include "package_manifest.h"
#include <plugify/package_manager.h>
#include <plugify/package.h>

namespace plugify {
	class HTTPDownloader;
	class PackageManager final : public IPackageManager, public PlugifyContext {
	public:
		explicit PackageManager(std::weak_ptr<IPlugify> plugify);
		~PackageManager();

	public:
		/** IPackageManager interface */
		bool Initialize();
		void Terminate();
		bool IsInitialized();
		bool Reload();

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
			return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), PLUGIFY_PLATFORM) != supportedPlatforms.end();
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
		static bool IsPackageLegit(const std::string& checksum, std::span<const uint8_t> packageData);

	private:
		std::unique_ptr<HTTPDownloader> _httpDownloader;
		std::vector<LocalPackage> _localPackages;
		std::vector<RemotePackage> _remotePackages;
		using Dependency = std::pair<RemotePackageRef, std::optional<int32_t>>;
		std::unordered_map<std::string, Dependency> _missedPackages;
		std::vector<LocalPackageRef> _conflictedPackages;
	};
}