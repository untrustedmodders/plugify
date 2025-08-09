#pragma once

#include <optional>
#include <string_view>

#include "constrant.hpp"
#include "resolver.hpp"
#include "repository.hpp"

namespace plugify {
	// Package operation results
	struct InstallResult {
		bool success;
		std::vector<std::string> installedPackages;
		std::vector<Error> errors;
	};

	struct RemoveResult {
		bool success;
		std::vector<std::string> removedPackages;
		std::vector<Error> errors;
	};

	struct UpdateResult {
		bool success;
		std::vector<std::pair<std::string, plg::version>> updatedPackages;
		std::vector<Error> errors;
	};

	/**
	 * @brief Main package manager interface
	 *
	 * @details Pure interface for testability and flexibility.
	 *          Defines the public API for package management operations.
	 *
	 * @note Follows Interface Segregation Principle
	 */
	class IPackageManager {
	public:
	    virtual ~IPackageManager() = default;

	    // Repository management
	    /**
	     * Add a repository source
	     */
	    virtual void AddRepository(std::unique_ptr<IPackageRepository> repository) = 0;

	    /**
	     * Remove a repository by identifier
	     */
	    virtual bool RemoveRepository(std::string_view identifier) = 0;

	    /**
	     * Refresh all repository data
	     */
	    virtual Result<void> RefreshRepositories() = 0;

	    // Package enumeration
	    /**
	     * Get all locally installed packages
	     */
	    virtual std::vector<LocalPackage> GetInstalledPackages() const = 0;

	    /**
	     * Get all available remote packages
	     */
	    virtual std::vector<RemotePackage> GetAvailablePackages() const = 0;

	    /**
	     * Get packages by type filter
	     */
	    virtual std::vector<LocalPackage> GetInstalledPackagesByType(std::string_view type) const = 0;

	    // Package operations
	    /**
	     * Install package(s) by name
	     */
	    virtual InstallResult Install(std::span<const std::string> packageNames) = 0;

	    /**
	     * Remove package(s) by name
	     */
	    virtual RemoveResult Remove(std::span<const std::string> packageNames) = 0;

	    /**
	     * Update package(s)
	     */
	    virtual UpdateResult Update(std::span<const std::string> packageNames = {}) = 0;

	    /**
	     * Search for packages
	     */
	    virtual std::vector<std::variant<LocalPackage, RemotePackage>> Search(std::string_view query) = 0;

	    /**
	     * Get detailed package information
	     */
	    virtual std::optional<std::variant<LocalPackage, RemotePackage>> GetPackageInfo(std::string_view name) = 0;

	    // Dependency and conflict management
	    /**
	     * Check dependencies for a package
	     */
	    virtual Result<std::vector<PackageConstraint>> CheckDependencies(std::string_view packageName) = 0;

	    /**
	     * Verify system integrity (check all conflicts and dependencies)
	     */
	    virtual Result<void> VerifySystemIntegrity() = 0;

	    /**
	     * Set conflict resolution strategy
	     */
	    virtual void SetConflictStrategy(ConflictResolutionStrategy strategy) = 0;

	    // Configuration
	    /**
	     * Set the root directory for local packages
	     */
	    virtual void SetPackageRoot(const std::filesystem::path& path) = 0;

	    /**
	     * Get current package root directory
	     */
	    virtual std::filesystem::path GetPackageRoot() const = 0;
	};
} // namespace plugify
