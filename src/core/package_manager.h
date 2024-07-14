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
		~PackageManager() override;

	public:
		/** IPackageManager interface */
		bool Initialize() override;
		void Terminate() override;
		bool IsInitialized() const override;
		bool Reload() override;

		void InstallPackage(std::string_view packageName, std::optional<int32_t> requiredVersion) override;
		void InstallPackages(std::span<const std::string> packageNames) override;
		void InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) override;
		void InstallAllPackages(const std::string& manifestUrl, bool reinstall) override;

		void UpdatePackage(std::string_view packageName, std::optional<int32_t> requiredVersion) override;
		void UpdatePackages(std::span<const std::string> packageNames) override;
		void UpdateAllPackages() override;

		void UninstallPackage(std::string_view packageName) override;
		void UninstallPackages(std::span<const std::string> packageNames) override;
		void UninstallAllPackages() override;

		void SnapshotPackages(const fs::path& manifestFilePath, bool prettify) override;

		bool HasMissedPackages() const override { return !_missedPackages.empty(); }
		bool HasConflictedPackages() const override { return !_conflictedPackages.empty(); }
		void InstallMissedPackages() override;
		void UninstallConflictedPackages() override;

		LocalPackageOpt FindLocalPackage(std::string_view packageName) const override;
		RemotePackageOpt FindRemotePackage(std::string_view packageName) const override;

		std::vector<LocalPackageRef> GetLocalPackages() const override;
		std::vector<RemotePackageRef> GetRemotePackages() const override;

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
		static bool IsPackageLegit(std::string_view checksum, std::span<const uint8_t> packageData);

		using Dependency = std::pair<RemotePackageRef, std::optional<int32_t>>;
		
	private:
		std::unique_ptr<HTTPDownloader> _httpDownloader;
		std::vector<LocalPackage> _localPackages;
		std::vector<RemotePackage> _remotePackages;
		std::unordered_map<std::string, Dependency> _missedPackages;
		std::vector<LocalPackageRef> _conflictedPackages;
	};
}