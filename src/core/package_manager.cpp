#include "package_manager.h"
#include "package_manifest.h"
#include "module.h"
#include "plugin.h"

#include <plugify/plugify.h>
#include <utils/file_system.h>
#include <utils/http_downloader.h>
#include <utils/json.h>
#include <SHA256.h>
#include <miniz.h>

using namespace plugify;

static std::array<std::pair<std::string_view, std::string_view>, 2> packageTypes {
	std::pair{ "modules", Module::kFileExtension },
	std::pair{ "plugins", Plugin::kFileExtension },
	// Might add more package types in future
};

PackageManager::PackageManager(std::weak_ptr<IPlugify> plugify) : IPackageManager(*this), PlugifyContext(std::move(plugify)) {
}

PackageManager::~PackageManager() {
	Terminate();
}

bool PackageManager::Initialize() {
	if (IsInitialized())
		return false;

	auto debugStart = DateTime::Now();
	_httpDownloader = HTTPDownloader::Create();
	LoadLocalPackages();
	LoadRemotePackages();
	FindDependencies();
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
	_httpDownloader.reset();
}

bool PackageManager::IsInitialized() {
	return _httpDownloader != nullptr;
}

template<typename Cnt, typename Pr = std::equal_to<typename Cnt::value_type>>
bool RemoveDuplicates(Cnt& cnt, Pr cmp = Pr()) {
	auto size = std::size(cnt);
	Cnt result;
	result.reserve(size);

	std::copy_if(
		std::make_move_iterator(std::begin(cnt)),
		std::make_move_iterator(std::end(cnt)),
		std::back_inserter(result),
		[&](const typename Cnt::value_type& what) {
			return std::find_if(std::begin(result), std::end(result), [&](const typename Cnt::value_type& existing) {
				return cmp(what, existing);
			}) == std::end(result);
		}
	);

	cnt = std::move(result);
	return std::size(cnt) != size;
}

template<typename T>
std::optional<LocalPackage> GetPackageFromDescriptor(const fs::path& path, const std::string& name) {
	auto json = FileSystem::ReadText(path);
	auto descriptor = glz::read_json<T>(json);
	if (!descriptor.has_value()) {
		PL_LOG_ERROR("Package: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
		return {};
	}
	if (!PackageManager::IsSupportsPlatform(descriptor->supportedPlatforms))
		return {};
	std::string type;
	if constexpr (std::is_same_v<T, LanguageModuleDescriptor>) {
		if (descriptor->language.empty() || descriptor->language == "plugin") {
			PL_LOG_ERROR("Module descriptor: '{}' has JSON parsing error: Forbidden language name", name);
			return {};
		}
		type = descriptor->language;
	} else {
		if (descriptor->languageModule.name.empty()) {
			PL_LOG_ERROR("Plugin descriptor: '{}' has JSON parsing error: Missing language name", name);
			return {};
		}
		type = "plugin";

		if (RemoveDuplicates(descriptor->dependencies)) {
			PL_LOG_WARNING("Plugin descriptor: '{}' has multiple dependencies with same name!", name);
		}

		if (RemoveDuplicates(descriptor->exportedMethods)) {
			PL_LOG_WARNING("Plugin descriptor: '{}' has multiple method with same name!", name);
		}
	}
	auto version = descriptor->version;
	return { {Package{name, type}, path, version, std::make_unique<T>(std::move(*descriptor))} };
}

void PackageManager::LoadLocalPackages()  {
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_DEBUG("Loading local packages");

	_localPackages.clear();

	FileSystem::ReadDirectory(plugify->GetConfig().baseDir, [&](const fs::path& path, int depth) {
		if (depth != 1)
			return;

		auto extension = path.extension().string();
		bool isModule = extension == Module::kFileExtension;
		if (!isModule && extension != Plugin::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();
		if (name.empty())
			return;

		auto package = isModule ?
				GetPackageFromDescriptor<LanguageModuleDescriptor>(path, name) :
				GetPackageFromDescriptor<PluginDescriptor>(path, name);
		if (!package.has_value())
			return;

		auto it = std::find_if(_localPackages.begin(), _localPackages.end(), [&name](const auto& plugin) {
			return plugin.name == name;
		});

		if (it == _localPackages.end()) {
			_localPackages.emplace_back(std::move(*package));
		} else {
			auto& existingPackage = *it;

			auto& existingVersion = existingPackage.version;
			if (existingVersion != package->version) {
				PL_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package->version), name, std::min(existingVersion, package->version));

				if (existingVersion < package->version) {
					existingPackage = std::move(*package);
				}
			} else {
				PL_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' - second location will be ignored.", existingVersion, name, path.string());
			}
		}
	}, 3);
}

void PackageManager::LoadRemotePackages() {
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_DEBUG("Loading remote packages");

	_remotePackages.clear();

	std::mutex mutex;

	auto fetchManifest = [&](const std::string& url) {
		if (!HTTPDownloader::IsValidURL(url)) {
			PL_LOG_WARNING("Tried to fetch a package that is not have valid url: \"{}\", aborting", url);
			return;
		}
		
		_httpDownloader->CreateRequest(url, [&](int32_t statusCode, const std::string& contentType, HTTPDownloader::Request::Data data) {
			if (statusCode == HTTPDownloader::HTTP_STATUS_OK) {
				if (contentType != "text/plain" || contentType != "application/json" || contentType != "text/json" || contentType != "text/javascript") {
					PL_LOG_ERROR("Package manifest: '{}' should be in text format to be read correctly", url);
					return;
				}

				std::string buffer(data.begin(), data.end());
				auto manifest = glz::read_json<PackageManifest>(buffer);
				if (!manifest.has_value()) {
					PL_LOG_ERROR("Packages manifest from '{}' has JSON parsing error: {}", url, glz::format_error(manifest.error(), buffer));
					return;
				}

				for (auto& [name, package] : manifest->content) {
					if (name.empty() || package.name != name) {
						PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", url, name, package.name);
						continue;
					}
					if (package.versions.empty()) {
						PL_LOG_ERROR("Package manifest: '{}' has empty version list at '{}'", url, name);
						continue;
					}

					const auto& n = name; // clang fix
					auto it = std::find_if(_remotePackages.begin(), _remotePackages.end(), [&n](const auto& plugin) {
						return plugin.name == n;
					});
					if (it == _remotePackages.end()) {
						std::unique_lock<std::mutex> lock(mutex);
						_remotePackages.emplace_back(std::move(package));
					} else {
						auto& existingPackage = *it;
						if (existingPackage == package) {
							std::unique_lock<std::mutex> lock(mutex);
							existingPackage.versions.merge(package.versions);
						} else {
							PL_LOG_WARNING("The package '{}' exists at '{}' - second location will be ignored.", name, url);
						}
					}
				}
			}
		});
	};

	for (const auto& url : plugify->GetConfig().repositories) {
		fetchManifest(url);
	}

	for (const auto& package : _localPackages) {
		fetchManifest(package.descriptor->updateURL);
	}

	//FetchPackagesListFromAPI(mutex);

	_httpDownloader->WaitForAllRequests();
}

template<typename T>
std::optional<std::reference_wrapper<const T>> FindLanguageModule(const std::vector<T>& container, const std::string& name)  {
	for (const auto& package : container) {
		if (package.type == name) {
			return package;
		}
	}
	return {};
}

void PackageManager::FindDependencies() {
	_missedPackages.clear();
	_conflictedPackages.clear();

	for (const auto& package : _localPackages) {
		if (package.type == "plugin") {
			auto pluginDescriptor = std::static_pointer_cast<PluginDescriptor>(package.descriptor);

			const auto& lang = pluginDescriptor->languageModule.name;
			if (!FindLanguageModule(_localPackages, lang)) {
				auto remotePackage = FindLanguageModule(_remotePackages, lang);
				if (remotePackage.has_value()) {
					auto it = _missedPackages.find(lang);
					if (it == _missedPackages.end()) {
						_missedPackages.emplace(lang, std::pair{*remotePackage, std::nullopt }); // by default prioritizing latest language modules
					}
				} else {
					PL_LOG_ERROR("Package: '{}' has language module dependency: '{}', but it was not found.", package.name, lang);
					_conflictedPackages.emplace_back(package);
					continue;
				}
			}

			for (const auto& dependency : pluginDescriptor->dependencies) {
				if (dependency.optional || !IsSupportsPlatform(dependency.supportedPlatforms))
					continue;

				auto localPackage = FindLocalPackage(dependency.name);
				if (localPackage.has_value()) {
					if (dependency.requestedVersion.has_value() && *dependency.requestedVersion != localPackage->get().version)  {
						PL_LOG_ERROR("Package: '{}' has dependency: '{}' which required (v{}), but (v{}) installed. Conflict cannot be resolved automatically.", package.name, dependency.name, *dependency.requestedVersion, localPackage->get().version);
					}
					continue;
				}

				auto remotePackage = FindRemotePackage(dependency.name);
				if (remotePackage.has_value()) {
					if (dependency.requestedVersion.has_value() && !remotePackage->get().Version(*dependency.requestedVersion).has_value()) {
						PL_LOG_ERROR("Package: '{}' has dependency: '{}' which required (v{}), but version was not found. Problem cannot be resolved automatically.", package.name, dependency.name, *dependency.requestedVersion);
						_conflictedPackages.emplace_back(package);
						continue;
					}

					auto it = _missedPackages.find(dependency.name);
					if (it == _missedPackages.end()) {
						_missedPackages.emplace(dependency.name, std::pair{*remotePackage, dependency.requestedVersion });
					} else {
						auto& existingDependency = std::get<Dependency>(*it);

						auto& existingVersion = existingDependency.second;
						if (dependency.requestedVersion.has_value()) {
							if (existingVersion.has_value()) {
								if (*existingVersion != *dependency.requestedVersion) {
									PL_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' dependency, over older version (v{}).", std::max(*existingVersion, *dependency.requestedVersion), dependency.name, std::min(*existingVersion, *dependency.requestedVersion));

									if (*existingVersion < *dependency.requestedVersion) {
										existingVersion = dependency.requestedVersion;
									}
								} else {
									PL_LOG_WARNING("The same version (v{}) of dependency '{}' required by '{}' at '{}' - second one will be ignored.", *existingVersion, dependency.name, package.name, package.path.string());
								}
							} else {
								existingVersion = dependency.requestedVersion;
							}
						}
					}
				} else {
					PL_LOG_ERROR("Package: '{}' has dependency: '{}' which could not be found.", package.name, dependency.name);
					_conflictedPackages.emplace_back(package);
				}
			}
		}
	}

	for (const auto& [_, dependency] : _missedPackages) {
		const auto& [package, version] = dependency;
		PL_LOG_INFO("Required to install: '{}' [{}] (v{})", package.get().name, package.get().type, version.has_value() ? std::to_string(*version) : "[latest]");
	}

	for (const auto& packageRef : _conflictedPackages) {
		const auto& package = packageRef.get();
		PL_LOG_WARNING("Unable to install: '{}' [{}] (v{}) due to unresolved conflicts", package.name, package.type, package.version);
	}
}

void PackageManager::InstallMissedPackages() {
	Request([&]{
		std::string missed;
		bool first = true;
		for (const auto& [name, dependency] : _missedPackages) {
			const auto& [package, version] = dependency;
			InstallPackage(package, version);
			if (first) {
				std::format_to(std::back_inserter(missed), "'{}", name);
				first = false;
			} else {
				std::format_to(std::back_inserter(missed), "', '{}", name);
			}
		}
		if (!first) {
			std::format_to(std::back_inserter(missed), "'");
			PL_LOG_INFO("Trying install {} missing package(s) to solve dependency issues", missed);
		}
	}, __func__);
}

void PackageManager::UninstallConflictedPackages() {
	Request([&]{
		std::string conflicted;
		bool first = true;
		for (const auto& package : _conflictedPackages) {
			UninstallPackage(package);
			if (first) {
				std::format_to(std::back_inserter(conflicted), "'{}", package.get().name);
				first = false;
			} else {
				std::format_to(std::back_inserter(conflicted), "', '{}", package.get().name);
			}
		}
		if (!first) {
			std::format_to(std::back_inserter(conflicted), "'");
			PL_LOG_INFO("Trying uninstall {} conflicted package(s) to solve dependency issues", conflicted);
		}
	}, __func__);
}

void PackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) {
	auto debugStart = DateTime::Now();

	std::unordered_map<std::string, RemotePackage> packages;
	packages.reserve(_localPackages.size());

	for (const auto& package : _localPackages) {
		packages.emplace(package.name, package);
	}

	if (packages.empty()) {
		PL_LOG_WARNING("Packages was not found!");
		return;
	}

	PackageManifest manifest{ std::move(packages) };
	std::string buffer;
	glz::write_json(manifest, buffer);
	FileSystem::WriteText(manifestFilePath, prettify ? glz::prettify(buffer) : buffer);

	PL_LOG_DEBUG("Snapshot '{}' created in {}ms", manifestFilePath.string(), (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

void PackageManager::InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion) {
	if (packageName.empty())
		return;

	Request([&] {
		auto package = FindRemotePackage(packageName);
		if (package.has_value()) {
			InstallPackage(*package, requiredVersion);
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::InstallPackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::string error;
		bool first = true;
		for (const auto& packageName: packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			auto package = FindRemotePackage(packageName);
			if (package.has_value()) {
				InstallPackage(*package);
			} else {
				if (first) {
					std::format_to(std::back_inserter(error), "'{}", packageName);
					first = false;
				} else {
					std::format_to(std::back_inserter(error), "', '{}", packageName);
				}
			}
			unique.insert(packageName);
		}
		if (!first) {
			std::format_to(std::back_inserter(error), "'");
			PL_LOG_ERROR("Not found {} packages(s)", error);
		}
	}, __func__);
}

void PackageManager::InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) {
	if (manifestFilePath.extension().string() != PackageManifest::kFileExtension) {
		PL_LOG_ERROR("Package manifest: '{}' should be in *{} format", manifestFilePath.string(), PackageManifest::kFileExtension);
		return;
	}

	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	auto path = plugify->GetConfig().baseDir / manifestFilePath;

	PL_LOG_INFO("Read package manifest from '{}'", path.string());

	auto json = FileSystem::ReadText(path);
	auto manifest = glz::read_json<PackageManifest>(json);
	if (!manifest.has_value()) {
		PL_LOG_ERROR("Package manifest: '{}' has JSON parsing error: {}", path.string(), glz::format_error(manifest.error(), json));
		return;
	}

	if (!reinstall) {
		for (const auto& package : _localPackages) {
			manifest->content.erase(package.name);
		}
	}

	if (manifest->content.empty()) {
		PL_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
		return;
	}

	Request([&] {
		for (const auto& [name, package]: manifest->content) {
			if (name.empty() || package.name != name) {
				PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", path.string(), name, package.name);
				continue;
			}
			if (package.versions.empty()) {
				PL_LOG_ERROR("Package manifest: '{}' has empty version list at '{}'", path.string(), name);
				continue;
			}
			InstallPackage(package);
		}
	}, __func__);
}

void PackageManager::InstallAllPackages(const std::string& manifestUrl, bool reinstall) {
	if (!HTTPDownloader::IsValidURL(manifestUrl)) {
		PL_LOG_WARNING("Tried to install packages from manifest which is not have valid url: \"{}\", aborting", manifestUrl);
		return;
	}

	PL_LOG_INFO("Read package manifest from '{}'", manifestUrl);

	const char* func = __func__;

	_httpDownloader->CreateRequest(manifestUrl, [&](int32_t statusCode, const std::string& contentType, HTTPDownloader::Request::Data data) {
		if (statusCode == HTTPDownloader::HTTP_STATUS_OK) {
			if (contentType != "text/plain" || contentType != "application/json" || contentType != "text/json" || contentType != "text/javascript") {
				PL_LOG_ERROR("Package manifest: '{}' should be in text format to be read correctly", manifestUrl);
				return;
			}

			std::string buffer(data.begin(), data.end());
			auto manifest = glz::read_json<PackageManifest>(buffer);
			if (!manifest.has_value()) {
				PL_LOG_ERROR("Packages manifest from '{}' has JSON parsing error: {}", manifestUrl, glz::format_error(manifest.error(), buffer));
				return;
			}

			if (!reinstall) {
				for (const auto& package : _localPackages) {
					manifest->content.erase(package.name);
				};
			}

			if (manifest->content.empty()) {
				PL_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
				return;
			}

			Request([&] {
				for (const auto& [name, package] : manifest->content) {
					if (name.empty() || package.name != name) {
						PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", manifestUrl, name, package.name);
						continue;
					}
					if (package.versions.empty()) {
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

bool PackageManager::InstallPackage(const RemotePackage& package, std::optional<int32_t> requiredVersion) {
	auto localPackage = FindLocalPackage(package.name);
	if (localPackage.has_value()) {
		PL_LOG_WARNING("Package: '{}' (v{}) already installed", package.name, localPackage->get().version);
		return false;
	}

	PackageOpt newVersion;
	if (requiredVersion.has_value()) {
		newVersion = package.Version(*requiredVersion);
		if (newVersion.has_value()) {
			const auto& version = newVersion->get();
			if (!IsSupportsPlatform(version.platforms))
				return false;
		} else {
			PL_LOG_WARNING("Package: '{}' (v{}) has not been found", package.name, *requiredVersion);
			return false;
		}
	} else {
		newVersion = package.LatestVersion();
		if (newVersion.has_value()) {
			const auto& version = newVersion->get();
			if (!IsSupportsPlatform(version.platforms))
				return false;
		} else {
			PL_LOG_WARNING("Package: '{}' (v[latest]]) has not been found", package.name);
			return false;
		}
	}

	return DownloadPackage(package, newVersion->get());
}

void PackageManager::UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion) {
	if (packageName.empty())
		return;

	Request([&] {
		auto package = FindLocalPackage(packageName);
		if (package.has_value()) {
			UpdatePackage(*package, requiredVersion);
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::UpdatePackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::string error;
		bool first = true;
		for (const auto& packageName: packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			auto package = FindLocalPackage(packageName);
			if (package.has_value()) {
				UpdatePackage(*package);
			} else {
				if (first) {
					std::format_to(std::back_inserter(error), "'{}", packageName);
					first = false;
				} else {
					std::format_to(std::back_inserter(error), "', '{}", packageName);
				}
			}
			unique.insert(packageName);
		}
		if (!first) {
			std::format_to(std::back_inserter(error), "'");
			PL_LOG_ERROR("Not found {} packages(s)", error);
		}
	}, __func__);
}

void PackageManager::UpdateAllPackages() {
	Request([&] {
		for (const auto& package : _localPackages) {
			UpdatePackage(package);
		}
	}, __func__);
}

bool PackageManager::UpdatePackage(const LocalPackage& package, std::optional<int32_t> requiredVersion) {
	auto remotePackage = FindRemotePackage(package.name);
	if (!remotePackage.has_value()) {
		PL_LOG_WARNING("Package: '{}' has not been found", package.name);
		return false;
	}

	const auto& newPackage =  remotePackage->get();
	PackageOpt newVersion;
	if (requiredVersion.has_value()) {
		newVersion = newPackage.Version(*requiredVersion);
		if (newVersion.has_value()) {
			const auto& version = newVersion->get();
			if (!IsSupportsPlatform(version.platforms))
				return false;

			PL_LOG_INFO("Package '{}' (v{}) will be {}, to different version (v{})", package.name, package.version, version.version > package.version ? "upgraded" : version.version == package.version ? "reinstalled" : "downgraded", version.version);
		} else {
			PL_LOG_WARNING("Package: '{}' (v{}) has not been found", package.name, *requiredVersion);
			return false;
		}
	} else {
		newVersion = newPackage.LatestVersion();
		if (newVersion.has_value()) {
			const auto& version = newVersion->get();
			if (!IsSupportsPlatform(version.platforms))
				return false;

			if (version.version > package.version) {
				PL_LOG_INFO("Update available, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(package.version, version.version), newPackage.name, std::min(package.version, version.version));
			} else {
				PL_LOG_WARNING("Package: '{}' has no update available", package.name);
				return false;
			}
		} else {
			PL_LOG_WARNING("Package: '{}' (v[latest]) has not been found", package.name);
			return false;
		}
	}

	return DownloadPackage(package, newVersion->get());
}

void PackageManager::UninstallPackage(const std::string& packageName) {
	if (packageName.empty())
		return;

	Request([&] {
		auto package = FindLocalPackage(packageName);
		if (package.has_value()) {
			UninstallPackage(*package);
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::UninstallPackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::string error;
		bool first = true;
		for (const auto& packageName: packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			auto package = FindLocalPackage(packageName);
			if (package.has_value()) {
				UninstallPackage(*package);
			} else {
				if (first) {
					std::format_to(std::back_inserter(error), "'{}", packageName);
					first = false;
				} else {
					std::format_to(std::back_inserter(error), "', '{}", packageName);
				}
			}
			unique.insert(packageName);
		}
		if (!first) {
			std::format_to(std::back_inserter(error), "'");
			PL_LOG_ERROR("Not found {} packages(s)", error);
		}
	}, __func__);
}

void PackageManager::UninstallAllPackages() {
	Request([&] {
		for (const auto& package : _localPackages) {
			UninstallPackage(package, false);
		}
		_localPackages.clear();
	}, __func__);
}

bool PackageManager::UninstallPackage(const LocalPackage& package, bool remove) {
	PL_ASSERT(package.path.has_parent_path(), "Package path doesn't contain parent path");
	auto packagePath = package.path.parent_path();
	std::error_code ec = FileSystem::RemoveFolder(packagePath);
	if (!ec) {
		if (remove)
			_localPackages.erase(std::find(_localPackages.begin(), _localPackages.end(), package));
		PL_LOG_ERROR("Package: '{}' (v{}) was removed from: '{}'", package.name, package.version, packagePath.string());
		return true;
	}
	return false;
}

LocalPackageOpt PackageManager::FindLocalPackage(const std::string& packageName) {
	auto it = std::find_if(_localPackages.begin(), _localPackages.end(), [&packageName](const auto& plugin) {
		return plugin.name == packageName;
	});
	if (it != _localPackages.end())
		return *it;
	return {};
}

RemotePackageOpt PackageManager::FindRemotePackage(const std::string& packageName) {
	auto it = std::find_if(_remotePackages.begin(), _remotePackages.end(), [&packageName](const auto& plugin) {
		return plugin.name == packageName;
	});
	if (it != _remotePackages.end())
		return *it;
	return {};
}

std::vector<LocalPackageRef> PackageManager::GetLocalPackages() {
	std::vector<LocalPackageRef> localPackages;
	localPackages.reserve(_localPackages.size());
	for (const auto& package : _localPackages)  {
		localPackages.emplace_back(package);
	}
	return localPackages;
}

std::vector<RemotePackageRef> PackageManager::GetRemotePackages() {
	std::vector<RemotePackageRef> remotePackages;
	remotePackages.reserve(remotePackages.size());
	for (const auto& package : _remotePackages)  {
		remotePackages.emplace_back(package);
	}
	return remotePackages;
}

void PackageManager::Request(const std::function<void()>& action, std::string_view function) {
	auto debugStart = DateTime::Now();

	action();

	_httpDownloader->WaitForAllRequests();

	LoadLocalPackages();
	LoadRemotePackages();
	FindDependencies();

	PL_LOG_DEBUG("{} processed in {}ms", function, (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

bool PackageManager::Reload() {
	if (!IsInitialized())
		return false;
	
	LoadLocalPackages();
	LoadRemotePackages();
	FindDependencies();
	
	return true;
}

bool PackageManager::DownloadPackage(const Package& package, const PackageVersion& version) const {
	if (!HTTPDownloader::IsValidURL(version.download)) {
		PL_LOG_WARNING("Tried to download a package that is not have valid url: \"{}\", aborting", version.download);
		return false;
	}

	PL_LOG_VERBOSE("Start downloading: '{}'", package.name);

	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_INFO("Downloading: '{}'", version.download);

	_httpDownloader->CreateRequest(version.download, [&name = package.name, plugin = (package.type == "plugin"), &baseDir = plugify->GetConfig().baseDir, &checksum = version.checksum] // should be safe to pass ref
		(int32_t statusCode, const std::string& contentType, HTTPDownloader::Request::Data data) {
		if (statusCode == HTTPDownloader::HTTP_STATUS_OK) {
			PL_LOG_VERBOSE("Done downloading: '{}'", name);

			if (contentType != "application/zip") {
				PL_LOG_ERROR("Package: '{}' should be in *.zip format to be extracted correctly", name);
				return;
			}

			if (!IsPackageLegit(checksum, data)) {
				PL_LOG_WARNING("Archive hash does not match expected checksum, aborting");
				return;
			}

			const auto& [folder, extension] = packageTypes[plugin];

			fs::path finalPath = baseDir / folder;
			fs::path finalLocation = finalPath / std::format("{}-{}", name, DateTime::Get("%Y_%m_%d_%H_%M_%S"));

			std::error_code ec;
			if (!fs::exists(finalLocation, ec) || !fs::is_directory(finalLocation, ec)) {
				if (!fs::create_directories(finalLocation, ec)) {
					PL_LOG_ERROR("Error creating output directory '{}'", finalLocation.string());
				}
			}

			auto error = ExtractPackage(data, finalLocation, extension);
			if (error.empty()) {
				PL_LOG_VERBOSE("Done extracting: '{}'", name);
				auto destinationPath = finalPath / name;
				ec = FileSystem::MoveFolder(finalLocation, destinationPath);
				if (ec) {
					PL_LOG_ERROR("Package: '{}' could be renamed from '{}' to '{}' - {}", name, finalLocation.string(), destinationPath.string(), ec.message());
				} else {

				}

			} else {
				PL_LOG_ERROR("Failed extracting: '{}' - {}", name, error);
			}
		} else {
			PL_LOG_ERROR("Failed downloading: '{}' - Code: {}", name, statusCode);
		}
	});

	return true;
}

std::string PackageManager::ExtractPackage(std::span<const uint8_t> packageData, const fs::path& extractPath, std::string_view descriptorExt) {
	PL_LOG_VERBOSE("Start extracting....");

	auto zipClose = [](mz_zip_archive* zipArchive){ mz_zip_reader_end(zipArchive); delete zipArchive; };
	std::unique_ptr<mz_zip_archive, decltype(zipClose)> zipArchive(new mz_zip_archive, zipClose);
	std::memset(zipArchive.get(), 0, sizeof(mz_zip_archive));

	mz_zip_reader_init_mem(zipArchive.get(), packageData.data(), packageData.size(), 0);

	//state.total = zipArchive->m_archive_size;
	//state.progress = 0;

	size_t numFiles = mz_zip_reader_get_num_files(zipArchive.get());
	std::vector<mz_zip_archive_file_stat> fileStats(numFiles);

	bool foundDescriptor = false;

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat& fileStat = fileStats[i];

		if (!mz_zip_reader_file_stat(zipArchive.get(), i, &fileStat)) {
			return std::format("Error getting file stat: {}", i);
		}

		fs::path filename(fileStat.m_filename);
		if (filename.extension().string() == descriptorExt) {
			foundDescriptor = true;
		}
	}

	if (!foundDescriptor) {
		return std::format("Package descriptor *{} missing", descriptorExt);
	}

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat& fileStat = fileStats[i];

		std::vector<char> fileData(fileStat.m_uncomp_size);

		if (!mz_zip_reader_extract_to_mem(zipArchive.get(), i, fileData.data(), fileData.size(), 0)) {
			return std::format("Failed extracting file: '{}'", fileStat.m_filename);
		}

		fs::path finalPath = extractPath / fileStat.m_filename;
		fs::path finalDir = finalPath.parent_path();

		std::error_code ec;
		if (!fs::exists(finalDir, ec) || !fs::is_directory(finalDir, ec)) {
			if (!fs::create_directories(finalDir, ec)) {
				return std::format("Error creating output directory '{}'", finalDir.string());
			}
		}

		std::ofstream outputFile(finalPath, std::ios::binary);
		if (outputFile.is_open()) {
			outputFile.write(fileData.data(), static_cast<std::streamsize>(fileData.size()));
		} else {
			return std::format("Failed creating destination file: '{}'", fileStat.m_filename);
		}

		//state.progress += fileStat.m_comp_size;
		//state.ratio = std::roundf(static_cast<float>(_packageState.progress) / static_cast<float>(_packageState.total) * 100.0f);
	}

	return {};
}

bool PackageManager::IsPackageLegit(const std::string& checksum, std::span<const uint8_t> packageData) {
	if (checksum.empty())
		return true;

	SHA256 sha;
	sha.update(packageData.data(), packageData.size());
	std::string hash(SHA256::toString(sha.digest()));

	PL_LOG_VERBOSE("Expected checksum: {}", checksum);
	PL_LOG_VERBOSE("Computed checksum: {}", hash);

	return checksum == hash;
}
