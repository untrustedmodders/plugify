#pragma once

#include <string>
#include <variant>
#include <vector>
#include <span>
#include <filesystem>

#include "package.hpp"
#include "package_manager.hpp"

namespace plugify {
	// Repository interface for dependency inversion
	class IPackageRepository {
	public:
		virtual ~IPackageRepository() = default;

		/**
		 * Fetch available packages from the repository
		 */
		virtual Result<std::vector<RemotePackage>> FetchPackages() = 0;

		/**
		 * Search packages by query
		 */
		virtual Result<std::vector<RemotePackage>> SearchPackages(std::string_view query) = 0;

		/**
		 * Download a specific package version
		 */
		virtual Result<fs::path> DownloadPackage(
			const RemotePackage& package,
			const PackageVersion& version,
			std::function<bool(uint32_t, uint32_t)> progressCallback = nullptr
		) = 0;

		/**
		 * Get repository identifier
		 */
		virtual std::string_view GetIdentifier() const = 0;

		/**
		 * Check if repository is available
		 */
		virtual bool IsAvailable() = 0;
	};

	// Local package scanner interface
	class IPackageScanner {
	public:
		virtual ~IPackageScanner() = default;

		/**
		 * Scan directory for packages
		 */
		virtual Result<std::vector<LocalPackage>> ScanDirectory(const fs::path& path) = 0;

		/**
		 * Verify package integrity
		 */
		virtual Result<bool> VerifyPackage(const LocalPackage& package) = 0;

		/**
		 * Load package descriptor from manifest
		 */
		virtual Result<std::shared_ptr<Descriptor>> LoadDescriptor(const fs::path& manifestPath) = 0;
	};
} // namespace plugify
