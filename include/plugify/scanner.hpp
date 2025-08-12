#pragma once

#include <vector>

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
	 * @brief Scanner configuration
	 */
	struct PackageScannerConfig {
	    std::vector<fs::path> searchPaths;           // Directories to scan
	    std::vector<std::string> manifestFileNames;  // e.g., "package.json", "manifest.yaml"
	    ManifestFormat preferredFormat{ManifestFormat::JSON};
	    bool scanRecursively{true};
	    bool validateOnScan{true};
	    size_t maxDepth{10};  // Max recursion depth
	};

	/**
	 * @brief Information about a scanned package
	 */
	struct ScannedPackage {
	    Package package;
	    fs::path manifestPath;
	    ManifestFormat format;
	    std::chrono::system_clock::time_point scanTime;
	    std::optional<std::string> error;  // Set if package is corrupted/invalid
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
	        const fs::path& directory,
	        bool recursive = true) = 0;

	    /**
	     * @brief Check if a directory contains a valid package
	     */
	    virtual Result<ScannedPackage> ScanPackage(const fs::path& packagePath) = 0;

	    /**
	     * @brief Parse a manifest file
	     */
	    virtual Result<PackageInfo> ParseManifest(
	        const fs::path& manifestPath,
	        ManifestFormat format = ManifestFormat::JSON) = 0;

	    /**
	     * @brief Validate package structure and files
	     */
	    virtual Result<bool> ValidatePackageStructure(const fs::path& packagePath) = 0;

	    /**
	     * @brief Get last scan results (cached)
	     */
	    virtual std::optional<std::vector<ScannedPackage>> GetLastScanResults() const = 0;

	    /**
	     * @brief Watch directories for package changes (async)
	     */
	    virtual Result<void> StartWatching(std::function<void(const ScannedPackage&, bool added)> callback) = 0;

	    /**
	     * @brief Stop watching for changes
	     */
	    virtual void StopWatching() = 0;
	};
}