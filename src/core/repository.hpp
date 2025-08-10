#pragma once

#include <plugify/repository.hpp>

namespace plugify {
	class IPackageScanner;
	class IHTTPDownloader;

	/**
	 * @brief Local filesystem repository with scanner integration
	 */
	class LocalRepository : public IPackageRepository {
	private:
		fs::path _rootPath;
		std::shared_ptr<IPackageScanner> _scanner;
    
	public:
		explicit LocalRepository(fs::path rootPath, std::unique_ptr<IPackageScanner> scanner);
    
		Result<std::vector<Package>> EnumeratePackages() override;
		Result<std::vector<Package>> SearchPackages(std::string_view query) override;
		Result<Package> GetPackage(const PackageId& id, const std::optional<plg::version>& version) override;
		Result<fs::path> DownloadPackage(const Package& package, const fs::path& destination, ProgressCallback progress) override;
		Result<bool> VerifyPackage(const Package& package, const fs::path& path) override;
		std::string GetName() const override;
		Result<bool> IsAvailable() override;
	};

	/**
	 * @brief Remote HTTP repository
	 */
	class RemoteRepository : public IPackageRepository {
	private:
		std::string _baseUrl;
		std::shared_ptr<IHTTPDownloader> _downloader;
		mutable std::optional<std::vector<Package>> _cachedPackages;
    
	public:
		RemoteRepository(std::string baseUrl, std::shared_ptr<IHTTPDownloader> downloader);
    
		Result<std::vector<Package>> EnumeratePackages() override;
		Result<std::vector<Package>> SearchPackages(std::string_view query) override;
		Result<Package> GetPackage(const PackageId& id, const std::optional<plg::version>& version) override;
		Result<fs::path> DownloadPackage(const Package& package, const fs::path& destination, ProgressCallback progress) override;
		Result<bool> VerifyPackage(const Package& package, const fs::path& path) override;
		std::string GetName() const override;
		Result<bool> IsAvailable() override;
	};

} // nnamespace plugify