#include "package_downloader.h"
#include "module.h"
#include "plugin.h"

#include <curl/curl.h>
#include <picosha2.h>
#include <miniz.h>
#include <wizard/wizard.h>
#include <utils/json.h>

using namespace wizard;

PackageDownloader::PackageDownloader(Config config) : _config(std::move(config)) {
	auto debugStart = DateTime::Now();
	curl_global_init(CURL_GLOBAL_ALL);
	FetchPackagesListFromAPI();
	WZ_LOG_DEBUG("PackageDownloader loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

PackageDownloader::~PackageDownloader() {
	curl_global_cleanup();
}

size_t WriteToString(void* ptr, size_t size, size_t count, std::string* stream) {
	size_t totalSize = size * count;
	stream->append(static_cast<char*>(ptr), totalSize);
	return totalSize;
}

void PackageDownloader::FetchPackagesListFromAPI() {
	if (!_config.packageVerification)
		return;

	if (!_config.packageVerifyUrl.empty()) {
		WZ_LOG_INFO("Found custom verified packages URL in config: '{}'", _config.packageVerifyUrl);
	} else {
		WZ_LOG_INFO("Custom verified packages URL not found in config, using default URL");
		_config.packageVerifyUrl = kDefaultPackageList;
	}

	std::string json;

	auto curl_close = [](CURL* curl){ curl_easy_cleanup(curl); };
	std::unique_ptr<CURL, decltype(curl_close)> handle{curl_easy_init(), curl_close};

	curl_easy_setopt(handle.get(), CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(handle.get(), CURLOPT_TIMEOUT, 30L);
	curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle.get(), CURLOPT_URL, _config.packageVerifyUrl.c_str());
	curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, &json);
	curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, WriteToString);
	CURLcode result = curl_easy_perform(handle.get());

	if (result == CURLcode::CURLE_OK) {
		WZ_LOG_INFO("Packages list successfully fetched.");
	} else {
		WZ_LOG_ERROR("Fetching packages list failed: {}", curl_easy_strerror(result));
		return;
	}

	WZ_LOG_INFO("Loading packages configuration...");

	auto packages = glz::read_json<VerifiedPackageMap>(json);
	if (!packages.has_value()) {
		WZ_LOG_ERROR("File from '{}' has JSON parsing error: {}", _config.packageVerifyUrl, glz::format_error(packages.error(), json));
		return;
	}

	_packages = std::move(*packages);

	// No verified packages found, disable then
	if (_packages.verified.empty()) {
		WZ_LOG_INFO("Empty verification packages list, disabling...");
		_config.packageVerification = false;
	} else {
		WZ_LOG_INFO("Done loading verified packages list.");
	}
}

bool PackageDownloader::IsPackageAuthorized(const Package& package) {
	if (!_config.packageVerification)
		return true;

	auto it = _packages.verified.find(package.name);
	if (it == _packages.verified.end())
		return false;

	return std::get<VerifiedPackageDetails>(*it).versions.contains(std::to_string(package.version));
}

std::optional<Package> PackageDownloader::Update(const Package& package) {
	_packageState.error = PackageError::None;

	if (!IsValidURL(package.url)) {
		WZ_LOG_ERROR("Package: '{}' (v{}) has invalid update URL: '{}'", package.name, package.version, package.url);
		_packageState.error = PackageError::InvalidURL;
		return {};
	}

	_packageState.state = PackageInstallState::Updating;

	std::string json;

	auto curl_close = [](CURL* curl){ curl_easy_cleanup(curl); };
	std::unique_ptr<CURL, decltype(curl_close)> handle{curl_easy_init(), curl_close};

	curl_easy_setopt(handle.get(), CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(handle.get(), CURLOPT_TIMEOUT, 30L);
	curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle.get(), CURLOPT_URL, package.url.c_str());
	curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, &json);
	curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, WriteToString);
	CURLcode result = curl_easy_perform(handle.get());

	if (result == CURLcode::CURLE_OK) {
		WZ_LOG_INFO("Package '{}' (v{}) successfully fetched.", package.name, package.version);
	} else {
		WZ_LOG_ERROR("Fetching package '{}' (v{}) failed: {}", package.name, package.version, curl_easy_strerror(result));
		return {};
	}

	WZ_LOG_INFO("Loading package info...");

	auto newPackage = glz::read_json<Package>(json);
	if (!newPackage.has_value()) {
		WZ_LOG_ERROR("File from '{}' has JSON parsing error: {}", package.url, glz::format_error(newPackage.error(), json));
		return {};
	}

	if (newPackage->version > package.version) {
		WZ_LOG_INFO("Update available, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(package.version, newPackage->version), newPackage->name, std::min(package.version, newPackage->version));
		return std::move(*newPackage);
	}

	WZ_LOG_INFO("Done loading package info. (No update available)");

	return {};
}

std::optional<fs::path> PackageDownloader::Download(const Package& package)  {
	_packageState.error = PackageError::None;

	if (!IsValidURL(package.url)) {
		WZ_LOG_ERROR("Package: '{}' (v{}) has invalid download URL : '{}'", package.name, package.version, package.url);
		_packageState.error = PackageError::InvalidURL;
		return {};
	}

	if (!IsPackageAuthorized(package)) {
		WZ_LOG_WARNING("Tried to download a package that is not verified, aborting");
		_packageState.error = PackageError::PackageAuthorizationFailed;
		return {};
	}

	auto fetchingResult = FetchPackageFromURL(package);
	if (!fetchingResult.has_value()) {
		WZ_LOG_ERROR("Something went wrong while fetching archive, aborting");
		_packageState.error = PackageError::PackageFetchingFailed;
		return {};
	}

	fs::path& archiveLocation = fetchingResult.value().path;
	
	if (!IsPackageLegit(package, archiveLocation)) {
		WZ_LOG_WARNING("Archive hash does not match expected checksum, aborting");
		_packageState.error = PackageError::PackageCorrupted;
		return {};
	}

	fs::path finalLocation = _config.baseDir / (package.languageModule ? "modules" : "plugins") / archiveLocation.filename().replace_extension();

	std::error_code ec;
	if (!fs::exists(finalLocation, ec) || !fs::is_directory(finalLocation, ec)) {
		if (!fs::create_directories(finalLocation, ec)) {
			WZ_LOG_ERROR("Error creating output directory '{}'", finalLocation.string());
			_packageState.error = PackageError::FailedCreatingDirectory;
			return {};
		}
	}

	if (ExtractPackage(archiveLocation, finalLocation, package.languageModule)) {
		WZ_LOG_INFO("Done downloading '{}'", package.name);
		_packageState.state = PackageInstallState::Done;
		return { std::move(finalLocation) };
	} else {
		WZ_LOG_INFO("Failed downloading '{}'", package.name);
		_packageState.state = PackageInstallState::Failed;
		return {};
	}
}

bool PackageDownloader::ExtractPackage(const fs::path& packagePath, const fs::path& extractPath, bool isModule) {
	std::ifstream zipFile{packagePath, std::ios::binary};
	if (!zipFile.is_open()) {
		WZ_LOG_ERROR("Cannot open archive located at '{}'", packagePath.string());
		_packageState.error = PackageError::FailedReadingArchive;
		return false;
	}

	WZ_LOG_INFO("Start extracting....");

	_packageState.state = PackageInstallState::Extracting;

	std::vector<char> zipData{(std::istreambuf_iterator<char>(zipFile)), std::istreambuf_iterator<char>()};

	auto zip_close = [](mz_zip_archive* zipArchive){ mz_zip_reader_end(zipArchive); delete zipArchive; };
	std::unique_ptr<mz_zip_archive, decltype(zip_close)> zipArchive{new mz_zip_archive, zip_close};
	std::memset(zipArchive.get(), 0, sizeof(mz_zip_archive));

	mz_zip_reader_init_mem(zipArchive.get(), zipData.data(), zipData.size(), 0);

	_packageState.total = zipArchive->m_archive_size;
	_packageState.progress = 0;

	size_t numFiles = mz_zip_reader_get_num_files(zipArchive.get());
	std::vector<mz_zip_archive_file_stat> fileStats(numFiles);

	bool foundDescriptor = false;
	std::string_view extension = isModule ? Module::kFileExtension : Plugin::kFileExtension;

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat& fileStat = fileStats[i];

		if (!mz_zip_reader_file_stat(zipArchive.get(), i, &fileStat)) {
			WZ_LOG_ERROR("Error getting file stat: {} in zip located at '{}'", i, packagePath.string());
			_packageState.error = PackageError::FailedReadingArchive;
			return false;
		}

		fs::path filename{ fileStat.m_filename };
		if (filename.extension().string() == extension) {
			foundDescriptor = true;
		}
	}

	if (!foundDescriptor) {
		WZ_LOG_ERROR("Package descriptor *{} missing inside zip located at '{}'", extension, packagePath.string());
		_packageState.error = PackageError::PackageMissingDescriptor;
		return false;
	}

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat& fileStat = fileStats[i];

		std::vector<char> fileData(fileStat.m_uncomp_size);

		if (!mz_zip_reader_extract_to_mem(zipArchive.get(), i, fileData.data(), fileData.size(), 0)) {
			WZ_LOG_ERROR("Failed extracting file: '{}'", fileStat.m_filename);
			_packageState.error = PackageError::NoMemoryAvailable;
			return false;
		}

		fs::path finalPath = extractPath / fileStat.m_filename;
		fs::path finalLocation = finalPath.parent_path();

		std::error_code ec;
		if (!fs::exists(finalLocation, ec) || !fs::is_directory(finalLocation, ec)) {
			if (!fs::create_directories(finalLocation, ec)) {
				WZ_LOG_ERROR("Error creating output directory '{}'", finalLocation.string());
				_packageState.error = PackageError::FailedCreatingDirectory;
				return false;
			}
		}

		std::ofstream outputFile{finalPath, std::ios::binary};
		if (outputFile.is_open()) {
			outputFile.write(fileData.data(), static_cast<std::streamsize>(fileData.size()));
		} else {
			WZ_LOG_ERROR("Failed creating destination file: '{}'", fileStat.m_filename);
			_packageState.error = PackageError::FailedWritingToDisk;
			return false;
		}

		_packageState.progress += fileStat.m_comp_size;
		_packageState.ratio = std::roundf(static_cast<float>(_packageState.progress) / static_cast<float>(_packageState.total) * 100.0f);
	}

	return true;
}

size_t WriteToStream(void* contents, size_t size, size_t count, std::ostream* stream) {
	size_t totalSize = size * count;
	stream->write(static_cast<char*>(contents), static_cast<std::streamsize>(totalSize));
	return totalSize;
}

int PackageDownloader::PackageFetchingProgressCallback(void* ptr, curl_off_t totalDownloadSize, curl_off_t finishedDownloadSize, curl_off_t totalToUpload, curl_off_t nowUploaded) {
	if (totalDownloadSize != 0 && finishedDownloadSize != 0) {
		auto currentDownloadProgress = std::roundf(static_cast<float>(finishedDownloadSize) / static_cast<float>(totalDownloadSize) * 100.0f);
		auto instance = static_cast<PackageDownloader*>(ptr);
		instance->_packageState.progress = static_cast<size_t>(finishedDownloadSize);
		instance->_packageState.total = static_cast<size_t>(totalDownloadSize);
		instance->_packageState.ratio = currentDownloadProgress;
	}
	return 0;
}

std::optional<PackageDownloader::FetchResult> PackageDownloader::FetchPackageFromURL(const Package& package) {
	WZ_LOG_INFO("Fetching package archive from: '{}'", package.url);

	auto fileName = std::format("{}-{}-{}.zip", package.name, std::to_string(package.version), wizard::DateTime::Get("%Y_%m_%d_%H_%M_%S"));
	auto downloadPath = std::filesystem::temp_directory_path() / fileName;
	WZ_LOG_INFO("Downloading archive to '{}'", downloadPath.string());

	_packageState.state = PackageInstallState::Downloading;

	std::ofstream file{downloadPath, std::ios::binary};
	if (!file.is_open()) {
		WZ_LOG_ERROR("Failed creating destination file: {}", downloadPath.string());
		_packageState.error = PackageError::FailedWritingToDisk;
		return {};
	}

	WZ_LOG_INFO("Start downloading....");

	auto curl_close = [](CURL* curl){ curl_easy_cleanup(curl); };
	std::unique_ptr<CURL, decltype(curl_close)> handle{curl_easy_init(), curl_close};

	curl_easy_setopt(handle.get(), CURLOPT_URL, package.url.c_str());
	curl_easy_setopt(handle.get(), CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, &file);
	curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, WriteToStream);
	curl_easy_setopt(handle.get(), CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(handle.get(), CURLOPT_XFERINFOFUNCTION, PackageFetchingProgressCallback);
	curl_easy_setopt(handle.get(), CURLOPT_XFERINFODATA, this);
	CURLcode result = curl_easy_perform(handle.get());

	if (result == CURLcode::CURLE_OK) {
		WZ_LOG_INFO("Package archive successfully fetched.");
	} else {
		WZ_LOG_ERROR("Fetching package archive failed: {}", curl_easy_strerror(result));
		return {};
	}

	return { FetchResult{ std::move(downloadPath) } };
}

bool PackageDownloader::IsPackageLegit(const Package& package, const fs::path& packagePath) {
	if (!_config.packageVerification)
		return true;

	_packageState.state = PackageInstallState::Checksuming;

	std::ifstream file{packagePath, std::ios::binary};
	if (!file.is_open()) {
		WZ_LOG_ERROR("Unable to open archive: '{}'", packagePath.string());
		_packageState.error = PackageError::FailedReadingArchive;
		return false;
	}

	std::string& expectedHash = _packages.verified[package.name].versions[std::to_string(package.version)].checksum;

	std::vector<uint8_t> bytes(picosha2::k_digest_size);
	picosha2::hash256(file, bytes.begin(), bytes.end());
	std::string hash = picosha2::bytes_to_hex_string(bytes.begin(), bytes.end());

	WZ_LOG_INFO("Expected checksum: {}", expectedHash);
	WZ_LOG_INFO("Computed checksum: {}", hash);

	bool equal = expectedHash == hash;
	if (!equal)
		_packageState.error = PackageError::PackageCorrupted;
	return equal;
}

PackageDownloader::FetchResult::FetchResult(fs::path filePath) : path{std::move(filePath)} {
}

PackageDownloader::FetchResult::~FetchResult() {
	std::error_code ec;
	if (fs::exists(path, ec))
		fs::remove(path, ec);
}

std::string PackageState::GetProgress(int barWidth) const {
	std::stringstream ss;
	ss << "[";
	int pos = static_cast<int>(static_cast<float>(barWidth) * (ratio / 100.0f));
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) ss << "=";
		else if (i == pos) ss << ">";
		else ss << " ";
	}
	ss << "] " << static_cast<int>(ratio) << " %\n";
	return ss.str();
}

std::string_view PackageState::GetError() const {
	switch (error) {
		case PackageError::InvalidURL:
			return "Invalid URL";
		case PackageError::FailedReadingArchive:
			return "Failed reading archive";
		case PackageError::FailedCreatingDirectory:
			return "Failed creating directory";
		case PackageError::FailedWritingToDisk:
			return "Failed writing to disk";
		case PackageError::PackageFetchingFailed:
			return "Package fetching failed";
		case PackageError::PackageAuthorizationFailed:
			return "Package authorization failed";
		case PackageError::PackageCorrupted:
			return "Downloaded archive checksum does not match verified hash";
		case PackageError::NoMemoryAvailable:
			return "No memory available";
		case PackageError::NotFound:
			return "Package not found, not currently being downloaded";
		case PackageError::None:
		default:
			return "";
	}
}
