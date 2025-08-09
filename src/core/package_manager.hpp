#pragma once

#include <plugify/package_manager.hpp>
#include <utils/hash.hpp>

namespace plugify {
	/**
	 * @brief Concrete implementation of the package manager
	 *
	 * This class orchestrates all package management operations by coordinating
	 * between repositories, local package providers, and resolvers.
	 */
	class PackageManager final : public IPackageManager, public PlugifyContext {
	public:
		explicit PackageManager(std::weak_ptr<IPlugify> plugify);
		~PackageManager() override;

		// IPackageManager implementation
		bool Initialize() override;
		void Terminate() override;
		bool IsInitialized() const override;
		bool Reload() override;

		void AddRepository(std::unique_ptr<IPackageRepository> repository) override;
		void RemoveRepository(std::string_view identifier) override;
		std::vector<std::string> ListRepositories() const override;

		Result<std::vector<LocalPackage>> ListLocalPackages() const override;
		Result<std::vector<RemotePackage>> ListRemotePackages() const override;
		Result<std::vector<PackageInfo>> SearchPackages(std::string_view pattern) const override;
		Result<PackageInfo> GetPackageInfo(std::string_view packageName) const override;

		Result<InstallResult> InstallPackage(std::string_view packageName,
											 std::optional<plg::version> version = std::nullopt) override;
		Result<void> RemovePackage(std::string_view packageName) override;
		Result<InstallResult> UpdatePackage(std::string_view packageName) override;
		Result<std::vector<std::string>> UpdateAll() override;

		Result<void> RefreshRepositories() override;
		Result<std::vector<ConflictInfo>> VerifySystem() override;
		Result<void> ResolveConflicts() override;

		void SetLocalPackagePaths(std::vector<std::filesystem::path> paths) override;
		std::span<const std::filesystem::path> GetLocalPackagePaths() const override;

	private:

		// Dependencies (injected)
		std::unique_ptr<ILocalPackageProvider> _localProvider;
		std::unique_ptr<IDependencyResolver> _dependencyResolver;
		std::unique_ptr<IConflictResolver> _conflictResolver;

		// State
		std::unordered_map<std::string, std::unique_ptr<IPackageRepository>, string_hash, std::equal_to<>> _repositories;
		std::vector<std::filesystem::path> _localPackagePaths;

		// Cache for performance
		mutable std::optional<std::vector<RemotePackage>> _remotePackagesCache;
		mutable std::optional<std::vector<LocalPackage>> _localPackagesCache;

		// Helper methods
		void InvalidateCache();
		Result<std::vector<RemotePackage>> GetAllRemotePackages() const;
		Result<std::vector<LocalPackage>> GetAllLocalPackages() const;
		Result<void> InstallLocalPackage(const std::filesystem::path& packagePath);
		Result<void> DownloadAndInstall(const RemotePackage& package, const plg::version& version);
	};
} // namespace plugify

#if 0
#include "package_manifest.hpp"
#include "plugify_context.hpp"
#include <plugify/package.hpp>
#include <plugify/package_manager.hpp>
#include <utils/hash.hpp>

namespace plugify {
#if PLUGIFY_DOWNLOADER
	class IHTTPDownloader;
	struct PluginReferenceDescriptor;
	struct LanguageModuleInfo;
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
		void CheckLanguageModuleDependency(const LocalPackagePtr& package, const LanguageModuleInfo& languageModule);
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

#endif