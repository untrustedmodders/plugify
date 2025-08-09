#pragma once

#include <plugify/repository.hpp>

namespace plugify {
	/**
	 * @brief HTTP-based repository implementation
	 */
	class HttpPackageRepository : public IPackageRepository {
	public:
	    explicit HttpPackageRepository(std::string baseUrl);
	    ~HttpPackageRepository() override = default;

	    Result<std::vector<RemotePackage>> FetchPackages() const override;
	    Result<std::vector<RemotePackage>> SearchPackages(std::string_view pattern) const override;
	    Result<void> DownloadPackage(std::string_view packageName,
	                                const plg::version& version,
	                                const std::filesystem::path& destinationPath) const override;
	    std::string_view GetIdentifier() const override;

	private:
	    std::string _baseUrl;
	    std::string _identifier;

	    // HTTP client would be injected or created here
	    // Implementation would handle HTTP requests, JSON parsing, etc.
	};

	/**
	 * @brief Filesystem-based repository implementation
	 */
	/*class FilesystemPackageRepository : public IPackageRepository {
	public:
	    explicit FilesystemPackageRepository(std::filesystem::path repositoryPath);
	    ~FilesystemPackageRepository() override = default;

	    Result<std::vector<RemotePackage>> FetchPackages() const override;
	    Result<std::vector<RemotePackage>> SearchPackages(std::string_view pattern) const override;
	    Result<void> DownloadPackage(std::string_view packageName,
	                                const plg::version& version,
	                                const std::filesystem::path& destinationPath) const override;
	    std::string_view GetIdentifier() const override;

	private:
	    std::filesystem::path _repositoryPath;
	    std::string _identifier;
	};*/

	/**
	 * @brief Default implementation for local package discovery
	 */
	class FilesystemLocalPackageProvider : public ILocalPackageProvider {
	public:
	    FilesystemLocalPackageProvider() = default;
	    ~FilesystemLocalPackageProvider() override = default;

	    Result<std::vector<LocalPackage>> ScanLocalPackages(
	        std::span<const std::filesystem::path> scanPaths) const override;
	    Result<LocalPackage> ValidatePackage(const std::filesystem::path& packagePath) const override;

	private:
	    // Implementation would scan directories for manifest files
	    // Parse manifest files and create LocalPackage objects
	};
} // nnamespace plugify