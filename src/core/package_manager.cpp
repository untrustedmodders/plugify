#include "package_manager.h"

#include <wizard/wizard.h>
#include <utils/file_system.h>
#include <utils/json.h>

using namespace wizard;

PackageManager::PackageManager(std::weak_ptr<IWizard> wizard) : WizardContext(std::move(wizard)) {
	auto debugStart = DateTime::Now();
	//Initialize();
	WZ_LOG_DEBUG("PackageManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

PackageManager::~PackageManager() = default;

void PackageManager::LoadPackages() {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	std::vector<fs::path> manifestsFilePaths = FileSystem::GetFiles(wizard->GetConfig().baseDir, true, PackageManifest::kFileExtension);

	for (const auto& path : manifestsFilePaths) {
		WZ_LOG_INFO("Read package manifest from '{}'", path.string());

		auto json = FileSystem::ReadText(path);
		auto manifest = glz::read_json<PackageManifest>(json);
		if (!manifest.has_value()) {
			WZ_LOG_ERROR("Package manifest: '{}' has JSON parsing error: {}", path.string(), glz::format_error(manifest.error(), json));
			continue;
		}

		std::vector<const std::string*> namesToMove;

		for (const auto& [name, package] : manifest->content) {
			auto it = _allPackages.find(name);
			if (it == _allPackages.end()) {
				namesToMove.push_back(&name);
			} else {
				const auto& existingPackage = std::get<Package>(*it);

				auto& existingVersion = existingPackage.version;
				if (existingVersion != package.version) {
					WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package.version), name, std::min(existingVersion, package.version));

					if (existingVersion < package.version) {
						namesToMove.push_back(&name);
					}
				} else {
					WZ_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' and '{}' - second location will be ignored.", existingVersion, name, existingPackage.url, path.string());
				}
			}
		}

		for (const std::string* name : namesToMove) {
			_allPackages[*name] = std::move(manifest->content[*name]);
		}
	}
}