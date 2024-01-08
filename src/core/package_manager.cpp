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
	auto version = descriptor->version;
	return std::make_optional<LocalPackage>(name, std::is_same_v<T, LanguageModuleDescriptor> ? "module" : "plugin", path, version, std::make_unique<Descriptor>(std::move(*descriptor)));
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
			const auto& existingPackage = std::get<LocalPackage>(*it);

			auto& existingVersion = existingPackage.version;
			if (existingVersion != package->version) {
				WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package->version), name, std::min(existingVersion, package->version));

				if (existingVersion < package->version) {
					_localPackages[std::move(name)] = std::move(*package);
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

	for (const auto& url : wizard->GetConfig().repositories) {
		auto repository = _downloader.FetchPackageManifest(url);
		if (!repository.has_value())
			continue;

		for (auto& [name, package] : repository->content) {
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
}

void PackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) {
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

void PackageManager::InstallPackage(const std::string& packageName) {
	DoPackage([&]{
		auto package = FindRemotePackage(packageName);
		if (package.has_value()) {
			InstallPackage(*package);
		}
	});
}

void PackageManager::InstallPackages(std::span<const std::string> packageNames) {
	DoPackage([&]{
		for (const auto& packageName : packageNames) {
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
		};
	}

	if (manifest->content.empty()) {
		WZ_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
		return;
	}

	DoPackage([&]{
		for (const auto& [name, package] : manifest->content) {
			if (package.name != name) {
				WZ_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", path.string(), name, package.name);
				continue;
			}
			InstallPackage(package);
		}
	});
}

void PackageManager::InstallPackage(const RemotePackage& package) {
	if (auto tempPath = _downloader.DownloadPackage(package)) {
		auto destinationPath = tempPath->parent_path() / package.name;
		std::error_code ec = FileSystem::MoveFolder(*tempPath, destinationPath);
		if (ec) {
			WZ_LOG_ERROR("Package: '{}' could be renamed from '{}' to '{}' - {}", package.name, tempPath->string(), destinationPath.string(), ec.message());
		}
	} else {
		WZ_LOG_ERROR("Package: '{}' has downloading error: {}", package.name, _downloader.GetState().GetError());
	}
}

void PackageManager::UpdatePackage(const std::string& packageName) {
	DoPackage([&]{
		auto package = FindLocalPackage(packageName);
		if (package.has_value()) {
			UpdatePackage(*package);
		}
	});
}

void PackageManager::UpdatePackages(std::span<const std::string> packageNames) {
	DoPackage([&]{
		for (const auto& packageName : packageNames) {
			auto package = FindLocalPackage(packageName);
			if (package.has_value()) {
				UpdatePackage(*package);
			}
		}
	});
}

void PackageManager::UpdateAllPackages() {
	DoPackage([&]{
		for (const auto& [_, package] : _localPackages) {
			UpdatePackage(package);
		}
	});
}

void PackageManager::UpdatePackage(const LocalPackage& package) {
	if (auto newPackage = _downloader.UpdatePackage(package)) {
		if (auto tempPath = _downloader.DownloadPackage(*newPackage)) {
			auto destinationPath = tempPath->parent_path() / newPackage->name;
			if (newPackage->name != package.name) {
				FileSystem::RemoveFolder(package.path.parent_path());
			}
			std::error_code ec = FileSystem::MoveFolder(*tempPath, destinationPath);
			if (ec) {
				WZ_LOG_ERROR("Package: '{}' could be renamed from '{}' to '{}' - {}", newPackage->name, tempPath->string(), destinationPath.string(), ec.message());
			}
		} else {
			WZ_LOG_ERROR("Package: '{}' has downloading error: {}", newPackage->name, _downloader.GetState().GetError());
		}
	}
}

void PackageManager::UninstallPackage(const std::string& packageName) {
	DoPackage([&]{
		auto package = FindLocalPackage(packageName);
		if (package.has_value()) {
			UninstallPackage(*package);
		}
	});
}

void PackageManager::UninstallPackages(std::span<const std::string> packageNames) {
	DoPackage([&]{
		for (const auto& packageName : packageNames) {
			auto package = FindLocalPackage(packageName);
			if (package.has_value()) {
				UninstallPackage(*package);
			}
		}
	});
}

void PackageManager::UninstallAllPackages() {
	DoPackage([&]{
		for (const auto& [_, package] : _localPackages) {
			UninstallPackage(package);
		}
	});
}

void PackageManager::UninstallPackage(const LocalPackage& package) {
	// TODO:
}

LocalPackageRef PackageManager::FindLocalPackage(const std::string& packageName) {
	auto it = _localPackages.find(packageName);
	if (it != _localPackages.end())
		return std::get<LocalPackage>(*it);
	return {};
}

RemotePackageRef PackageManager::FindRemotePackage(const std::string& packageName) {
	auto it = _remotePackages.find(packageName);
	if (it != _remotePackages.end())
		return std::get<RemotePackage>(*it);
	return {};
}

void PackageManager::DoPackage(const std::function<void()>& body) {
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

	body();

	stopFlag = true;

	LoadLocalPackages();
}