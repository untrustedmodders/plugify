#include "package_manager.h"
#include "package_downloader.h"
#include "module.h"
#include "plugin.h"

#include <wizard/wizard.h>
#include <utils/file_system.h>
#include <utils/json.h>
#include <thread>

using namespace wizard;

PackageManager::PackageManager(std::weak_ptr<IWizard> wizard) : IPackageManager(*this), WizardContext(std::move(wizard)), _downloader{ _wizard.lock()->GetConfig() } { // TODO: REWORK
	auto debugStart = DateTime::Now();
	LoadLocalPackages();
	LoadRemotePackages();
	ResolveDependencies();
	WZ_LOG_DEBUG("PackageManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

PackageManager::~PackageManager() = default;

template<typename T>
std::optional<LocalPackage> GetPackageFromDescriptor(const fs::path& path, const std::string& name) {
	auto json = FileSystem::ReadText(path);
	auto descriptor = glz::read_json<T>(json);
	if (!descriptor.has_value()) {
		WZ_LOG_ERROR("Package: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
		return {};
	}
	if (!PackageManager::IsSupportsPlatform(descriptor->supportedPlatforms))
		return {};
	std::string type;
	if constexpr (std::is_same_v<T, LanguageModuleDescriptor>) {
		if (descriptor->language == "plugin") {
			WZ_LOG_ERROR("Package: '{}' has JSON parsing error: Forbidden language name", name);
			return {};
		}
		type = descriptor->language;
	} else {
		type = "plugin";
	}
	auto version = descriptor->version;
	return std::make_optional<LocalPackage>(name, type, path, version, std::make_unique<T>(std::move(*descriptor)));
}

void PackageManager::LoadLocalPackages()  {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	WZ_LOG_DEBUG("Loading local packages");

	_localPackages.clear();

	FileSystem::ReadDirectory(wizard->GetConfig().baseDir, [&](const fs::path& path, int depth) {
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

		auto it = _localPackages.find(name);
		if (it == _localPackages.end()) {
			_localPackages.emplace(std::move(name), std::move(*package));
		} else {
			auto& existingPackage = std::get<LocalPackage>(*it);

			auto& existingVersion = existingPackage.version;
			if (existingVersion != package->version) {
				WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package->version), name, std::min(existingVersion, package->version));

				if (existingVersion < package->version) {
					existingPackage = std::move(*package);
				}
			} else {
				WZ_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' - second location will be ignored.", existingVersion, name, path.string());
			}
		}
	}, 3);
}

void PackageManager::LoadRemotePackages() {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	WZ_LOG_DEBUG("Loading remote packages");

	_remotePackages.clear();

	std::unordered_set<std::string> fetched;

	auto fetchManifest = [&](std::string url) {
		if (!url.empty() && url.back() == '/')
			url.pop_back();

		if (fetched.contains(url))
			return;

		auto manifest = _downloader.FetchPackageManifest(url);
		if (manifest.has_value()) {
			for (auto& [name, package] : manifest->content) {
				if (package.name != name) {
					WZ_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", url, name, package.name);
					continue;
				}

				auto it = _remotePackages.find(name);
				if (it == _remotePackages.end()) {
					_remotePackages.emplace(name, package);
				} else {
					auto& existingPackage = std::get<RemotePackage>(*it);
					if (existingPackage == package) {
						existingPackage.versions.merge(package.versions);
					} else {
						WZ_LOG_WARNING("The package '{}' exists at '{}' - second location will be ignored.", name, url);
					}
				}
			}
		}

		fetched.insert(std::move(url));
	};

	for (const auto& url : wizard->GetConfig().repositories) {
		if (!url.empty())
			fetchManifest(url);
	}

	for (const auto& [_, package] : _localPackages) {
		const auto& url = package.descriptor->updateURL;
		if (!url.empty() )
			fetchManifest(url);
	}
}

template<typename K, typename V>
std::optional<std::reference_wrapper<const V>> GetLanguageModule(const std::unordered_map<K, V>& container, const std::string& name)  {
	for (auto& [_, package] : container) {
		if (package.type == name) {
			return package;
		}
	}
	return {};
}

void PackageManager::ResolveDependencies() {
	using Dependency = std::pair<std::reference_wrapper<const RemotePackage>, std::optional<int32_t>>;
	std::unordered_map<std::string, Dependency> dependencies;

	// TODO: Remove localPackages which could not be solve dependency issues

	for (const auto& [_, package] : _localPackages) {
		if (package.type == "plugin") {
			auto pluginDescriptor = std::static_pointer_cast<PluginDescriptor>(package.descriptor);

			const auto& lang = pluginDescriptor->languageModule.name;
			if (!GetLanguageModule(_localPackages, lang)) {
				auto remotePackage = GetLanguageModule(_remotePackages, lang);
				if (remotePackage.has_value()) {
					auto it = dependencies.find(lang);
					if (it == dependencies.end()) {
						dependencies.emplace(lang, std::pair{ *remotePackage, std::nullopt }); // by default prioritizing latest language modules

					}
				} else {
					WZ_LOG_ERROR("Package: '{}' has language module dependency: '{}', but it was not found.", package.name, lang);
					continue;
				}
			}

			for (const auto& dependency : pluginDescriptor->dependencies) {
				if (dependency.optional || !IsSupportsPlatform(dependency.supportedPlatforms))
					continue;

				auto localPackage = FindLocalPackage(dependency.name);
				if (localPackage.has_value()) {
					if (dependency.requestedVersion.has_value() && *dependency.requestedVersion != localPackage->get().version)  {
						WZ_LOG_ERROR("Package: '{}' has dependency: '{}' which required (v{}), but (v{}) installed. Conflict cannot be resolved automatically.", package.name, dependency.name, *dependency.requestedVersion, localPackage->get().version);
					}
					continue;
				}

				auto remotePackage = FindRemotePackage(dependency.name);
				if (remotePackage.has_value()) {
					if (dependency.requestedVersion.has_value() && !remotePackage->get().Version(*dependency.requestedVersion).has_value()) {
						WZ_LOG_ERROR("Package: '{}' has dependency: '{}' which required (v{}), but version was not found. Problem cannot be resolved automatically.", package.name, dependency.name, *dependency.requestedVersion);
						continue;
					}

					auto it = dependencies.find(dependency.name);
					if (it == dependencies.end()) {
						dependencies.emplace(dependency.name, std::pair{ *remotePackage, dependency.requestedVersion });

					} else {
						auto& existingDependency = std::get<Dependency>(*it);

						auto& existingVersion = existingDependency.second;
						if (dependency.requestedVersion.has_value()) {
							if (existingVersion.has_value()) {
								if (*existingVersion != *dependency.requestedVersion) {
									WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' dependency, over older version (v{}).", std::max(*existingVersion, *dependency.requestedVersion), dependency.name, std::min(*existingVersion, *dependency.requestedVersion));

									if (*existingVersion < *dependency.requestedVersion) {
										existingVersion = dependency.requestedVersion;
									}
								} else {
									WZ_LOG_WARNING("The same version (v{}) of dependency '{}' required by '{}' at '{}' - second one will be ignored.", *existingVersion, dependency.name, package.name, package.path.string());
								}
							} else {
								existingVersion = dependency.requestedVersion;
							}
						}
					}
				} else {
					WZ_LOG_ERROR("Package: '{}' has dependency: '{}' which could not be found.", package.name, dependency.name);
				}
			}

		} else /* if (package.type == "module") */ {

		}
	}

	/*Request([&] {
		for (const auto& [_, dependency] : dependencies) {
			const auto& [package, version] = dependency;
			InstallPackage(package, version);
		}
	});*/
}

void PackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) const {
	auto debugStart = DateTime::Now();

	std::unordered_map<std::string, RemotePackage> packages;

	for (const auto& [name, package] : _localPackages) {
		packages.emplace(name, package);
	}

	if (packages.empty()) {
		WZ_LOG_WARNING("Packages was not found!");
		return;
	}

	PackageManifest manifest{ std::move(packages) };
	std::string buffer;
	glz::write_json(manifest, buffer);
	FileSystem::WriteText(manifestFilePath, prettify ? glz::prettify(buffer) : buffer);

	WZ_LOG_DEBUG("Snapshot '{}' created in {}ms", manifestFilePath.string(), (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

void PackageManager::InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion) {
	Request([&] {
		auto package = FindRemotePackage(packageName);
		if (package.has_value()) {
			InstallPackage(*package, requiredVersion);
		}
	});
}

void PackageManager::InstallPackages(std::span<const std::string> packageNames) {
	Request([&] {
		for (const auto& packageName: packageNames) {
			auto package = FindRemotePackage(packageName);
			if (package.has_value()) {
				InstallPackage(*package);
			}
		}
	});
}

void PackageManager::InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) {
	if (manifestFilePath.extension().string() != PackageManifest::kFileExtension) {
		WZ_LOG_ERROR("Package manifest: '{}' should be in *{} format", manifestFilePath.string(), PackageManifest::kFileExtension);
		return;
	}

	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	auto path = wizard->GetConfig().baseDir / manifestFilePath;

	WZ_LOG_INFO("Read package manifest from '{}'", path.string());

	auto json = FileSystem::ReadText(path);
	auto manifest = glz::read_json<PackageManifest>(json);
	if (!manifest.has_value()) {
		WZ_LOG_ERROR("Package manifest: '{}' has JSON parsing error: {}", path.string(), glz::format_error(manifest.error(), json));
		return;
	}

	if (!reinstall) {
		for (const auto& [name, _] : _localPackages) {
			manifest->content.erase(name);
		}
	}

	if (manifest->content.empty()) {
		WZ_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
		return;
	}

	Request([&] {
		for (const auto& [name, package]: manifest->content) {
			if (package.name != name) {
				WZ_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", path.string(), name, package.name);
				continue;
			}
			InstallPackage(package);
		}
	});
}

void PackageManager::InstallAllPackages(const std::string& manifestUrl, bool reinstall) {
	if (manifestUrl.empty())
		return;

	WZ_LOG_INFO("Read package manifest from '{}'", manifestUrl);

	auto manifest = _downloader.FetchPackageManifest(manifestUrl);
	if (!manifest.has_value())
		return;

	if (!reinstall) {
		for (const auto& [name, _] : _localPackages) {
			manifest->content.erase(name);
		};
	}

	if (manifest->content.empty()) {
		WZ_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
		return;
	}

	Request([&] {
		for (const auto& [name, package] : manifest->content) {
			if (package.name != name) {
				WZ_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", manifestUrl, name, package.name);
				continue;
			}
			InstallPackage(package);
		}
	});
}

bool PackageManager::InstallPackage(const RemotePackage& package, std::optional<int32_t> requiredVersion) {
	auto localPackage = FindLocalPackage(package.name);
	if (localPackage.has_value()) {
		WZ_LOG_WARNING("Package: '{}' (v{}) already installed", package.name, localPackage->get().version);
		return false;
	}

	PackageRef newVersion;
	if (requiredVersion.has_value()) {
		newVersion = package.Version(*requiredVersion);
		if (newVersion.has_value()) {
			const auto& version = newVersion->get();
			if (!IsSupportsPlatform(version.platforms))
				return false;
		} else {
			WZ_LOG_WARNING("Package: '{}' (v{}) has not been found", package.name, *requiredVersion);
			return false;
		}
	} else {
		newVersion = package.LatestVersion();
		if (newVersion.has_value()) {
			const auto& version = newVersion->get();
			if (!IsSupportsPlatform(version.platforms))
				return false;
		} else {
			WZ_LOG_WARNING("Package: '{}' (v[latest]]) has not been found", package.name);
			return false;
		}
	}

	if (auto tempPath = _downloader.DownloadPackage(package, *newVersion)) {
		auto destinationPath = tempPath->parent_path() / package.name;
		std::error_code ec = FileSystem::MoveFolder(*tempPath, destinationPath);
		if (ec) {
			WZ_LOG_ERROR("Package: '{}' could be renamed from '{}' to '{}' - {}", package.name, tempPath->string(), destinationPath.string(), ec.message());
		}
		return true;
	} else {
		WZ_LOG_ERROR("Package: '{}' has downloading error: {}", package.name, _downloader.GetState().GetError());
	}
	return false;
}

void PackageManager::UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion) {
	Request([&] {
		auto package = FindLocalPackage(packageName);
		if (package.has_value()) {
			UpdatePackage(*package, requiredVersion);
		}
	});
}

void PackageManager::UpdatePackages(std::span<const std::string> packageNames) {
	Request([&] {
		for (const auto& packageName: packageNames) {
			auto package = FindLocalPackage(packageName);
			if (package.has_value()) {
				UpdatePackage(*package);
			}
		}
	});
}

void PackageManager::UpdateAllPackages() {
	Request([&] {
		for (const auto& [_, package]: _localPackages) {
			UpdatePackage(package);
		}
	});
}

bool PackageManager::UpdatePackage(const LocalPackage& package, std::optional<int32_t> requiredVersion) {
	auto remotePackage = FindRemotePackage(package.name);
	if (!remotePackage.has_value()) {
		WZ_LOG_WARNING("Package: '{}' has not been found", package.name);
		return false;
	}

	const auto& newPackage =  remotePackage->get();
	PackageRef newVersion;
	if (requiredVersion.has_value()) {
		newVersion = newPackage.Version(*requiredVersion);
		if (newVersion.has_value()) {
			const auto& version = newVersion->get();
			if (!IsSupportsPlatform(version.platforms))
				return false;

			WZ_LOG_INFO("Package '{}' (v{}) will be {}, to different version (v{})", package.name, package.version, version.version > package.version ? "upgraded" : version.version == package.version ? "reinstalled" : "downgraded", version.version);
		} else {
			WZ_LOG_WARNING("Package: '{}' (v{}) has not been found", package.name, *requiredVersion);
			return false;
		}
	} else {
		newVersion = newPackage.LatestVersion();
		if (newVersion.has_value()) {
			const auto& version = newVersion->get();
			if (!IsSupportsPlatform(version.platforms))
				return false;

			if (version.version > package.version) {
				WZ_LOG_INFO("Update available, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(package.version, version.version), newPackage.name, std::min(package.version, version.version));
			} else {
				WZ_LOG_WARNING("Package: '{}' has no update available", package.name);
				return false;
			}
		} else {
			WZ_LOG_WARNING("Package: '{}' (v[latest]) has not been found", package.name);
			return false;
		}
	}

	if (auto tempPath = _downloader.DownloadPackage(newPackage, *newVersion)) {
		auto destinationPath = tempPath->parent_path() / newPackage.name;
		if (newPackage.name != package.name) {
			UninstallPackage(package);
		}
		std::error_code ec = FileSystem::MoveFolder(*tempPath, destinationPath);
		if (ec) {
			WZ_LOG_ERROR("Package: '{}' could be renamed from '{}' to '{}' - {}", newPackage.name, tempPath->string(), destinationPath.string(), ec.message());
		}
		return true;
	} else {
		WZ_LOG_ERROR("Package: '{}' has downloading error: {}", newPackage.name, _downloader.GetState().GetError());
	}
	return false;
}

void PackageManager::UninstallPackage(const std::string& packageName) {
	Request([&] {
		auto package = FindLocalPackage(packageName);
		if (package.has_value()) {
			UninstallPackage(*package);
		}
	});
}

void PackageManager::UninstallPackages(std::span<const std::string> packageNames) {
	Request([&] {
		for (const auto& packageName: packageNames) {
			auto package = FindLocalPackage(packageName);
			if (package.has_value()) {
				UninstallPackage(*package);
			}
		}
	});
}

void PackageManager::UninstallAllPackages() {
	Request([&] {
		for (const auto& [_, package]: _localPackages) {
			UninstallPackage(package);
		}
	});
}

bool PackageManager::UninstallPackage(const LocalPackage& package) {
	auto packagePath = package.path.parent_path();
	std::error_code ec = FileSystem::RemoveFolder(packagePath);
	if (!ec) {
		WZ_LOG_ERROR("Package: '{}' (v{}) was removed from: '{}'", package.name, package.version, packagePath.string());
		return true;
	}
	return false;
}

LocalPackageRef PackageManager::FindLocalPackage(const std::string& packageName) const {
	auto it = _localPackages.find(packageName);
	if (it != _localPackages.end())
		return std::get<LocalPackage>(*it);
	return {};
}

RemotePackageRef PackageManager::FindRemotePackage(const std::string& packageName) const {
	auto it = _remotePackages.find(packageName);
	if (it != _remotePackages.end())
		return std::get<RemotePackage>(*it);
	return {};
}

std::vector<std::reference_wrapper<const LocalPackage>> PackageManager::GetLocalPackages() const {
	std::vector<std::reference_wrapper<const LocalPackage>> localPackages;
	localPackages.reserve(_localPackages.size());
	for (const auto& [_, package] : _localPackages)  {
		localPackages.emplace_back(package);
	}
	return localPackages;
}

std::vector<std::reference_wrapper<const RemotePackage>> PackageManager::GetRemotePackages() const {
	std::vector<std::reference_wrapper<const RemotePackage>> remotePackages;
	remotePackages.reserve(remotePackages.size());
	for (const auto& [_, package] : _remotePackages)  {
		remotePackages.emplace_back(package);
	}
	return remotePackages;
}

void PackageManager::Request(const std::function<void()>& action) {
	std::atomic<bool> stopFlag{false};

	std::thread printer([&](){
		float lastRatio = 0;
		while (!stopFlag) {
			const auto& state = _downloader.GetState();
			if (lastRatio != state.ratio && state.state < PackageInstallState::Done) {
				lastRatio = state.ratio;
				WZ_LOG_INFO("{}", state.GetProgress(60));
			}
		}
	});

	printer.detach();

	action();

	stopFlag = true;

	LoadLocalPackages();
	LoadRemotePackages();
	ResolveDependencies();
}