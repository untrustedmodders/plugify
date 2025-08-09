#pragma once

#include <span>
#include <string>
#include <vector>
#include <variant>

#include "package.hpp"
#include "expected.hpp"

namespace plugify {
	enum class ErrorCode {
		None,
		PackageNotFound,
		VersionConflict,
		DependencyMissing,
		NetworkError,
		FileSystemError,
		InvalidManifest,
		ChecksumMismatch,
		PlatformIncompatible
	};

	struct Error {
		ErrorCode code;
		std::string message;
	};

	template<typename T>
	using Result = plg::expected<T, Error>;

	struct ConflictInfo {
		LocalPackage existing;
		std::variant<LocalPackage, PackageVersion> conflicting;
		std::string reason;
	};

	// Conflict resolution strategies
	enum class ConflictResolutionStrategy {
		KeepExisting,
		ReplaceWithNewer,
		Interactive,
		Fail
	};

	// Dependency resolver interface
	class IDependencyResolver {
	public:
		virtual ~IDependencyResolver() = default;

		/**
		 * Resolve dependencies for a package
		 */
		virtual Result<std::vector<PackageConstraint>> ResolveDependencies(
			const Package& package,
			std::span<const LocalPackage> installed,
			std::span<const RemotePackage> available
		) = 0;

		/**
		 * Check if all dependencies are satisfied
		 */
		virtual bool AreDependenciesSatisfied(
			const std::vector<PackageConstraint>& dependencies,
			std::span<const LocalPackage> installed
		) = 0;

		/**
		 * Calculate installation order based on dependencies
		 */
		virtual Result<std::vector<std::string>> CalculateInstallOrder(
			std::span<const std::string> packages,
			std::span<const LocalPackage> installed,
			std::span<const RemotePackage> available
		) = 0;
	};

	// Conflict resolver interface
	class IConflictResolver {
	public:
		virtual ~IConflictResolver() = default;

		/**
		 * Detect conflicts between packages
		 */
		virtual std::vector<ConflictInfo> DetectConflicts(
			std::span<const LocalPackage> packages
		) = 0;

		/**
		 * Resolve conflicts based on strategy
		 */
		virtual Result<std::vector<LocalPackage>> ResolveConflicts(
			std::span<const ConflictInfo> conflicts,
			ConflictResolutionStrategy strategy
		) = 0;

		/**
		 * Check if a package would conflict with installed packages
		 */
		virtual bool WouldConflict(
			const Package& package,
			const PackageVersion& version,
			std::span<const LocalPackage> installed
		) = 0;
	};
}