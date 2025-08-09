#pragma once

#include <string>
#include <variant>
#include <vector>
#include <span>
#include <filesystem>

#include "package.hpp"
#include "expected.hpp"

namespace plugify {
	enum class PackageError {
		NotFound,
		VersionConflict,
		DependencyMissing,
		InvalidPackage,
		NetworkError,
		FileSystemError,
		PermissionDenied
	};

	template<typename T>
	using Result = plg::expected<T, PackageError>;

	// Repository abstraction interface
	class IPackageRepository {
	public:
	    virtual ~IPackageRepository() = default;

	    /**
	     * @brief Fetch available packages from this repository
	     * @return Result containing vector of remote packages or error
	     */
	    virtual Result<std::vector<RemotePackage>> FetchPackages() const = 0;

	    /**
	     * @brief Search for packages by name pattern
	     * @param pattern Search pattern (can be regex or simple wildcard)
	     * @return Result containing matching packages or error
	     */
	    virtual Result<std::vector<RemotePackage>> SearchPackages(std::string_view pattern) const = 0;

	    /**
	     * @brief Download a specific package version
	     * @param packageName Name of the package to download
	     * @param version Version to download
	     * @param destinationPath Where to download the package
	     * @return Result indicating success or error
	     */
	    virtual Result<void> DownloadPackage(std::string_view packageName,
	                                        plg::version version,
	                                        const std::filesystem::path& destinationPath) const = 0;

	    /**
	     * @brief Get repository identifier
	     * @return Repository identifier string
	     */
	    virtual std::string_view GetIdentifier() const = 0;
	};

	// Local package discovery interface
	class ILocalPackageProvider {
	public:
	    virtual ~ILocalPackageProvider() = default;

	    /**
	     * @brief Scan for locally installed packages
	     * @param scanPaths Paths to scan for packages
	     * @return Result containing vector of local packages or error
	     */
	    virtual Result<std::vector<LocalPackage>> ScanLocalPackages(
	        std::span<const std::filesystem::path> scanPaths) const = 0;

	    /**
	     * @brief Validate a local package
	     * @param packagePath Path to package directory
	     * @return Result containing local package info or error
	     */
	    virtual Result<LocalPackage> ValidatePackage(const std::filesystem::path& packagePath) const = 0;
	};

} // namespace plugify
