#include "package_manager.h"
#include "package_downloader.h"
#include "module.h"
#include "plugin.h"

#include <wizard/wizard.h>
#include <utils/file_system.h>
#include <utils/json.h>

using namespace wizard;

PackageManager::PackageManager(std::weak_ptr<IWizard> wizard) : IPackageManager(*this), WizardContext(std::move(wizard)) {
	auto debugStart = DateTime::Now();
	//Initialize();
	WZ_LOG_DEBUG("PackageManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

PackageManager::~PackageManager() = default;

void PackageManager::InstallPackages(const fs::path& manifestFilePath) {
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

	std::vector<const std::string*> packages;

	for (const auto& [name, package] : manifest->content) {
		auto it = _allPackages.find(name);
		if (it == _allPackages.end()) {
			packages.push_back(&name);
		} else {
			const auto& existingPackage = std::get<Package>(*it);

			auto& existingVersion = existingPackage.version;
			if (existingVersion != package.version) {
				WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package.version), name, std::min(existingVersion, package.version));

				if (existingVersion < package.version) {
					packages.push_back(&name);
				}
			} else {
				WZ_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' and '{}' - second location will be ignored.", existingVersion, name, existingPackage.url, path.string());
			}
		}
	}

	for (const std::string* name : packages) {
		_allPackages[*name] = std::move(manifest->content[*name]);
	}

	// TODO: Download packages

	return;
}

template<bool Update>
std::optional<Package> GetPackageFromDescriptor(const fs::path& path, const std::string& name) {
	auto json = FileSystem::ReadText(path);
	auto descriptor = glz::read_json<Descriptor>(json);
	if (!descriptor.has_value()) {
		WZ_LOG_ERROR("Package: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
		return {};
	}
	auto& url = Update ? descriptor->updateURL : descriptor->downloadURL;
	if (!IsValidURL(url)) {
		WZ_LOG_ERROR("Package: {} at '{}' has invalid {} URL: '{}'", name, path.string(), Update ? "update" : "download", url);
		return  {};
	}
	return std::make_optional<Package>(name, url, descriptor->version, true, false);
}

void PackageManager::UpdatePackages() {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	PackageDownloader downloader{ wizard->GetConfig() };

	std::unordered_map<std::string, Package> packages;

	FileSystem::ReadDirectory(wizard->GetConfig().baseDir, [&](const fs::path& path, int depth) {
		// TODO: Add read of zip
		if (depth != 1)
			return;

		auto extension = path.extension().string();
		if (extension != Module::kFileExtension && extension != Plugin::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();

		auto package = GetPackageFromDescriptor<true>(path, name);
		if (!package.has_value())
			return;

		if (auto newPackage = downloader.Update(*package)) {
			if (auto newPath = downloader.Download(*newPackage)) {
				auto baseDir = path.parent_path();

				std::error_code ec;
				if (fs::exists(baseDir, ec) ) {
					if (fs::is_directory(baseDir, ec))
						fs::remove_all(baseDir, ec);
					else if (fs::is_regular_file(baseDir, ec))
						fs::remove(baseDir, ec);
				}

				fs::rename(*newPath, newPath->parent_path() / baseDir.filename(), ec);
			}
		}
	}, 3);
}

void PackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	auto debugStart = DateTime::Now();

	std::unordered_map<std::string, Package> packages;

	FileSystem::ReadDirectory(wizard->GetConfig().baseDir, [&](const fs::path& path, int depth) {
		// TODO: Add read of zip
		if (depth != 1)
			return;

		auto extension = path.extension().string();
		if (extension != Module::kFileExtension && extension != Plugin::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();

		auto package = GetPackageFromDescriptor<false>(path, name);
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

/*std::vector<Package> PackageManager::GetPackagesFromDescriptor(const fs::path& descriptorFilePath, bool isModule, bool isUpdate) {
	// TODO: use template ?
	auto name = descriptorFilePath.filename().replace_extension().string();
	if (isModule) {
		auto json = FileSystem::ReadText(descriptorFilePath);
		auto descriptor = glz::read_json<LanguageModuleDescriptor>(json);
		if (!descriptor.has_value()) {
			WZ_LOG_ERROR("Module descriptor: {} has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
			return {};
		}
		auto& url = isUpdate ? descriptor->updateURL : descriptor->downloadURL;
		if (!IsValidURL(url)) {
			WZ_LOG_ERROR("Module descriptor: {} at '{}' has invalid {} URL: '{}'", name, descriptorFilePath.string(), isUpdate ? "update" : "download", url);
			return  {};
		}
		return { Package{std::move(name), url, descriptor->version, false, true} };
	} else {
		auto json = FileSystem::ReadText(descriptorFilePath);
		auto descriptor = glz::read_json<PluginDescriptor>(json);
		if (!descriptor.has_value()) {
			WZ_LOG_ERROR("Plugin descriptor: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
			return {};
		}
		auto& url = isUpdate ? descriptor->updateURL : descriptor->downloadURL;
		if (!IsValidURL(url)) {
			WZ_LOG_ERROR("Plugin descriptor: {} at '{}' has invalid {} URL: '{}'", name, descriptorFilePath.string(), isUpdate ? "update" : "download", url);
			return  {};
		}
		std::vector<Package> packages;
		packages.reserve(descriptor->dependencies.size() + 1);
		packages.emplace_back(std::move(name), url, descriptor->version, false, false);
		for (const auto& dependency : descriptor->dependencies) {
			if (!IsValidURL(dependency.downloadURL))
				continue;
			packages.emplace_back(dependency.name, dependency.downloadURL, dependency.requestedVersion.value_or(-1), false, false);
		}
		return packages;
	}
}*/