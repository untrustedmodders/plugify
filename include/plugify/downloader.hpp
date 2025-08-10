#pragma once

/**
 * @brief Package downloader interface (wraps IHTTPDownloader)
 */
class IPackageDownloader {
public:
	using ProgressCallback = std::function<bool(const Package&, size_t bytesDownloaded, size_t totalBytes)>;

	virtual ~IPackageDownloader() = default;

	/**
	 * @brief Download package to local storage
	 */
	virtual Result<std::filesystem::path> DownloadPackage(
		const Package& package,
		const std::filesystem::path& targetDirectory,
		ProgressCallback progressCallback = nullptr) = 0;

	/**
	 * @brief Verify package integrity
	 */
	virtual Result<bool> VerifyPackage(
		const std::filesystem::path& packagePath,
		const std::string& expectedChecksum) const = 0;
};