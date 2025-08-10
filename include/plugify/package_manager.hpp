#pragma once

#include <optional>
#include <functional>
#include <string_view>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include "resolver.hpp"
#include "repository.hpp"

namespace plugify {
	/**
	 * @brief Operation progress callback
	 */
	using ProgressCallback = std::function<void(std::string_view operation, float progress)>;

	/**
	 * @brief Operation result with detailed information
	 */
	/*struct OperationResult {
		std::string message;
		std::vector<std::string> warnings;
		std::vector<Package> affectedPackages;
	};*/

	/**
	 * @brief Main package manager interface
	 */
	class IPackageManager {
	public:
	    virtual ~IPackageManager() = default;

	    // Repository Management
	    /**
	     * @brief Add a repository source
	     */
	    virtual Result<void> AddRepository(std::unique_ptr<IPackageRepository> repository) = 0;

	    /**
	     * @brief Remove a repository by identifier
	     */
	    virtual Result<void> RemoveRepository(std::string_view repositoryId) = 0;

	    /**
	     * @brief Refresh all repositories
	     */
	    virtual Result<void> RefreshRepositories() = 0;

	    // Package Discovery
	    /**
	     * @brief List all available packages (local + remote)
	     */
	    virtual std::vector<Package> ListAvailablePackages(std::optional<PackageType> type = {}) const = 0;

	    /**
	     * @brief List installed packages
	     */
	    virtual std::vector<Package> ListInstalledPackages(std::optional<PackageType> type = {}) const = 0;

	    /**
	     * @brief Search packages by query
	     */
	    virtual std::vector<Package> SearchPackages(const PackageQuery& query) const = 0;

	    /**
	     * @brief Get detailed package information
	     */
	    virtual std::optional<Package> GetPackageInfo(std::string_view packageId) const = 0;

	    // Package Operations
	    /**
	     * @brief Install a package
	     */
	    virtual Result<OperationResult> InstallPackage(
	        std::string_view packageId,
	        const std::optional<plg::version>& version = {},
	        ProgressCallback progress = nullptr) = 0;

	    /**
	     * @brief Remove a package
	     */
	    virtual Result<OperationResult> RemovePackage(
	        std::string_view packageId,
	        bool removeUnusedDependencies = false,
	        bool removeUserData = false) = 0;

	    /**
	     * @brief Update a package
	     */
	    virtual Result<OperationResult> UpdatePackage(
	        std::string_view packageId,
	        const std::optional<plg::version>& version = {},
	        ProgressCallback progress = nullptr) = 0;

	    /**
	     * @brief Update all packages
	     */
	    virtual Result<OperationResult> UpdateAllPackages(
	        ProgressCallback progress = nullptr) = 0;

	    // Dependency and Conflict Management
	    /**
	     * @brief Check package dependencies
	     */
	    virtual Result<std::vector<DependencyResolutionResult>> CheckDependencies(
	        std::string_view packageId) const = 0;

		/**
		 * @brief Check for conflicts
		 */
		virtual Result<std::vector<ConflictInfo>> CheckConflicts() = 0;

		/**
		 * @brief Resolve conflicts
		 */
		virtual Result<OperationResult> ResolveConflicts() = 0;

	    /**
	     * @brief Verify system integrity
	     */
	    virtual Result<std::vector<std::pair<std::string_view, bool>>> VerifySystemIntegrity() const = 0;

		/**
		 * @brief Clean cache and temporary files
		 */
		virtual Result<void> CleanCache() = 0;
	};

} // namespace plugify
