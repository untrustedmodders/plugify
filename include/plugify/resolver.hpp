#pragma once

#include <span>
#include <string>
#include <vector>
#include <variant>

#include "package.hpp"
#include "expected.hpp"

namespace plugify {
	template<typename T>
	using Result = plg::expected<T, std::string>;

	/**
	 * @brief Package operation result
	 */
	struct OperationResult {
		bool success;
		std::string message;
		std::vector<std::string> affectedPackages;
		std::optional<std::string> errorCode;
	};

	/**
	 * @brief Dependency resolution result
	 */
	struct DependencyResolutionResult {
		std::vector<Package> requiredPackages;
		std::vector<PackageDependency> missingDependencies;
		std::vector<PackageConflict> conflictingConstraints; // Dependencies with conflicting version constraints
	};

	/**
	 * @brief Interface for dependency resolution
	 */
	class IDependencyResolver {
	public:
		virtual ~IDependencyResolver() = default;

		/**
		 * @brief Resolve dependencies for a package
		 */
		virtual DependencyResolutionResult ResolveDependencies(
			const Package& package,
			std::span<const Package> availablePackages) = 0;

		/**
		 * @brief Build dependency graph
		 */
		//virtual std::unordered_map<PackageId, std::vector<PackageId>> BuildDependencyGraph(
		//	std::span<const Package> packages) = 0;

		/**
		 * @brief Check for circular dependencies
		 */
		//virtual bool HasCircularDependencies(std::span<const Package> packages) = 0;

		/**
		 * @brief Find the best version of a package that satisfies constraints
		 */
		virtual std::optional<Package> FindBestMatch(
			std::string_view packageId,
			const std::vector<VersionConstraint>& constraints,
			std::span<const Package> availablePackages) = 0;

		/**
		 * @brief Check if multiple constraints are compatible
		 */
		virtual bool AreConstraintsCompatible(
			const std::vector<VersionConstraint>& constraints1,
			const std::vector<VersionConstraint>& constraints2) = 0;
	};

	/**
	 * @brief Detected conflict information
	 */
	struct ConflictInfo {
		Package package1;
		Package package2;
		std::string reason;
        std::vector<std::string> suggestions;
	};

	/**
	 * @brief Interface for conflict resolution
	 */
	class IConflictResolver {
	public:
		virtual ~IConflictResolver() = default;

		/**
		 * @brief Detect conflicts between packages
		 */
		virtual std::vector<ConflictInfo> DetectConflicts(
			std::span<const Package> packages) const = 0;

		/**
		 * @brief Resolve conflicts automatically if possible
		 */
		virtual Result<std::vector<Package>> ResolveConflicts(
			std::span<const ConflictInfo> conflicts) = 0;

		/**
		 * @brief Check if package would conflict with installed packages
		 */
		virtual bool WouldConflict(
			const Package& package,
			std::span<const Package> installedPackages) const = 0;
	};
}