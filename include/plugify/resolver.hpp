#pragma once

#include <string_view>
#include <vector>
#include <span>

namespace plugify {
	// Dependency resolution interface
	class IDependencyResolver {
	public:
		virtual ~IDependencyResolver() = default;

		/**
		 * @brief Resolve dependencies for a package installation
		 * @param packageName Target package name
		 * @param version Target package version
		 * @param availableLocal Currently available local packages
		 * @param availableRemote Available remote packages
		 * @return Result containing ordered installation list or error
		 */
		virtual Result<std::vector<PackageInfo>> ResolveDependencies(
			std::string_view packageName,
			plg::version version,
			std::span<const LocalPackage> availableLocal,
			std::span<const RemotePackage> availableRemote) const = 0;

		/**
		 * @brief Check for conflicts in current package set
		 * @param packages Packages to check for conflicts
		 * @return Vector of detected conflicts
		 */
		virtual std::vector<ConflictInfo> DetectConflicts(
			std::span<const PackageInfo> packages) const = 0;
	};

	// Conflict resolution interface
	class IConflictResolver {
	public:
		virtual ~IConflictResolver() = default;

		/**
		 * @brief Attempt to resolve detected conflicts automatically
		 * @param conflicts List of conflicts to resolve
		 * @param availablePackages Available packages for resolution
		 * @return Result containing resolution plan or error
		 */
		virtual Result<std::vector<std::string>> ResolveConflicts(
			std::span<const ConflictInfo> conflicts,
			std::span<const PackageInfo> availablePackages) const = 0;
	};
}