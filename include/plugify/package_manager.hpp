#pragma once

#include <optional>
#include <string_view>

#include "constrant.hpp"
#include "repository.hpp"

namespace plugify {
	/**
	 * @brief Result of dependency resolution: planned actions or errors.
	 */
	/*struct DependencyPlan {
		// packages to install (remote package + chosen version)
		std::vector<std::pair<RemotePackage, PackageVersion>> installQueue;
		// packages to remove / upgrade
		std::vector<LocalPackage> removeQueue;
		std::vector<std::string> warnings;
		OperationResult result;
	};*/

	/**
	 * @brief Result of conflict analysis.
	 */
	/*struct ConflictReport {
		bool hasConflicts{false};
		std::vector<std::string> conflictDetails;
		std::optional<std::string> resolutionSuggestion;
	};*/

	struct InstallResult {
		bool success;
		std::vector<std::string> installedPackages;
		std::vector<std::string> errors;
	};

	struct ConflictInfo {
		std::string package1;
		std::string package2;
		std::string reason;
		std::vector<std::string> suggestedResolutions;
	};

	struct PackageInfo {
		std::string name;
		std::string type;
		plg::version version;
		std::optional<std::string> description;
		std::optional<std::string> author;
		bool isLocal;
		std::optional<std::filesystem::path> localPath;
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

		/**
		 * @brief Initialize the package manager.
		 * @return True if initialization is successful, false otherwise.
		 */
		virtual bool Initialize() = 0;

		/**
		 * @brief Terminate the package manager.
		 */
		virtual void Terminate() = 0;

		/**
		 * @brief Check if the package manager is initialized.
		 * @return True if the package manager is initialized, false otherwise.
		 */
		virtual bool IsInitialized() const = 0;

		/**
		 * @brief Reloads the package manager.
		 * @return True if was initialized, false otherwise.
		 */
		virtual bool Reload() = 0;



		// Repository management
		virtual void AddRepository(std::unique_ptr<IPackageRepository> repository) = 0;
		virtual void RemoveRepository(std::string_view identifier) = 0;
		virtual std::vector<std::string> ListRepositories() const = 0;

		// Package discovery and information
		virtual Result<std::vector<LocalPackage>> ListLocalPackages() const = 0;
		virtual Result<std::vector<RemotePackage>> ListRemotePackages() const = 0;
		virtual Result<std::vector<PackageInfo>> SearchPackages(std::string_view pattern) const = 0;
		virtual Result<PackageInfo> GetPackageInfo(std::string_view packageName) const = 0;

		// Package operations
		virtual Result<InstallResult> InstallPackage(std::string_view packageName, std::optional<plg::version> version = std::nullopt) = 0;
		virtual Result<void> RemovePackage(std::string_view packageName) = 0;
		virtual Result<InstallResult> UpdatePackage(std::string_view packageName) = 0;
		virtual Result<std::vector<std::string>> UpdateAll() = 0;

		// System operations
		virtual Result<void> RefreshRepositories() = 0;
		virtual Result<std::vector<ConflictInfo>> VerifySystem() = 0;
		virtual Result<void> ResolveConflicts() = 0;

		// Configuration
		virtual void SetLocalPackagePaths(std::vector<std::filesystem::path> paths) = 0;
		virtual std::span<const std::filesystem::path> GetLocalPackagePaths() const = 0;

		// Basic operations
		//! virtual OperationResult Install(std::string_view packageName, std::optional<VersionConstraint> constraint = std::nullopt) = 0;
		//! virtual OperationResult Remove(std::string_view packageName, bool force = false) = 0;
		//! virtual OperationResult Update(std::optional<std::string> packageName = std::nullopt, std::optional<VersionConstraint> constraint = std::nullopt) = 0; ///< update all or a single package
		//! virtual std::vector<RemotePackage> Search(std::string_view query) const = 0;
		//! virtual std::vector<LocalPackage> ListInstalled() const = 0;
		//! virtual std::optional<RemotePackage> ShowInfo(std::string_view packageName) const = 0;
		//! virtual DependencyPlan CheckDependencies(std::string_view packageName, std::optional<VersionConstraint> constraint = std::nullopt) = 0;
		//! virtual ConflictReport CheckConflicts(bool autoResolve = false) = 0;
	};

} // namespace plugify
