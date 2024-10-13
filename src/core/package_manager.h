#pragma once

#include "plugify_context.h"
#include "package_manifest.h"
#include <plugify/package_manager.h>
#include <plugify/package.h>
#include <utils/hash.h>

namespace plugify {
#if PLUGIFY_DOWNLOADER
	class HTTPDownloader;
#endif // PLUGIFY_DOWNLOADER
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

		bool HasMissedPackages() const override;
		bool HasConflictedPackages() const override;
		void InstallMissedPackages() override;
		void UninstallConflictedPackages() override;

		LocalPackageOpt FindLocalPackage(std::string_view packageName) const override;
		RemotePackageOpt FindRemotePackage(std::string_view packageName) const override;

		std::vector<LocalPackage> GetLocalPackages() const override;
		std::vector<RemotePackage> GetRemotePackages() const override;

	public:
		static bool IsSupportsPlatform(std::span<const std::string> supportedPlatforms) {
			return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), PLUGIFY_PLATFORM) != supportedPlatforms.end();
		}

	private:
		void LoadAllPackages();
		void LoadLocalPackages();
#if PLUGIFY_DOWNLOADER
		void LoadRemotePackages();
		void FindDependencies();

		void Request(const std::function<void()>& action, std::string_view function);

		bool UpdatePackage(const LocalPackage& package, std::optional<int32_t> requiredVersion = {});
		bool InstallPackage(const RemotePackage& package, std::optional<int32_t> requiredVersion = {});
		bool UninstallPackage(const LocalPackage& package, bool remove = true);

		[[nodiscard]] bool DownloadPackage(const Package& package, const PackageVersion& version) const;
		static std::string ExtractPackage(std::span<const uint8_t> packageData, const fs::path& extractPath, std::string_view descriptorExt);
		static bool IsPackageLegit(std::string_view checksum, std::span<const uint8_t> packageData);
#endif // PLUGIFY_DOWNLOADER

		using Dependency = std::pair<const RemotePackage*, std::optional<int32_t>>;
		
	private:
#if PLUGIFY_DOWNLOADER
		std::unique_ptr<HTTPDownloader> _httpDownloader;
#endif // PLUGIFY_DOWNLOADER
		std::unordered_map<std::string, LocalPackage, string_hash, std::equal_to<>> _localPackages;
		std::unordered_map<std::string, RemotePackage, string_hash, std::equal_to<>> _remotePackages;
		std::unordered_map<std::string, Dependency> _missedPackages;
		std::vector<const LocalPackage*> _conflictedPackages;
		bool _inited{ false };
	};
}
