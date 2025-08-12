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
		static Error FileSystem(std::string_view op, std::string_view msg) {
			return {ErrorCode::FileSystemError, std::format("File system error during {}: {}", op, msg)};
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
	class IResolver {
	public:
		virtual ~IResolver() = default;

		/**
		 * @brief Resolve dependencies for a package
		 */
		virtual DependencyResolutionResult ResolveDependencies(
			const Package& package,
			std::span<const Package> availablePackages) = 0;
	};
}