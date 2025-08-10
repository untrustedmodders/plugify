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
	 * @brief Main package manager interface
	 */
	class IPackageManager {
	public:
	    virtual ~IPackageManager() = default;

	    // Repository Management
	    /**
	     * @brief Add a repository to the package manager
	     */
	    virtual Result<void> AddRepository(std::shared_ptr<IPackageRepository> repository) = 0;

	    /**
	     * @brief Remove a repository
	     */
	    virtual Result<void> RemoveRepository(std::string_view name) = 0;

	    /**
	     * @brief Update repository metadata
	     */
	    virtual Result<void> UpdateRepositories(ProgressCallback progress = {}) = 0;

	    // Package Discovery
	    /**
	     * @brief List all available packages
	     */
	    virtual Result<std::vector<Package>> ListAvailable(std::optional<PackageType> type = {}) = 0;

	    /**
	     * @brief List installed packages
	     */
	    virtual Result<std::vector<Package>> ListInstalled(std::optional<PackageType> type = {}) = 0;

	    /**
	     * @brief Search for packages
	     */
	    virtual Result<std::vector<Package>> Search(std::string_view query) = 0;

	    /**
	     * @brief Get detailed package information
	     */
	    virtual Result<Package> GetPackageInfo(const PackageId& id, const std::optional<plg::version>& version = {}) = 0;

	    // Package Operations
	    /**
	     * @brief Install a package
	     */
	    virtual Result<void> Install(const PackageId& id, const std::optional<plg::version>& version = {}, ProgressCallback progress = {}) = 0;

	    /**
	     * @brief Remove an installed package
	     */
	    virtual Result<void> Remove(const PackageId& id, bool removeDependents = false) = 0;

	    /**
	     * @brief Update a package to specified or latest version
	     */
	    virtual Result<void> Update(const PackageId& id, const std::optional<plg::version>& targetVersion = {}, ProgressCallback progress = {}) = 0;

	    /**
	     * @brief Update all installed packages to specified or latest versions
	     */
	    virtual Result<void> UpdateAll(const std::optional<std::unordered_map<PackageId, plg::version>>& targetVersions = {}, ProgressCallback progress = {}) = 0;

	    // Dependency & Conflict Management
	    /**
	     * @brief Check dependencies for a package
	     */
	    virtual Result<DependencyResolutionResult> CheckDependencies(const PackageId& id) = 0;

	    /**
	     * @brief Check for conflicts
	     */
	    virtual Result<std::vector<ConflictInfo>> CheckConflicts() = 0;

	    /**
	     * @brief Resolve conflicts with specified strategy
	     */
	    virtual Result<void> ResolveConflicts(ConflictResolutionStrategy strategy) = 0;

	    // System Verification
	    /**
	     * @brief Verify integrity of all installed packages
	     */
	    virtual Result<std::vector<std::pair<PackageId, bool>>> VerifyIntegrity() = 0;

	    /**
	     * @brief Clean cache and temporary files
	     */
	    virtual Result<void> CleanCache() = 0;
	};
} // namespace plugify
