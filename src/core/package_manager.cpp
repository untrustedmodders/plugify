#include "package_manager.hpp"
#include "module.hpp"
#include "package_manifest.hpp"
#include "plugin.hpp"

#include <miniz.h>
#include <plugify/plugify.hpp>
#include <utils/file_system.hpp>
#include <utils/json.hpp>
#include <utils/defer.hpp>
#include <utils/strings.hpp>
#if PLUGIFY_DOWNLOADER
#include <utils/http_downloader.hpp>
#include <utils/sha256.hpp>
#endif // PLUGIFY_DOWNLOADER

using namespace plugify;

static std::array<std::pair<std::string_view, std::string_view>, 2> packageTypes {
	std::pair{ "modules", Module::kFileExtension },
	std::pair{ "plugins", Plugin::kFileExtension },
	// Might add more package types in future
};

PackageManager::PackageManager(std::weak_ptr<IPlugify> plugify) : PlugifyContext(std::move(plugify)) {
}

PackageManager::~PackageManager() {
	Terminate();
}

bool PackageManager::Initialize() {
	if (IsInitialized())
		return false;

	auto debugStart = DateTime::Now();

#if PLUGIFY_DOWNLOADER
	_httpDownloader = IHTTPDownloader::Create();
#endif // PLUGIFY_DOWNLOADER

	LoadAllPackages();

	_inited = true;

	PL_LOG_DEBUG("PackageManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
	return true;
}

void PackageManager::Terminate() {
	if (!IsInitialized())
		return;

	_localPackages.clear();
	_remotePackages.clear();
	_missedPackages.clear();
	_conflictedPackages.clear();

#if PLUGIFY_DOWNLOADER
	_httpDownloader.reset();
#endif // PLUGIFY_DOWNLOADER

	_inited = false;
}

bool PackageManager::IsInitialized() const {
	return _inited;
}

bool PackageManager::Reload() {
	if (!IsInitialized())
		return false;

	LoadAllPackages();

	return true;
}

void PackageManager::LoadAllPackages() {
	LoadLocalPackages();
#if PLUGIFY_DOWNLOADER
	LoadRemotePackages();
	FindDependencies();
#endif // PLUGIFY_DOWNLOADER
}

static bool IsSupportsPlatform(const std::optional<std::vector<std::string>>& supportedPlatforms) {
	if (!supportedPlatforms || supportedPlatforms->empty())
		return true;

	constexpr std::string_view platform = PLUGIFY_PLATFORM; // e.g., "linux_x64"
	static_assert(platform.find('_') != std::string_view::npos, "PLUGIFY_PLATFORM must be in the format 'name-arch'");
	constexpr std::string_view platform_name = platform.substr(0, platform.find('_')); // e.g., "linux"

	for (const auto& supported : *supportedPlatforms) {
		if (supported == platform || supported == platform_name) {
			return true;
		}
	}

	return false;
}

static void RemoveUnsupported(RemotePackagePtr& package) {
	std::set<PackageVersion>& versions = package->versions;
	for (auto it = versions.begin(); it != versions.end(); ) {
		if (!IsSupportsPlatform(it->platforms)) {
			it = versions.erase(it);
		} else {
			++it;
		}
	}
}

template<typename T>
static LocalPackagePtr GetPackageFromDescriptor(const fs::path& path, const std::string& name) {
	auto json = FileSystem::ReadText(path);
	auto dest = glz::read_jsonc<std::shared_ptr<T>>(json);
	if (!dest.has_value()) {
		PL_LOG_ERROR("Package: '{}' has JSON parsing error: {}", name, glz::format_error(dest.error(), json));
		return {};
	}
	auto& descriptor = *dest;

	if (!IsSupportsPlatform(descriptor->supportedPlatforms))
		return {};

	std::vector<std::string> errors = descriptor->Validate(name);
	if (!errors.empty()) {
		PL_LOG_ERROR("Package: '{}' has error(s): {}", name, plg::join(errors, ", "));
		return {};
	}

	auto version = descriptor->version;
	auto type = descriptor->GetType();
	descriptor->versionName = version.to_string_noexcept();
	return std::make_shared<LocalPackage>(Package{name, type}, path, std::move(version), std::move(descriptor));
}

void PackageManager::LoadLocalPackages()  {
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_DEBUG("Loading local packages");

	_localPackages.clear();
	//_localPackages.reserve()

	FileSystem::ReadDirectory(plugify->GetConfig().baseDir, [&](const fs::path& path, int depth) {
		if (depth != 1)
			return;

		auto extension = path.extension();
		bool isModule = extension == Module::kFileExtension;
		if (!isModule && extension != Plugin::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();
		if (name.empty())
			return;

		auto package = isModule ?
				GetPackageFromDescriptor<LanguageModuleDescriptor>(path, name) :
				GetPackageFromDescriptor<PluginDescriptor>(path, name);
		if (!package)
			return;

		auto it = _localPackages.find(name);
		if (it == _localPackages.end()) {
			_localPackages.emplace(std::move(name), std::move(package));
		} else {
			auto& [_, existingPackage] = *it;

			auto& existingVersion = existingPackage->version;
			if (existingVersion != package->version) {
				PL_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package->version), name, std::min(existingVersion, package->version));

				if (existingVersion < package->version) {
					existingPackage = std::move(package);
				}
			} else {
				PL_LOG_VERBOSE("The same version (v{}) of package '{}' exists at '{}' - second location will be ignored.", existingVersion, name, path.string());
			}
		}
	}, 3);
}

#if PLUGIFY_DOWNLOADER

void PackageManager::LoadRemotePackages() {
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_DEBUG("Loading remote packages");

	const auto& repositories = plugify->GetConfig().repositories;

	_remotePackages.clear();
	_remotePackages.reserve(repositories.size() + _localPackages.size());

	std::mutex mutex;

	auto fetchManifest = [&](const std::string& url, const std::shared_ptr<Descriptor>& descriptor = nullptr) {
		if (!String::IsValidURL(url)) {
			PL_LOG_VERBOSE("Tried to fetch a package: '{}' that is not have valid url: \"{}\", aborting",
						   descriptor ? descriptor->friendlyName : "<from config>", url.empty() ? "<empty>" : url);
			return;
		}
		
		_httpDownloader->CreateRequest(url, [&](int32_t statusCode, std::string_view, IHTTPDownloader::Request::Data data) {
			if (statusCode == IHTTPDownloader::HTTP_STATUS_OK) {
				/*if (contentType != "text/plain" || contentType != "application/json" || contentType != "text/json" || contentType != "text/javascript") {
					PL_LOG_ERROR("Package manifest: '{}' should be in text format to be read correctly", url);
					return;
				}*/

				std::string buffer(data.begin(), data.end());
				auto manifest = glz::read_jsonc<PackageManifest>(buffer);
				if (!manifest.has_value()) {
					PL_LOG_ERROR("Packages manifest from '{}' has JSON parsing error: {}", url, glz::format_error(manifest.error(), buffer));
					return;
				}

				for (auto& [name, package] : manifest->content) {
					if (name.empty() || package->name != name) {
						PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", url, name, package->name);
						continue;
					}
					RemoveUnsupported(package);
					if (package->versions.empty()) {
						PL_LOG_ERROR("Package manifest: '{}' has empty version list at '{}'", url, name);
						continue;
					}

					auto it = _remotePackages.find(name);
					if (it == _remotePackages.end()) {
						std::unique_lock<std::mutex> lock(mutex);
						_remotePackages.emplace(name, std::move(package));
					} else {
						auto& [_, existingPackage] = *it;
						if (existingPackage == package) {
							std::unique_lock<std::mutex> lock(mutex);
							existingPackage->versions.merge(package->versions);
						} else {
							PL_LOG_VERBOSE("The package '{}' exists at '{}' - second location will be ignored.", name, url);
						}
					}
				}
			}
		});
	};

	for (const auto& url : repositories) {
		fetchManifest(url);
	}

	for (const auto& [_, package] : _localPackages) {
		if (const auto& url = package->descriptor->updateURL) {
			fetchManifest(*url, package->descriptor);
		}
	}

	//FetchPackagesListFromAPI(mutex);

	_httpDownloader->WaitForAllRequests();
}

template<typename T>
static T FindLanguageModule(const std::unordered_map<std::string, T, string_hash, std::equal_to<>>& container, const std::string& lang) {
	for (const auto& [_, package] : container) {
		if (package->type == lang) {
			return package;
		}
	}
	return {};
}

void PackageManager::CheckLanguageModuleDependency(const LocalPackagePtr& package, const std::string& lang) {
	if (FindLanguageModule(_localPackages, lang)) {
		return; // Already installed
	}

	if (auto remoteModule = FindLanguageModule(_remotePackages, lang)) {
		_missedPackages.try_emplace(lang, std::pair{ std::move(remoteModule), std::nullopt }); // by default prioritizing latest language modules
	} else {
		PL_LOG_ERROR("Package: '{}' needs language module '{}', but it was not found.", package->name, lang);
		_conflictedPackages.emplace_back(package);
	}
}

void PackageManager::CheckPluginDependency(const LocalPackagePtr& package, const PluginReferenceDescriptor& dependency) {
	if (dependency.optional.value_or(false) || !IsSupportsPlatform(dependency.supportedPlatforms)) {
		return;
	}

	if (auto itl = _localPackages.find(dependency.name); itl != _localPackages.end()) {
		const auto& [_, localPackage] = *itl;
		if (const auto& version = dependency.requestedVersion) {
			if (*version != localPackage->version) {
				PL_LOG_ERROR("Package: '{}' has dependency: '{}' which required (v{}), but (v{}) installed. Conflict cannot be resolved automatically.", package->name, dependency.name, version->to_string(), localPackage->version);
			}
		}
		return;
	}

	if (auto itr = _remotePackages.find(dependency.name); itr != _remotePackages.end()) {
		const auto& [_, remotePackage] = *itr;
		if (const auto& version = dependency.requestedVersion) {
			if (!remotePackage->Version(*version)) {
				PL_LOG_ERROR("Package: '{}' has dependency: '{}' which required (v{}), but version was not found. Problem cannot be resolved automatically.", package->name, dependency.name, version->to_string());
				_conflictedPackages.emplace_back(package);
				return;
			}
		}

		auto it = _missedPackages.find(dependency.name);
		if (it == _missedPackages.end()) {
			_missedPackages.emplace(dependency.name, std::pair{ remotePackage, dependency.requestedVersion }); //-V837
		} else {
			auto& existingVersion = it->second.second;
			if (const auto& version = dependency.requestedVersion) {
				if (*existingVersion != *version) {
					PL_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' dependency, over older version (v{}).", std::max(*existingVersion, *version), dependency.name, std::min(*existingVersion, *version));

					if (*existingVersion < *version) {
						existingVersion = version;
					}
				} else {
					PL_LOG_VERBOSE("The same version (v{}) of dependency '{}' required by '{}' at '{}' - second location will be ignored.", *existingVersion, dependency.name, package->name, package->path.string());
				}
			} else {
				existingVersion = version;
			}
		}
	} else {
		PL_LOG_ERROR("Package: '{}' has dependency: '{}' which could not be found.", package->name, dependency.name);
		_conflictedPackages.emplace_back(package);
	}
}

void PackageManager::FindDependencies() {
	_missedPackages.clear();
	_conflictedPackages.clear();

	for (const auto& [name, package] : _localPackages) {
		if (package->type != "plugin") {
			continue;
		}

		auto descriptor = std::static_pointer_cast<PluginDescriptor>(package->descriptor);

		CheckLanguageModuleDependency(package, descriptor->languageModule.name);

		if (const auto& dependencies = descriptor->dependencies) {
			for (const auto& dependency : *dependencies) {
				CheckPluginDependency(package, dependency);
			}
		}
	}

	for (const auto& [_, dependency] : _missedPackages) {
		const auto& [package, version] = dependency;
		PL_LOG_INFO("Required to install: '{}' [{}] (v{})", package->name, package->type, version.has_value() ? version->to_string() : "[latest]");
	}

	for (const auto& package : _conflictedPackages) {
		PL_LOG_WARNING("Unable to install: '{}' [{}] (v{}) due to unresolved conflicts", package->name, package->type, package->version);
	}
}

template<typename F>
void PackageManager::Request(F&& action, std::string_view function) {
	auto debugStart = DateTime::Now();

	action();

	_httpDownloader->WaitForAllRequests();

	LoadAllPackages();

	PL_LOG_DEBUG("{} processed in {}ms", function, (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

void PackageManager::InstallMissedPackages() {
	Request([&]{
		std::vector<std::string_view> missed;
		for (const auto& [name, dependency] : _missedPackages) {
			const auto& [package, version] = dependency;
			InstallPackage(package, version);
			missed.emplace_back(name);
		}
		if (!missed.empty()) {
			PL_LOG_INFO("Trying install {} missing package(s) to solve dependency issues", plg::join(missed, ", "));
		}
	}, __func__);
}

void PackageManager::UninstallConflictedPackages() {
	Request([&]{
		std::vector<std::string_view> conflicted;
		for (const auto& package : _conflictedPackages) {
			UninstallPackage(package);
			conflicted.emplace_back(package->name);
		}
		if (!conflicted.empty()) {
			PL_LOG_INFO("Trying uninstall {} conflicted package(s) to solve dependency issues", plg::join(conflicted, ", "));
		}
	}, __func__);
}

void PackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) {
	auto debugStart = DateTime::Now();

	std::unordered_map<std::string, RemotePackagePtr> packages;
	packages.reserve(_localPackages.size());

	for (const auto& [name, package] : _localPackages) {
		packages.emplace(name, std::make_shared<RemotePackage>(*package)); //-V837
	}

	if (packages.empty()) {
		PL_LOG_WARNING("Packages was not found!");
		return;
	}

	PackageManifest manifest{ std::move(packages) };
	std::string buffer;
	const auto ec = glz::write_json(manifest, buffer);
	if (ec) {
		PL_LOG_ERROR("Snapshot packages: JSON writing error: {}", glz::format_error(ec));
		return;
	}
	FileSystem::WriteText(manifestFilePath, prettify ? glz::prettify_json(buffer) : buffer);

	PL_LOG_DEBUG("Snapshot '{}' created in {}ms", manifestFilePath.string(), (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

void PackageManager::InstallPackage(std::string_view packageName, std::optional<plg::version> requiredVersion) {
	if (packageName.empty())
		return;

	Request([&] {
		if (auto it = _remotePackages.find(packageName); it != _remotePackages.end()) {
			const auto& [_, remotePackage] = *it;
			InstallPackage(remotePackage, requiredVersion);
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::InstallPackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string, string_hash, std::equal_to<>> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::vector<std::string_view> errors;
		for (const auto& packageName : packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			if (auto it = _remotePackages.find(packageName); it != _remotePackages.end()) {
				const auto& [_, remotePackage] = *it;
				InstallPackage(remotePackage);
			} else {
				errors.emplace_back(packageName);
			}
			unique.insert(packageName);
		}
		if (!errors.empty()) {
			PL_LOG_ERROR("Not found {} packages(s)", plg::join(errors, ", "));
		}
	}, __func__);
}

void PackageManager::InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) {
	if (manifestFilePath.extension() != PackageManifest::kFileExtension) {
		PL_LOG_ERROR("Package manifest: '{}' should be in *{} format", manifestFilePath.string(), PackageManifest::kFileExtension);
		return;
	}

	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	auto path = plugify->GetConfig().baseDir / manifestFilePath;

	PL_LOG_INFO("Read package manifest from '{}'", path.string());

	auto json = FileSystem::ReadText(path);
	auto manifest = glz::read_jsonc<PackageManifest>(json);
	if (!manifest.has_value()) {
		PL_LOG_ERROR("Package manifest: '{}' has JSON parsing error: {}", path.string(), glz::format_error(manifest.error(), json));
		return;
	}

	if (!reinstall) {
		for (const auto& [name, _] : _localPackages) {
			manifest->content.erase(name);
		}
	}

	if (manifest->content.empty()) {
		PL_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
		return;
	}

	Request([&] {
		for (auto& [name, package] : manifest->content) {
			if (name.empty() || package->name != name) {
				PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", path.string(), name, package->name);
				continue;
			}
			RemoveUnsupported(package);
			if (package->versions.empty()) {
				PL_LOG_ERROR("Package manifest: '{}' has empty version list at '{}'", path.string(), name);
				continue;
			}
			InstallPackage(package);
		}
	}, __func__);
}

void PackageManager::InstallAllPackages(const std::string& manifestUrl, bool reinstall) {
	if (!String::IsValidURL(manifestUrl)) {
		PL_LOG_WARNING("Tried to install packages from manifest which is not have valid url: \"{}\", aborting", manifestUrl);
		return;
	}

	PL_LOG_INFO("Read package manifest from '{}'", manifestUrl);

	const char* func = __func__;

	_httpDownloader->CreateRequest(manifestUrl, [&](int32_t statusCode, std::string_view, IHTTPDownloader::Request::Data data) {
		if (statusCode == IHTTPDownloader::HTTP_STATUS_OK) {
			/*if (contentType != "text/plain" || contentType != "application/json" || contentType != "text/json" || contentType != "text/javascript") {
				PL_LOG_ERROR("Package manifest: '{}' should be in text format to be read correctly", manifestUrl);
				return;
			}*/

			std::string buffer(data.begin(), data.end());
			auto manifest = glz::read_jsonc<PackageManifest>(buffer);
			if (!manifest.has_value()) {
				PL_LOG_ERROR("Packages manifest from '{}' has JSON parsing error: {}", manifestUrl, glz::format_error(manifest.error(), buffer));
				return;
			}

			if (!reinstall) {
				for (const auto& [name, _] : _localPackages) {
					manifest->content.erase(name);
				};
			}

			if (manifest->content.empty()) {
				PL_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
				return;
			}

			Request([&] {
				for (auto& [name, package] : manifest->content) {
					if (name.empty() || package->name != name) {
						PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", manifestUrl, name, package->name);
						continue;
					}
					RemoveUnsupported(package);
					if (package->versions.empty()) {
						PL_LOG_ERROR("Package manifest: '{}' has empty version list at '{}'", manifestUrl, name);
						continue;
					}
					InstallPackage(package);
				}
			}, func);
		}
	});

	_httpDownloader->WaitForAllRequests();
}

bool PackageManager::InstallPackage(const RemotePackagePtr& package, std::optional<plg::version> requiredVersion) {
	if (auto it = _localPackages.find(package->name); it != _localPackages.end()) {
		const auto& [_, localPackage] = *it;
		PL_LOG_WARNING("Package: '{}' (v{}) already installed", package->name, localPackage->version);
		return false;
	}

	PackageOpt newVersion;
	if (const auto& version = requiredVersion) {
		newVersion = package->Version(*version);
		if (newVersion) {
			if (!IsSupportsPlatform(newVersion->platforms))
				return false;
		} else {
			PL_LOG_WARNING("Package: '{}' (v{}) has not been found", package->name, *version);
			return false;
		}
	} else {
		newVersion = package->LatestVersion();
		if (newVersion) {
			if (!IsSupportsPlatform(newVersion->platforms))
				return false;
		} else {
			PL_LOG_WARNING("Package: '{}' (v[latest]]) has not been found", package->name);
			return false;
		}
	}

	return DownloadPackage(package, *newVersion);
}

void PackageManager::UpdatePackage(std::string_view packageName, std::optional<plg::version> requiredVersion) {
	if (packageName.empty())
		return;

	Request([&] {
		if (auto it = _localPackages.find(packageName); it != _localPackages.end()) {
			const auto& [_, localPackage] = *it;
			UpdatePackage(localPackage, requiredVersion);
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::UpdatePackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string, string_hash, std::equal_to<>> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::vector<std::string_view> errors;
		for (const auto& packageName : packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			if (auto it = _localPackages.find(packageName); it != _localPackages.end()) {
				const auto& [_, localPackage] = *it;
				UpdatePackage(localPackage);
			} else {
				errors.emplace_back(packageName);
			}
			unique.insert(packageName);
		}
		if (!errors.empty()) {
			PL_LOG_ERROR("Not found {} packages(s)", plg::join(errors, ", "));
		}
	}, __func__);
}

void PackageManager::UpdateAllPackages() {
	Request([&] {
		for (const auto& [_, package] : _localPackages) {
			UpdatePackage(package);
		}
	}, __func__);
}

bool PackageManager::UpdatePackage(const LocalPackagePtr& package, std::optional<plg::version> requiredVersion) {
	auto it = _remotePackages.find(package->name);
	if (it == _remotePackages.end()) {
		PL_LOG_WARNING("Package: '{}' has not been found", package->name);
		return false;
	}
	const auto& [_, newPackage] = *it;

	PackageOpt newVersion;
	if (const auto& version = requiredVersion) {
		newVersion = newPackage->Version(*version);
		if (newVersion) {
			if (!IsSupportsPlatform(newVersion->platforms))
				return false;

			PL_LOG_INFO("Package '{}' (v{}) will be {}, to different version (v{})", package->name, package->version, newVersion->version > package->version ? "upgraded" : newVersion->version == package->version ? "reinstalled" : "downgraded", newVersion->version);
		} else {
			PL_LOG_WARNING("Package: '{}' (v{}) has not been found", package->name, *version);
			return false;
		}
	} else {
		newVersion = newPackage->LatestVersion();
		if (newVersion) {
			if (!IsSupportsPlatform(newVersion->platforms))
				return false;

			if (newVersion->version > package->version) {
				PL_LOG_INFO("Update available, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(package->version, newVersion->version), newPackage->name, std::min(package->version, newVersion->version));
			} else {
				PL_LOG_WARNING("Package: '{}' has no update available", package->name);
				return false;
			}
		} else {
			PL_LOG_WARNING("Package: '{}' (v[latest]) has not been found", package->name);
			return false;
		}
	}

	return DownloadPackage(package, *newVersion);
}

void PackageManager::UninstallPackage(std::string_view packageName) {
	if (packageName.empty())
		return;

	Request([&] {
		if (auto it = _localPackages.find(packageName); it != _localPackages.end()) {
			const auto& [_, localPackage] = *it;
			UninstallPackage(localPackage);
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::UninstallPackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string, string_hash, std::equal_to<>> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::vector<std::string_view> errors;
		for (const auto& packageName : packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			if (auto it = _localPackages.find(packageName); it != _localPackages.end()) {
				const auto& [_, localPackage] = *it;
				UninstallPackage(localPackage);
			} else {
				errors.emplace_back(packageName);
			}
			unique.insert(packageName);
		}
		if (!errors.empty()) {
			PL_LOG_ERROR("Not found {} packages(s)", plg::join(errors, ", "));
		}
	}, __func__);
}

void PackageManager::UninstallAllPackages() {
	Request([&] {
		for (const auto& [_, package] : _localPackages) {
			UninstallPackage(package, false);
		}
		_localPackages.clear();
	}, __func__);
}

bool PackageManager::UninstallPackage(const LocalPackagePtr& package, bool remove) {
	PL_ASSERT(package->path.has_parent_path() && "Package path doesn't contain parent path");
	auto packagePath = package->path.parent_path();
	std::error_code ec = FileSystem::RemoveFolder(packagePath);
	if (!ec) {
		if (remove)
			_localPackages.erase(package->name);
		PL_LOG_INFO("Package: '{}' (v{}) was removed from: '{}'", package->name, package->version, packagePath.string());
		return true;
	}
	return false;
}

bool PackageManager::DownloadPackage(const PackagePtr& package, const PackageVersion& version) const {
	if (!String::IsValidURL(version.download)) {
		PL_LOG_WARNING("Tried to download a package: '{}' that is not have valid url: \"{}\", aborting", package->name, version.download.empty() ? "<empty>" : version.download);
		return false;
	}

	PL_LOG_VERBOSE("Start downloading: '{}'", package->name);

	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_INFO("Downloading: '{}'", version.download);

	_httpDownloader->CreateRequest(version.download, [=, checksum = version.checksum]
		(int32_t statusCode, std::string_view, IHTTPDownloader::Request::Data data) {
		if (statusCode == IHTTPDownloader::HTTP_STATUS_OK) {
			PL_LOG_VERBOSE("Done downloading: '{}'", package->name);

			/*if (contentType != "application/zip") {
				PL_LOG_ERROR("Package: '{}' should be in *.zip format to be extracted correctly", name);
				return;
			}*/

			if (!IsPackageLegit(checksum, data)) {
				PL_LOG_WARNING("Archive hash '{}' does not match expected checksum, aborting", package->name);
				return;
			}

			const auto& [folder, extension] = packageTypes[package->type == "plugin"];

			fs::path finalPath = plugify->GetConfig().baseDir / folder;
			fs::path finalLocation = finalPath / std::format("{}-{}", package->name, DateTime::Get("%Y_%m_%d_%H_%M_%S"));

			std::error_code ec;
			if (!fs::exists(finalLocation, ec) || !fs::is_directory(finalLocation, ec)) {
				if (!fs::create_directories(finalLocation, ec)) {
					PL_LOG_ERROR("Error creating output directory '{}'", finalLocation.string());
				}
			}

			auto error = ExtractPackage(data, finalLocation, extension);
			if (error.empty()) {
				PL_LOG_VERBOSE("Done extracting: '{}'", package->name);
				auto destinationPath = finalPath / package->name;
				ec = FileSystem::MoveFolder(finalLocation, destinationPath);
				if (ec) {
					PL_LOG_ERROR("Package: '{}' could be renamed from '{}' to '{}' - {}", package->name, finalLocation.string(), destinationPath.string(), ec.message());
				} else {
					PL_LOG_VERBOSE("Package: '{}' was renamed successfully from '{}' to '{}'", package->name, finalLocation.string(), destinationPath.string());
				}

			} else {
				PL_LOG_ERROR("Failed extracting: '{}' - {}", package->name, error);
			}
		} else {
			PL_LOG_ERROR("Failed downloading: '{}' - Code: {}", package->name, statusCode);
		}
	});

	return true;
}

std::string PackageManager::ExtractPackage(std::span<const uint8_t> packageData, const fs::path& extractPath, std::string_view descriptorExt) {
	PL_LOG_VERBOSE("Start extracting: '{}' ....", extractPath.string());

	mz_zip_archive zipArchive;
	std::memset(&zipArchive, 0, sizeof(zipArchive));

	defer {
		mz_zip_reader_end(&zipArchive);
	};

	if (!mz_zip_reader_init_mem(&zipArchive, packageData.data(), packageData.size(), 0)) {
		return std::format("Failed initializing zip reader: {}", mz_zip_get_error_string(mz_zip_get_last_error(&zipArchive)));
	}

	//state.total = zipArchive.m_archive_size;
	//state.progress = 0;

	size_t numFiles = mz_zip_reader_get_num_files(&zipArchive);
	std::vector<mz_zip_archive_file_stat> fileStats(numFiles);

	bool foundDescriptor = false;

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat& fileStat = fileStats[i];

		if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat)) {
			return std::format("Failed getting file stat: {} - {}", i, mz_zip_get_error_string(mz_zip_get_last_error(&zipArchive)));
		}

		fs::path filename(fileStat.m_filename);
		if (filename.extension() == descriptorExt) {
			foundDescriptor = true;
		}
	}

	if (!foundDescriptor) {
		return std::format("Package descriptor *{} missing", descriptorExt);
	}

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat& fileStat = fileStats[i];

		std::vector<char> fileData(static_cast<size_t>(fileStat.m_uncomp_size));

		if (!mz_zip_reader_extract_to_mem(&zipArchive, i, fileData.data(), fileData.size(), 0)) {
			return std::format("Failed extracting file: '{}' - {}", fileStat.m_filename, mz_zip_get_error_string(mz_zip_get_last_error(&zipArchive)));
		}

		std::error_code ec;
		fs::path finalPath = extractPath / fileStat.m_filename;

		if (fileStat.m_is_directory) {
			fs::create_directories(finalPath, ec);
		} else {
			fs::create_directories(finalPath.parent_path(), ec);

			std::ofstream outputFile(finalPath, std::ios::binary);
			if (outputFile.is_open()) {
				outputFile.write(fileData.data(), static_cast<std::streamsize>(fileData.size()));
			} else {
				return std::format("Failed creating destination file: '{}'", fileStat.m_filename);
			}

			//state.progress += fileStat.m_comp_size;
			//state.ratio = std::roundf(static_cast<float>(_packageState.progress) / static_cast<float>(_packageState.total) * 100.0f);
		}
	}

	return {};
}

bool PackageManager::IsPackageLegit(std::string_view checksum, std::span<const uint8_t> packageData) {
	if (checksum.empty())
		return true;

	Sha256 sha;
	sha.update(packageData);
	sha.finalize();
	std::string hash = sha.to_string();

	PL_LOG_VERBOSE("Expected checksum: {}", checksum);
	PL_LOG_VERBOSE("Computed checksum: {}", hash);

	return checksum == hash;
}

#else

void PackageManager::InstallPackage(std::string_view /*packageName*/, std::optional<plg::version> /*requiredVersion*/) {}
void PackageManager::InstallPackages(std::span<const std::string> /*packageNames*/) {}
void PackageManager::InstallAllPackages(const fs::path& /*manifestFilePath*/, bool /*reinstall*/) {}
void PackageManager::InstallAllPackages(const std::string& /*manifestUrl*/, bool /*reinstall*/) {}

void PackageManager::UpdatePackage(std::string_view /*packageName*/, std::optional<plg::version> /*requiredVersion*/) {}
void PackageManager::UpdatePackages(std::span<const std::string> /*packageNames*/) {}
void PackageManager::UpdateAllPackages() {}

void PackageManager::UninstallPackage(std::string_view /*packageName*/) {}
void PackageManager::UninstallPackages(std::span<const std::string> /*packageNames*/) {}
void PackageManager::UninstallAllPackages() {}

void PackageManager::SnapshotPackages(const fs::path& /*manifestFilePath*/, bool /*reinstall*/) {}

void PackageManager::InstallMissedPackages() {}
void PackageManager::UninstallConflictedPackages() {}

#endif // PLUGIFY_DOWNLOADER

bool PackageManager::HasMissedPackages() const {
	return !_missedPackages.empty();
}

bool PackageManager::HasConflictedPackages() const {
	return !_conflictedPackages.empty();
}

LocalPackagePtr PackageManager::FindLocalPackage(std::string_view packageName) const {
	if (auto it = _localPackages.find(packageName); it != _localPackages.end())
		return std::get<LocalPackagePtr>(*it);
	return {};
}

RemotePackagePtr PackageManager::FindRemotePackage(std::string_view packageName) const {
	if (auto it = _remotePackages.find(packageName); it != _remotePackages.end())
		return std::get<RemotePackagePtr>(*it);
	return {};
}

std::vector<LocalPackagePtr> PackageManager::GetLocalPackages() const {
	std::vector<LocalPackagePtr> localPackages;
	localPackages.reserve(_localPackages.size());
	for (const auto& [_, package] : _localPackages)  {
		localPackages.emplace_back(package);
	}
	return localPackages;
}

std::vector<RemotePackagePtr> PackageManager::GetRemotePackages() const {
	std::vector<RemotePackagePtr> remotePackages;
	remotePackages.reserve(_remotePackages.size());
	for (const auto& [_, package] : _remotePackages)  {
		remotePackages.emplace_back(package);
	}
	return remotePackages;
}
