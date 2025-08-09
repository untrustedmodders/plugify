#pragma once

#include <plugify/package_manager.hpp>
#include <utils/hash.hpp>

namespace plugify {
	class PackageManager final : public IPackageManager, public PlugifyContext {
	public:
	    PackageManager(
	        std::unique_ptr<IPackageScanner> scanner,
	        std::unique_ptr<IDependencyResolver> depResolver,
	        std::unique_ptr<IConflictResolver> conflictResolver,
	        std::shared_ptr<IHTTPDownloader> downloader = nullptr
	    );

	    ~PackageManager() override = default;

	    // Repository management
	    void AddRepository(std::unique_ptr<IPackageRepository> repository) override;
	    bool RemoveRepository(std::string_view identifier) override;
	    Result<void> RefreshRepositories() override;

	    // Package enumeration
	    std::vector<LocalPackage> GetInstalledPackages() const override;
	    std::vector<RemotePackage> GetAvailablePackages() const override;
	    std::vector<LocalPackage> GetInstalledPackagesByType(std::string_view type) const override;

	    // Package operations
	    InstallResult Install(std::span<const std::string> packageNames) override;
	    RemoveResult Remove(std::span<const std::string> packageNames) override;
	    UpdateResult Update(std::span<const std::string> packageNames = {}) override;
	    std::vector<std::variant<LocalPackage, RemotePackage>> Search(std::string_view query) override;
	    std::optional<std::variant<LocalPackage, RemotePackage>> GetPackageInfo(std::string_view name) override;

	    // Dependency and conflict management
	    Result<std::vector<PackageConstraint>> CheckDependencies(std::string_view packageName) override;
	    Result<void> VerifySystemIntegrity() override;
	    void SetConflictStrategy(ConflictResolutionStrategy strategy) override;

	    // Configuration
	    void SetPackageRoot(const fs::path& path) override;
	    fs::path GetPackageRoot() const override;

	private:
	    // Helper methods
	    Result<void> ScanLocalPackages();
	    Result<PackageVersion*> FindBestVersion(RemotePackage& package, const std::optional<VersionConstraint>& constraint);
	    Result<void> InstallPackageVersion(const RemotePackage& package, const PackageVersion& version);
	    Result<void> RemoveLocalPackage(const LocalPackage& package);
	    bool IsPackageInstalled(std::string_view name) const;

	    // Member variables
	    fs::path _packageRoot;
	    ConflictResolutionStrategy _conflictStrategy{ConflictResolutionStrategy::Fail};

	    // Injected dependencies
	    std::unique_ptr<IPackageScanner> _scanner;
	    std::unique_ptr<IDependencyResolver> _dependencyResolver;
	    std::unique_ptr<IConflictResolver> _conflictResolver;
	    std::shared_ptr<IHTTPDownloader> _httpDownloader;

	    // Repository management
	    std::vector<std::unique_ptr<IPackageRepository>> _repositories;
	    std::unordered_map<std::string, std::unique_ptr<IPackageRepository>> _repositoryMap;

	    // Package caches
	    mutable std::vector<LocalPackage> _installedPackages;
	    mutable std::vector<RemotePackage> _availablePackages;
	    mutable bool _localCacheValid{false};
	    mutable bool _remoteCacheValid{false};

	    // Thread safety (if needed)
	    mutable std::shared_mutex _cacheMutex;
	    mutable std::shared_mutex _repositoryMutex;
		Result<void> DownloadAndInstall(const RemotePackage& package, const plg::version& version);
	};
} // namespace plugify
