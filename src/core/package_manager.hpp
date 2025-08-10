#pragma once

#include <plugify/package_manager.hpp>
#include <utils/hash.hpp>

namespace plugify {
	/**
	 * @brief Package manager configuration
	 */
	struct PackageManagerConfig {
		std::filesystem::path installDirectory;
		std::filesystem::path cacheDirectory;
		ConflictResolutionStrategy defaultConflictStrategy{ConflictResolutionStrategy::Fail};
		bool autoResolveDependencies{true};
		bool verifyChecksums{true};
		//size_t maxConcurrentDownloads{3};
	};

	class IPackageScanner;
	class IHTTPDownloader;
	class PackageManager : public IPackageManager {
	public:
	    /**
	     * @brief Construct package manager with configuration
	     */
	    explicit PackageManager(
	        PackageManagerConfig config,
	        std::shared_ptr<IHTTPDownloader> downloader = nullptr,
	        std::shared_ptr<IPackageScanner> scanner = nullptr);
	    
	    // Repository Management
	    Result<void> AddRepository(std::shared_ptr<IPackageRepository> repository) override;
	    Result<void> RemoveRepository(std::string_view name) override;
	    Result<void> UpdateRepositories(ProgressCallback progress) override;
	    
	    // Package Discovery
	    Result<std::vector<Package>> ListAvailable(std::optional<PackageType> type = {}) override;
	    Result<std::vector<Package>> ListInstalled(std::optional<PackageType> type = {}) override;
	    Result<std::vector<Package>> Search(std::string_view query) override;
	    Result<Package> GetPackageInfo(const PackageId& id, const std::optional<plg::version>& version = {}) override;
	    
	    // Package Operations
	    Result<void> Install(const PackageId& id, const std::optional<plg::version>& version, ProgressCallback progress) override;
	    Result<void> Remove(const PackageId& id, bool removeDependents) override;
	    Result<void> Update(const PackageId& id, const std::optional<plg::version>& targetVersion, ProgressCallback progress) override;
	    Result<void> UpdateAll(const std::optional<std::unordered_map<PackageId, plg::version>>& targetVersions, ProgressCallback progress) override;
	    
	    // Dependency & Conflict Management
	    Result<DependencyResolutionResult> CheckDependencies(const PackageId& id) override;
	    Result<std::vector<ConflictInfo>> CheckConflicts() override;
	    Result<void> ResolveConflicts(ConflictResolutionStrategy strategy) override;
	    
	    // System Verification
	    Result<std::vector<std::pair<PackageId, bool>>> VerifyIntegrity() override;
	    Result<void> CleanCache() override;
	    
	    // Additional configuration methods
	    void SetConflictResolver(std::unique_ptr<IConflictResolver> resolver);
	    void SetDependencyResolver(std::unique_ptr<IDependencyResolver> resolver);
	    
	    /**
	     * @brief Trigger a manual scan for installed packages
	     */
		Result<void> RescanInstalledPackages();

	private:
		PackageManagerConfig _config;
		std::vector<std::shared_ptr<IPackageRepository>> _repositories;
		std::unique_ptr<IConflictResolver> _conflictResolver;
		std::unique_ptr<IDependencyResolver> _dependencyResolver;
		std::shared_ptr<IPackageScanner> _packageScanner;
		std::shared_ptr<IHTTPDownloader> _downloader;

		// Cache for installed packages
		mutable std::optional<std::vector<Package>> _installedPackagesCache;
		mutable std::optional<std::vector<Package>> _availablePackagesCache;

		// Private helper methods
		Result<Package> FindPackageInRepositories(const PackageId& id, const std::optional<plg::version>& version);
		Result<void> InstallPackageWithDependencies(const Package& package, ProgressCallback progress);
		Result<void> ValidatePackageInstallation(const Package& package);
		Result<void> ScanAndUpdateInstalledPackages();
		void InvalidateCache();
	};
} // namespace plugify
