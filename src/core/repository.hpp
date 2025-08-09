#pragma once

#include <plugify/repository.hpp>

namespace plugify {
	// Concrete repository implementations
	class HTTPRepository : public IPackageRepository {
	public:
		HTTPRepository(std::string_view url, std::shared_ptr<IHTTPDownloader> downloader);

		Result<std::vector<RemotePackage>> FetchPackages() override;
		Result<std::vector<RemotePackage>> SearchPackages(std::string_view query) override;
		Result<std::filesystem::path> DownloadPackage(
			const RemotePackage& package,
			const PackageVersion& version,
			std::function<bool(uint32_t, uint32_t)> progressCallback
		) override;
		std::string GetIdentifier() const override;
		bool IsAvailable() override;

	private:
		std::string _url;
		std::string _identifier;
		std::shared_ptr<IHTTPDownloader> _downloader;
		std::optional<std::vector<RemotePackage>> _cachedPackages;
	};

	class FileSystemRepository : public IPackageRepository {
	public:
		FileSystemRepository(const std::filesystem::path& path);

		Result<std::vector<RemotePackage>> FetchPackages() override;
		Result<std::vector<RemotePackage>> SearchPackages(std::string_view query) override;
		Result<std::filesystem::path> DownloadPackage(
			const RemotePackage& package,
			const PackageVersion& version,
			std::function<bool(uint32_t, uint32_t)> progressCallback
		) override;
		std::string GetIdentifier() const override;
		bool IsAvailable() override;

	private:
		std::filesystem::path _path;
		std::string _identifier;
	};

	// Concrete scanner implementation
	class PackageScanner : public IPackageScanner {
	public:
		PackageScanner() = default;

		Result<std::vector<LocalPackage>> ScanDirectory(const std::filesystem::path& path) override;
		Result<bool> VerifyPackage(const LocalPackage& package) override;
		Result<std::shared_ptr<Descriptor>> LoadDescriptor(const std::filesystem::path& manifestPath) override;

	private:
		// Implementation would parse manifest files, verify checksums, etc.
		Result<std::shared_ptr<PluginDescriptor>> ParsePluginManifest(const std::filesystem::path& path);
		Result<std::shared_ptr<LanguageModuleDescriptor>> ParseLanguageModuleManifest(const std::filesystem::path& path);
	};
} // nnamespace plugify