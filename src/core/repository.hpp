#pragma once

#include <plugify/repository.hpp>

namespace plugify {
	class IPackageScanner;
	class IHTTPDownloader;

	/**
	 * @brief Local filesystem repository with scanner integration
	 */
	class LocalPackageRepository : public IPackageRepository {
		explicit LocalPackageRepository(const std::filesystem::path& rootPath);

		std::string GetIdentifier() const override;
		bool IsAvailable() const override;
		Result<void> Refresh() override;
		Result<std::vector<Package>> GetPackages() const override;
		std::optional<Package> FindPackage(std::string_view packageId, const std::optional<plg::version>& version) const override;
		std::vector<Package> QueryPackages(const PackageQuery& query) const override;
		Result<PackageManifest> GetManifest(std::string_view packageId) const override;

	private:
		std::filesystem::path _rootPath;
		mutable std::unordered_map<std::string, Package> _packages;
		mutable std::chrono::system_clock::time_point _lastRefresh;

		void ScanDirectory();
		std::optional<PackageManifest> ParseManifestFile(const std::filesystem::path& path) const;
	};

	/**
	 * @brief Remote HTTP repository
	 */
	class RemotePackageRepository : public IPackageRepository {
	public:
		RemotePackageRepository(std::string repositoryUrl, std::shared_ptr<IHTTPDownloader> httpDownloader);

		std::string GetIdentifier() const override;
		bool IsAvailable() const override;
		Result<void> Refresh() override;
		Result<std::vector<Package>> GetPackages() const override;
		std::optional<Package> FindPackage(std::string_view packageId, const std::optional<plg::version>& version) const override;
		std::vector<Package> QueryPackages(const PackageQuery& query) const override;
		Result<PackageManifest> GetManifest(std::string_view packageId) const override;

	private:
		std::string _repositoryUrl;
		std::shared_ptr<IHTTPDownloader> _httpDownloader;
		mutable std::unordered_map<std::string, Package> _packages;
		mutable std::chrono::system_clock::time_point _lastRefresh;

		Result<void> FetchRepositoryIndex();
		Result<PackageManifest> FetchManifest(std::string_view packageId) const;
	};

} // nnamespace plugify