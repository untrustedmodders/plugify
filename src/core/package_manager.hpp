#pragma once

#include "package_manifest.hpp"
#include "plugify_context.hpp"
#include <plugify/package.hpp>
#include <plugify/package_manager.hpp>
#include <utils/hash.hpp>

namespace plugify {
#if PLUGIFY_DOWNLOADER
	class IHTTPDownloader;
	class PluginReferenceDescriptor;
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

		void InstallPackage(std::string_view packageName, std::optional<plg::version> requiredVersion) override;
		void InstallPackages(std::span<const std::string> packageNames) override;
		void InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) override;
		void InstallAllPackages(const std::string& manifestUrl, bool reinstall) override;

		void UpdatePackage(std::string_view packageName, std::optional<plg::version> requiredVersion) override;
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

		LocalPackagePtr FindLocalPackage(std::string_view packageName) const override;
		RemotePackagePtr FindRemotePackage(std::string_view packageName) const override;

		std::vector<LocalPackagePtr> GetLocalPackages() const override;
		std::vector<RemotePackagePtr> GetRemotePackages() const override;

	private:
		void LoadAllPackages();
		void LoadLocalPackages();
#if PLUGIFY_DOWNLOADER
		void LoadRemotePackages();
		void FindDependencies();
		void CheckLanguageModuleDependency(const LocalPackagePtr& pluginPackage, const std::string& lang);
		void CheckPluginDependency(const LocalPackagePtr& package, const PluginReferenceDescriptor& dependency);

		template<typename F>
		void Request(F&& action, std::string_view function);

		bool UpdatePackage(const LocalPackagePtr& package, std::optional<plg::version> requiredVersion = {});
		bool InstallPackage(const RemotePackagePtr& package, std::optional<plg::version> requiredVersion = {});
		bool UninstallPackage(const LocalPackagePtr& package, bool remove = true);
		bool DownloadPackage(const PackagePtr& package, const PackageVersion& version) const;
		static std::string ExtractPackage(std::span<const uint8_t> packageData, const fs::path& extractPath, std::string_view descriptorExt);
		static bool IsPackageLegit(std::string_view checksum, std::span<const uint8_t> packageData);
#endif // PLUGIFY_DOWNLOADER

	private:
#if PLUGIFY_DOWNLOADER
		std::unique_ptr<IHTTPDownloader> _httpDownloader;
#endif // PLUGIFY_DOWNLOADER
		std::unordered_map<std::string, LocalPackagePtr, string_hash, std::equal_to<>> _localPackages;
		std::unordered_map<std::string, RemotePackagePtr, string_hash, std::equal_to<>> _remotePackages;
		std::unordered_map<std::string, std::pair<RemotePackagePtr, std::optional<plg::version>>> _missedPackages;
		std::vector<LocalPackagePtr> _conflictedPackages;
		bool _inited{ false };
	};
}
