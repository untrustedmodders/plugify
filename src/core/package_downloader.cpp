#include "package_downloader.h"

#include <curl/curl.h>
#include <picosha2.h>
#include <miniz.h>
#include <wizard/wizard.h>
#include <utils/json.h>

using namespace wizard;

PackageDownloader::PackageDownloader(std::weak_ptr<IWizard> wizard) : WizardContext(std::move(wizard)) {
	auto debugStart = DateTime::Now();
	curl_global_init(CURL_GLOBAL_ALL);
	Initialize();
	WZ_LOG_DEBUG("PackageDownloader loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

PackageDownloader::~PackageDownloader() {
	curl_global_cleanup();
}

void PackageDownloader::Initialize() {
	auto wizard = _wizard.lock();
	if (!wizard) {
		WZ_LOG_ERROR("Cannot initialize Package Downloader");
		return;
	}

	auto& config = wizard->GetConfig();

	_packageVerification = config.packageVerification;
	_packageVerifyUrl = config.packageVerifyUrl;

	if (!_packageVerifyUrl.empty()) {
		WZ_LOG_INFO("Not found custom verified packages URL in config: {}", _packageVerifyUrl);
	} else {
		WZ_LOG_INFO("Custom verified packages URL not found in config, using default URL");
		_packageVerifyUrl = kDefaultPackageList;
	}
}

size_t WriteToString(void* ptr, size_t size, size_t count, std::string* stream) {
	size_t totalSize = size * count;
	stream->append(static_cast<char*>(ptr), totalSize);
	return totalSize;
}

void PackageDownloader::FetchPackagesListFromAPI() {
	std::string json;

	auto curl_close = [](CURL* curl){ curl_easy_cleanup(curl); };
	std::unique_ptr<CURL, decltype(curl_close)> handle{curl_easy_init(), curl_close};

	curl_easy_setopt(handle.get(), CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(handle.get(), CURLOPT_TIMEOUT, 30L);
	curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle.get(), CURLOPT_URL, _packageVerifyUrl.c_str());
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
		WZ_LOG_ERROR("File from '{}' has JSON parsing error: {}", _packageVerifyUrl, glz::format_error(packages.error(), json));
		return;
	}

	_packages = std::move(*packages);

	WZ_LOG_INFO("Done loading verified packages list.");
}

bool PackageDownloader::IsPackageAuthorized(const Package& package) {
	if (!_packageVerification)
		return true;

	auto it = _packages.verified.find(package.name);
	if (it == _packages.verified.end())
		return false;

	return std::get<VerifiedPackageDetails>(*it).versions.contains(std::to_string(package.version));
}

void PackageDownloader::Download(const Package& package) {
	if (!IsPackageAuthorized(package)) {
		WZ_LOG_WARNING("Tried to download a package that is not verified, aborting");
		return;
	}

	auto fetchingResult = FetchPackageFromURL(package);
	if (!fetchingResult.has_value()) {
		WZ_LOG_ERROR("Something went wrong while fetching archive, aborting");
		_packageState.state = PackageInstallState::PackageFetchingFailed;
		return;
	}

	fs::path& archiveLocation = fetchingResult.value().path;
	std::string& expectedHash = _packages.verified[package.name].versions[std::to_string(package.version)].checksum;
	
	if (!IsPackageLegit(archiveLocation, expectedHash)) {
		WZ_LOG_WARNING("Archive hash does not match expected checksum, aborting");
		_packageState.state = PackageInstallState::PackageCorrupted;
		return;
	}

	auto wizard = _wizard.lock();
	if (!wizard) {
		WZ_LOG_ERROR("Cannot get destination folder");
		_packageState.state = PackageInstallState::FailedCreatingDirectory;
		return;
	}

	auto finalLocation = wizard->GetConfig().baseDir / (package.languageModule ? "modules" : "plugins");

	if (!fs::exists(finalLocation)) {
		if (!fs::create_directory(finalLocation)) {
			WZ_LOG_ERROR("Error creating output directory {}", finalLocation.string());
			_packageState.state = PackageInstallState::FailedCreatingDirectory;
			return;
		}
	}

	bool success;
	if (package.extractArchive)
		success = ExtractPackage(archiveLocation, finalLocation);
	else
		success = MovePackage(archiveLocation, finalLocation);

	if (success) {
		WZ_LOG_INFO("Done downloading {}", package.name);
		_packageState.state = PackageInstallState::Done;
	} else {
		WZ_LOG_INFO("Failed downloading {}", package.name);
		_packageState.state = PackageInstallState::Failed;
	}
}

bool PackageDownloader::ExtractPackage(const fs::path& packagePath, const fs::path& extractPath) {
	std::ifstream zipFile{packagePath, std::ios::binary};
	if (!zipFile.is_open()) {
		WZ_LOG_ERROR("Cannot open archive located at {}", packagePath.string());
		_packageState.state = PackageInstallState::FailedReadingArchive;
		return false;
	}

	_packageState.state = PackageInstallState::Extracting;

	std::vector<char> zipData{(std::istreambuf_iterator<char>(zipFile)), std::istreambuf_iterator<char>()};

	mz_zip_archive zipArchive;
	mz_zip_reader_init_mem(&zipArchive, zipData.data(), zipData.size(), 0);

	_packageState.total = zipArchive.m_archive_size;
	_packageState.progress = 0;

	size_t numFiles = mz_zip_reader_get_num_files(&zipArchive);

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat fileStat;
		if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat)) {
			WZ_LOG_ERROR("Error getting file stat: {} in zip located at {}", i, packagePath.string());
			_packageState.state = PackageInstallState::FailedReadingArchive;
			break;
		}

		std::vector<char> fileData(fileStat.m_uncomp_size);

		if (!mz_zip_reader_extract_to_mem(&zipArchive, i, fileData.data(), fileData.size(), 0)) {
			WZ_LOG_ERROR("Failed extracting file: {}", fileStat.m_filename);
			_packageState.state = PackageInstallState::NoMemoryAvailable;
			break;
		}

		std::ofstream outputFile{extractPath / fileStat.m_filename, std::ios::binary};
		if (outputFile.is_open()) {
			outputFile.write(fileData.data(), static_cast<std::streamsize>(fileData.size()));
		} else {
			WZ_LOG_ERROR("Failed creating destination file: {}", fileStat.m_filename);
			_packageState.state = PackageInstallState::FailedWritingToDisk;
			break;
		}

		_packageState.progress += fileData.size();
		_packageState.ratio = std::roundf(static_cast<float>(_packageState.progress) / static_cast<float>(_packageState.total) * 100.0f);
	}

	mz_zip_reader_end(&zipArchive);

	return _packageState.state == PackageInstallState::Extracting;
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
	WZ_LOG_INFO("Fetching package archive from {}", package.url);

	auto fileName = std::format("{}-{}.zip", package.name, std::to_string(package.version));
	auto downloadPath = std::filesystem::temp_directory_path() / fileName;
	WZ_LOG_INFO("Downloading archive to {}", downloadPath.string());

	_packageState.state = PackageInstallState::Downloading;

	std::ofstream file{downloadPath, std::ios::binary};

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

bool PackageDownloader::IsPackageLegit(const fs::path& packagePath, std::string_view expectedChecksum) {
	if (!_packageVerification)
		return true;

	_packageState.state = PackageInstallState::Checksuming;

	std::ifstream file{packagePath, std::ios::binary};
	if (!file.is_open()) {
		WZ_LOG_ERROR("Unable to open archive: {}", packagePath.string());
		_packageState.state = PackageInstallState::FailedReadingArchive;
		return false;
	}

	std::vector<uint8_t> bytes(picosha2::k_digest_size);
	picosha2::hash256(file, bytes.begin(), bytes.end());
	std::string hash = picosha2::bytes_to_hex_string(bytes.begin(), bytes.end());

	WZ_LOG_INFO("Expected checksum: {}", expectedChecksum.data());
	WZ_LOG_INFO("Computed checksum: {}", hash);

	bool equal = expectedChecksum.compare(hash) == 0;
	if (!equal)
		_packageState.state = PackageInstallState::PackageCorrupted;
	return equal;
}

bool PackageDownloader::MovePackage(const fs::path& filePath, const fs::path& movePath) {
#if WIZARD_PLATFORM_LINUX
	auto cmd = std::format("mv '{}' '{}'", filePath.string(), movePath.string());
    system(cmd.c_str());
#else
    auto cmd = std::format("move '{}' '{}'", filePath.string(), movePath.string());
    system(cmd.c_str());
#endif
    return fs::exists(movePath / filePath.filename());
}

PackageDownloader::FetchResult::FetchResult(fs::path filePath) : path{std::move(filePath)} {
}

PackageDownloader::FetchResult::~FetchResult() {
	try {
		if (fs::exists(path))
			fs::remove(path);
	} catch (const std::exception& e) {
		WZ_LOG_ERROR("Error while removing downloaded archive: {}", e.what());
	}
}

std::string PackageState::ToString(int barWidth) const {
	std::stringstream ss;
	ss << "[";
	int pos = barWidth * static_cast<int>(ratio);
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) ss << "=";
		else if (i == pos) ss << ">";
		else ss << " ";
	}
	ss << "] " << static_cast<int>(ratio * 100.0) << " %\n";
	return ss.str();
}
