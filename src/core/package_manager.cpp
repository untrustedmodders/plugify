#include "package_manager.h"
#include "package_downloader.h"
#include "module.h"
#include "plugin.h"

#include <wizard/wizard.h>
#include <utils/file_system.h>
#include <utils/json.h>
#include <thread>

using namespace wizard;

PackageManager::PackageManager(std::weak_ptr<IWizard> wizard) : IPackageManager(*this), WizardContext(std::move(wizard)) {
	auto debugStart = DateTime::Now();
	//Initialize();
	WZ_LOG_DEBUG("PackageManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

PackageManager::~PackageManager() = default;

void PackageManager::InstallPackages(const fs::path& manifestFilePath, bool reinstall) {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	if (manifestFilePath.extension().string() != PackageManifest::kFileExtension) {
		WZ_LOG_ERROR("Package manifest: '{}' should be in *{} format", manifestFilePath.string(), PackageManifest::kFileExtension);
		return;
	}

	auto path = wizard->GetConfig().baseDir / manifestFilePath;

	WZ_LOG_INFO("Read package manifest from '{}'", path.string());

	auto json = FileSystem::ReadText(path);
	auto manifest = glz::read_json<PackageManifest>(json);
	if (!manifest.has_value()) {
		WZ_LOG_ERROR("Package manifest: '{}' has JSON parsing error: {}", path.string(), glz::format_error(manifest.error(), json));
		return;
	}

	if (!reinstall) {
		FileSystem::ReadDirectory(wizard->GetConfig().baseDir, [&](const fs::path& path, int depth) {
			if (depth != 1)
				return;

			auto extension = path.extension().string();
			bool isModule = extension == Module::kFileExtension;
			if (!isModule && extension != Plugin::kFileExtension)
				return;

			manifest->content.erase(path.filename().replace_extension().string());

		}, 3);
	}

	if (manifest->content.empty()) {
		WZ_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
		return;
	}

	PackageDownloader downloader{ wizard->GetConfig() };

	std::atomic<bool> stopFlag{false};

	std::thread printer([&downloader, &stopFlag](){
		float lastRatio = 0;
		while (!stopFlag) {
			const auto& state = downloader.GetState();
			if (lastRatio != state.ratio && state.state < PackageInstallState::Done) {
				lastRatio = state.ratio;
				WZ_LOG_INFO("{}", state.GetProgress(60));
			}
		}
	});

	printer.detach();

	for (const auto& [name, package] : manifest->content) {
		if (package.name != name) {
			WZ_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", path.string(), name, package.name);
			continue;
		}

		if (auto tempPath = downloader.Download(package)) {
			auto destinationPath = tempPath->parent_path() / name;
			std::error_code ec;
			if (fs::exists(destinationPath, ec) ) {
				if (fs::is_directory(destinationPath, ec))
					fs::remove_all(destinationPath, ec);
			}
			if (!ec)
				fs::rename(*tempPath, destinationPath, ec);

		} else {
			WZ_LOG_ERROR("Package: '{}' has downloading error: {}", name, downloader.GetState().GetError());
		}
	}

	stopFlag = true;
}

template<typename T, bool U>
std::optional<Package> GetPackageFromDescriptor(const fs::path& path, const std::string& name, bool isModule) {
	auto json = T::ReadText(path);
	auto descriptor = glz::read_json<Descriptor>(json);
	if (!descriptor.has_value()) {
		WZ_LOG_ERROR("Package: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
		return {};
	}
	auto& url = U ? descriptor->updateURL : descriptor->downloadURL;
	if (!IsValidURL(url)) {
		WZ_LOG_ERROR("Package: {} at '{}' has invalid {} URL: '{}'", name, path.string(), U ? "update" : "download", url);
		return  {};
	}
	return std::make_optional<Package>(name, std::move(url), descriptor->version, isModule);
}

void PackageManager::UpdatePackages() {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	PackageDownloader downloader{ wizard->GetConfig() };

	std::atomic<bool> stopFlag{false};

	std::thread printer([&downloader, &stopFlag](){
		float lastRatio = 0;
		while (!stopFlag) {
			const auto& state = downloader.GetState();
			if (lastRatio != state.ratio && state.state < PackageInstallState::Done) {
				lastRatio = state.ratio;
				WZ_LOG_INFO("{}", state.GetProgress(60));
			}
		}
	});

	printer.detach();

	FileSystem::ReadDirectory(wizard->GetConfig().baseDir, [&](const fs::path& path, int depth) {
		if (depth != 1)
			return;

		auto extension = path.extension().string();
		bool isModule = extension == Module::kFileExtension;
		if (!isModule && extension != Plugin::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();

		auto package = GetPackageFromDescriptor<FileSystem, true>(path, name, isModule);
		if (!package.has_value())
			return;

		if (auto newPackage = downloader.Update(*package)) {
			if (auto tempPath = downloader.Download(*newPackage)) {
				auto destinationPath = tempPath->parent_path() / package->name;
				std::error_code ec;
				if (fs::exists(destinationPath, ec) ) {
					if (fs::is_directory(destinationPath, ec))
						fs::remove_all(destinationPath, ec);
				}
				if (!ec)
					fs::rename(*tempPath, destinationPath, ec);


			} else {
				WZ_LOG_ERROR("Package: '{}' has downloading error: {}", newPackage->name, downloader.GetState().GetError());
			}
		}

	}, 3);

	stopFlag = true;
}

void PackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	auto debugStart = DateTime::Now();

	std::unordered_map<std::string, Package> packages;

	FileSystem::ReadDirectory(wizard->GetConfig().baseDir, [&](const fs::path& path, int depth) {
		if (depth != 1)
			return;

		auto extension = path.extension().string();
		bool isModule = extension == Module::kFileExtension;
		if (!isModule && extension != Plugin::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();

		auto package = GetPackageFromDescriptor<FileSystem, false>(path, name, isModule);
		if (!package.has_value())
			return;

		auto it = packages.find(name);
		if (it == packages.end()) {
			packages.emplace(std::move(name), std::move(*package));
		} else {
			const auto& existingPackage = std::get<Package>(*it);

			auto& existingVersion = existingPackage.version;
			if (existingVersion != package->version) {
				WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package->version), name, std::min(existingVersion, package->version));

				if (existingVersion < package->version) {
					packages[std::move(name)] = std::move(*package);
				}
			} else {
				WZ_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' - second location will be ignored.", existingVersion, name, path.string());
			}
		}
	}, 3);

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