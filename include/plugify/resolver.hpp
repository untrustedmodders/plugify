#pragma once

#include <span>
#include <string>
#include <vector>
#include <variant>

#include "package.hpp"
#include "expected.hpp"

namespace plugify {
	enum class ErrorCode {
		Success,
		PackageNotFound,
		DependencyMissing,
		ConflictDetected,
		AlreadyInstalled,
		NotInstalled,
		DownloadFailed,
		VerificationFailed,
		FileSystemError,
		NetworkError,
		ParseError,
		Unknown
	};

	struct Error {
		ErrorCode code;
		std::string message;

		static Error Success() { return {ErrorCode::Success, ""}; }

		static Error PackageNotFound(std::string_view pkg) {
			return {ErrorCode::PackageNotFound, std::format("Package '{}' not found", pkg)};
		}
		static Error AlreadyInstalled(std::string_view pkg) {
			return {ErrorCode::AlreadyInstalled, std::format("Package '{}' is already installed", pkg)};
		}
		static Error NotInstalled(std::string_view pkg) {
			return {ErrorCode::NotInstalled, std::format("Package '{}' is not installed", pkg)};
		}
		static Error DependencyMissing(std::string_view dep) {
			return {ErrorCode::DependencyMissing, std::format("Missing dependency: {}", dep)};
		}
		static Error ConflictDetected(std::string_view msg) {
			return {ErrorCode::ConflictDetected, std::format("Conflict detected: {}", msg)};
		}
		static Error FileSystem(std::string_view op, const std::filesystem::filesystem_error& er) {
			return {ErrorCode::FileSystemError, std::format("File system error during {}: {}", op, er.what())};
		}
		static Error Download(std::string_view pkg) {
			return {ErrorCode::DownloadFailed, std::format("Failed to download package '{}'", pkg)};
		}
		static Error Verification(std::string_view pkg) {
			return {ErrorCode::VerificationFailed, std::format("Package '{}' verification failed", pkg)};
		}
		static Error Network(std::string_view msg) {
			return {ErrorCode::NetworkError, std::format("Network error: {}", msg)};
		}
		static Error Parse(std::string_view file) {
			return {ErrorCode::ParseError, std::format("Failed to parse: {}", file)};
		}
		static Error Unknown(std::string_view msg = "Unknown error") {
			return {ErrorCode::Unknown, std::string(msg)};
		}
	};

	template<typename T>
	using Result = plg::expected<T, Error>;

	/**
	 * @brief Conflict resolution strategy
	 */
	enum class ConflictResolutionStrategy {
		KeepExisting,
		UpgradeToLatest,
		Interactive,
		Fail
	};

	/**
	 * @brief Detected conflict information
	 */
	struct ConflictInfo {
		Package existing;
		Package conflicting;
		std::string reason;
		std::vector<ConflictResolutionStrategy> availableStrategies;
	};

	/**
	 * @brief Dependency resolution result
	 */
	struct DependencyResolutionResult {
		std::vector<Package> requiredPackages;
		std::vector<Dependency> missingDependencies;
		std::vector<Dependency> conflictingConstraints; // Dependencies with conflicting version constraints
		bool success;
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
		virtual std::unordered_map<PackageId, std::vector<PackageId>> BuildDependencyGraph(
			std::span<const Package> packages) = 0;

		/**
		 * @brief Check for circular dependencies
		 */
		virtual bool HasCircularDependencies(std::span<const Package> packages) = 0;

		/**
		 * @brief Find the best version of a package that satisfies constraints
		 */
		virtual std::optional<Package> FindBestMatch(
			const PackageId& id,
			const std::vector<Constraint>& constraints,
			std::span<const Package> availablePackages) = 0;

		/**
		 * @brief Check if multiple constraints are compatible
		 */
		virtual bool AreConstraintsCompatible(
			const std::vector<Constraint>& constraints1,
			const std::vector<Constraint>& constraints2) = 0;
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
		virtual std::vector<ConflictInfo> DetectConflicts(std::span<const Package> packages) = 0;

		/**
		 * @brief Resolve detected conflicts
		 */
		virtual Result<std::vector<Package>> ResolveConflicts(
			std::span<const ConflictInfo> conflicts,
			ConflictResolutionStrategy strategy) = 0;
	};
}