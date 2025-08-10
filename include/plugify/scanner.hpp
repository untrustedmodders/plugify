#pragma once

#include <string>
#include <variant>
#include <vector>
#include <span>
#include <filesystem>

#include "package.hpp"
#include "package_manager.hpp"
#include "date_time.hpp"

namespace plugify {
	/**
	 * @brief Package manifest file format
	 */
	enum class ManifestFormat {
		JSON,
		YAML,
		TOML,
		XML
	};

	/**
	 * @brief Information about a scanned package
	 */
	struct ScannedPackage {
		Package package;
		std::filesystem::path manifestPath;
		ManifestFormat format;
		DateTime scanTime;
		std::optional<std::string> error;  // Set if package is corrupted/invalid
	};

	/**
	 * @brief Scanner configuration
	 */
	struct PackageScannerConfig {
		std::vector<std::filesystem::path> searchPaths; // Directories to scan
		std::vector<std::string> manifestFileNames;  // e.g., "package.json", "manifest.yaml"
		ManifestFormat preferredFormat{ManifestFormat::JSON};
		bool scanRecursively{true};
		bool validateOnScan{true};
		size_t maxDepth{3};  // Max recursion depth
	};

	/**
	 * @brief Interface for scanning and discovering local packages
	 */
	class IPackageScanner {
	public:
		virtual ~IPackageScanner() = default;

		/**
		 * @brief Scan for packages in configured directories
		 */
		virtual Result<std::vector<ScannedPackage>> ScanForPackages(
			const PackageScannerConfig& config = {}) = 0;

		/**
		 * @brief Scan a specific directory for packages
		 */
		virtual Result<std::vector<ScannedPackage>> ScanDirectory(
			const std::filesystem::path& directory,
			bool recursive = true) = 0;

		/**
		 * @brief Check if a directory contains a valid package
		 */
		virtual Result<ScannedPackage> ScanPackage(const std::filesystem::path& packagePath) = 0;

		/**
		 * @brief Parse a manifest file
		 */
		virtual Result<PackageInfo> ParseManifest(
			const std::filesystem::path& manifestPath,
			ManifestFormat format = ManifestFormat::JSON) = 0;

		/**
		 * @brief Validate package structure and files
		 */
		virtual Result<bool> ValidatePackageStructure(const std::filesystem::path& packagePath) = 0;

		/**
		 * @brief Get last scan results (cached)
		 */
		virtual std::optional<std::vector<ScannedPackage>> GetLastScanResults() const = 0;

		/**
		 * @brief Watch directories for package changes (async)
		 */
		virtual Result<void> StartWatching(
			std::function<void(const ScannedPackage&, bool added)> callback) = 0;

		/**
		 * @brief Stop watching for changes
		 */
		virtual void StopWatching() = 0;
	};
} // namespace plugify
