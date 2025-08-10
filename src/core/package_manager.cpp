#include "package_manager.hpp"

using namespace plugify;

namespace {
	/**
	 * @brief Filter packages by type
	 */
	std::vector<Package> FilterByType(
		const std::vector<Package>& packages, 
		std::optional<PackageType> type) {
		if (!type) {
			return packages;
		}
        
		auto filtered = packages | std::views::filter([&type](const Package& pkg) {
			return pkg.info.type == *type;
		});

		return std::vector<Package>(filtered.begin(), filtered.end());
	}

	/**
	 * @brief Check if a package is installed
	 */
	bool IsPackageInstalled(const Package& package) {
		return package.installed && package.installPath;
	}

	/**
	 * @brief Compare versions for finding latest
	 */
	bool IsNewerVersion(const plg::version& v1, const plg::version& v2) {
		return v1 > v2;
	}
}

PackageManager::PackageManager(
    PackageManagerConfig config,
    std::shared_ptr<IHTTPDownloader> downloader,
    std::unique_ptr<IPackageScanner> scanner)
    : _config(std::move(config))
    , _downloader(std::move(downloader))
    , _packageScanner(std::move(scanner))
    , _conflictResolver(std::make_unique<ConflictResolver>())
    , _dependencyResolver(std::make_unique<DependencyResolver>()) {

    // Create directories if they don't exist
    if (!fs::exists(_config.installDirectory)) {
        fs::create_directories(_config.installDirectory);
    }
    if (!fs::exists(_config.cacheDirectory)) {
        fs::create_directories(_config.cacheDirectory);
    }

    // Create default scanner if not provided
    if (!_packageScanner) {
        PackageScannerConfig scanConfig;
        scanConfig.searchPaths = {_config.installDirectory};
        scanConfig.manifestFileNames = {"package.json", "manifest.json", "plugin.json"};
        _packageScanner = std::make_unique<PackageScanner>(scanConfig);
    }

    // Initial scan for installed packages
    RescanInstalledPackages();
}

// ----------------------------------------------------------------------------
// Repository Management
// ----------------------------------------------------------------------------

Result<void> PackageManager::AddRepository(std::shared_ptr<IPackageRepository> repository) {
    if (!repository) {
        return Error::Unknown("Repository is null");
    }

    // Check if repository with same name already exists
    auto it = std::ranges::find_if(_repositories, [&repository](const auto& repo) {
        return repo->GetName() == repository->GetName();
    });

    if (it != _repositories.end()) {
        return Error::AlreadyInstalled(repository->GetName());
    }

    _repositories.push_back(std::move(repository));
    InvalidateCache();

    return {};
}

Result<void> PackageManager::RemoveRepository(std::string_view name) {
    auto it = std::ranges::find_if(_repositories, [name](const auto& repo) {
        return repo->GetName() == name;
    });

    if (it == _repositories.end()) {
        return Error::PackageNotFound(name);
    }

    _repositories.erase(it);
    InvalidateCache();

    return {};
}

Result<void> PackageManager::UpdateRepositories(ProgressCallback progress) {
    size_t totalRepos = _repositories.size();
    size_t currentRepo = 0;

    for (auto& repository : _repositories) {
        if (progress) {
            float progressValue = static_cast<float>(currentRepo) / totalRepos;
            progress(std::format("Updating repository: {}", repository->GetName()), progressValue);
        }

        // Check if repository is available
        auto availableResult = repository->IsAvailable();
        if (!availableResult || *!availableResult) {
            // Log warning but continue with other repositories
            continue;
        }

        // Force refresh by enumerating packages (this will update internal caches)
        auto packagesResult = repository->EnumeratePackages();
        if (!packagesResult) {
            // Log error but continue
            continue;
        }

        currentRepo++;
    }

    InvalidateCache();

    if (progress) {
        progress("Repository update complete", 1.0f);
    }

    return {};
}

// ----------------------------------------------------------------------------
// Package Discovery
// ----------------------------------------------------------------------------

Result<std::vector<Package>> PackageManager::ListAvailable(std::optional<PackageType> type) {
    // Use cache if available
    if (_availablePackagesCache) {
        return FilterByType(*_availablePackagesCache, type);
    }

    std::vector<Package> allPackages;

    // Collect packages from all repositories
    for (const auto& repository : _repositories) {
        auto result = repository->EnumeratePackages();
        if (result) {
            allPackages.insert(allPackages.end(),
                             result->begin(),
                             result->end());
        }
    }

    // Remove duplicates (keep latest version)
    std::unordered_map<PackageId, Package> uniquePackages;
    for (auto& package : allPackages) {
        auto it = uniquePackages.find(package.info.id);
        if (it == uniquePackages.end() ||
            IsNewerVersion(package.info.version, it->second.info.version)) {
            uniquePackages[package.info.id] = std::move(package);
        }
    }

    // Convert map to vector
    std::vector<Package> resultPackages;
    resultPackages.reserve(uniquePackages.size());
    for (auto& [id, package] : uniquePackages) {
        resultPackages.push_back(std::move(package));
    }

    // Cache the results
    _availablePackagesCache = resultPackages;

    return FilterByType(resultPackages, type);
}

Result<std::vector<Package>> PackageManager::ListInstalled(std::optional<PackageType> type) {
    // Use cache if available
    if (_installedPackagesCache) {
        return FilterByType(*_installedPackagesCache, type);
    }

    // Scan for installed packages
    auto scanResult = _packageScanner->ScanForPackages();
    if (!scanResult) {
        return scanResult.error();
    }

    std::vector<Package> installedPackages;
    for (const auto& scannedPkg : *scanResult) {
        if (!scannedPkg.error) {
            Package pkg = scannedPkg.package;
            pkg.installed = true;
            pkg.installPath = scannedPkg.manifestPath.parent_path();
            installedPackages.push_back(std::move(pkg));
        }
    }

    // Cache the results
    _installedPackagesCache = installedPackages;

    return FilterByType(installedPackages, type);
}

Result<std::vector<Package>> PackageManager::Search(std::string_view query) {
    std::vector<Package> searchResults;

    // Search in all repositories
    for (const auto& repository : _repositories) {
        auto result = repository->SearchPackages(query);
        if (result) {
            searchResults.insert(searchResults.end(),
                               result->begin(),
                               result->end());
        }
    }

    // Also search in installed packages
    auto installedResult = ListInstalled();
    if (installedResult) {
        for (const auto& package : *installedResult) {
            // Simple search: check if query appears in name or description
            std::string lowerQuery(query);
            std::ranges::transform(lowerQuery, lowerQuery.begin(), ::tolower);

            std::string lowerName(package.info.id.name);
            std::ranges::transform(lowerName, lowerName.begin(), ::tolower);

            std::string lowerDesc(package.info.description);
            std::ranges::transform(lowerDesc, lowerDesc.begin(), ::tolower);

            if (lowerName.find(lowerQuery) != std::string::npos ||
                lowerDesc.find(lowerQuery) != std::string::npos) {
                searchResults.push_back(package);
            }
        }
    }

    // Remove duplicates
    std::set<PackageId> seen;
    auto end = std::ranges::remove_if(searchResults,
									  [&seen](const Package &pkg) {
										  return !seen.insert(pkg.info.id).second;
									  })
					   .begin();
    searchResults.erase(end, searchResults.end());

    return searchResults;
}

Result<Package> PackageManager::GetPackageInfo(const PackageId& id, const std::optional<plg::version>& version) {
    // First check installed packages
    auto installedResult = ListInstalled();
    if (installedResult) {
        for (const auto& package : *installedResult) {
            if (package.info.id == id) {
                if (!version || package.info.version == *version) {
                    return package;
                }
            }
        }
    }

    // Then check repositories
    return FindPackageInRepositories(id, version);
}

// ----------------------------------------------------------------------------
// Package Operations
// ----------------------------------------------------------------------------

Result<void> PackageManager::Install(
    const PackageId& id,
    const std::optional<plg::version>& version,
    ProgressCallback progress) {

    // Check if already installed
    auto installedResult = ListInstalled();
    if (installedResult) {
        for (const auto& pkg : *installedResult) {
            if (pkg.info.id == id) {
                if (!version || pkg.info.version == *version) {
                    return Error::AlreadyInstalled(id.name);
                }
            }
        }
    }

    // Find package in repositories
    auto packageResult = FindPackageInRepositories(id, version);
    if (!packageResult) {
        return packageResult.error();
    }

    const Package& package = *packageResult;

    // Check dependencies if enabled
    if (_config.autoResolveDependencies) {
        auto availableResult = ListAvailable();
        if (!availableResult) {
            return availableResult.error();
        }

        auto depResult = _dependencyResolver->ResolveDependencies(package, *availableResult);
    	if (!depResult.success) {
    		std::vector<std::string_view> missingDeps;
    		for (const auto& dep : depResult.missingDependencies) {
    			missingDeps.emplace_back(dep.id.name);
    		}
            return Error::DependencyMissing(plg::join(missingDeps, ", "));
        }

        // Install dependencies first
        for (const auto& depPackage : depResult.requiredPackages) {
            if (!IsPackageInstalled(depPackage)) {
                auto depInstallResult = InstallPackageWithDependencies(depPackage, progress);
                if (!depInstallResult) {
                    return depInstallResult;
                }
            }
        }
    }

    // Install the package
    return InstallPackageWithDependencies(package, progress);
}

Result<void> PackageManager::Remove(const PackageId& id, bool removeDependents) {
    // Find installed package
    auto installedResult = ListInstalled();
    if (!installedResult) {
        return installedResult.error();
    }

	const auto it = std::ranges::find_if(*installedResult, [&id](const Package& pkg) {
        return pkg.info.id == id;
    });

    if (it == installedResult->end()) {
        return Error::NotInstalled(id.name);
    }

    const Package& package = *it;

    // Check for dependent packages
    if (!removeDependents) {
        for (const auto& installed : *installedResult) {
            for (const auto& dep : installed.info.dependencies) {
                if (dep.id == id && !dep.optional) {
                    // Another package depends on this one
                	return Error::DependencyMissing(std::format("{} depends on {}", installed.info.id.name, id.name));
                }
            }
        }
    }

    // Remove package files
    if (package.installPath) {
        try {
            fs::remove_all(*package.installPath);
        } catch (const fs::filesystem_error& er) {
            return Error::FileSystem("remove package", er);
        }
    }

    // Remove dependents if requested
    if (removeDependents) {
        for (const auto& installed : *installedResult) {
            for (const auto& dep : installed.info.dependencies) {
                if (dep.id == id && !dep.optional) {
                    Remove(installed.info.id, true);
                }
            }
        }
    }

    InvalidateCache();
    return {};
}

Result<void> PackageManager::Update(
    const PackageId& id,
    const std::optional<plg::version>& targetVersion,
    ProgressCallback progress) {

    // Find currently installed package
    auto installedResult = ListInstalled();
    if (!installedResult) {
        return installedResult.error();
    }

    auto it = std::ranges::find_if(*installedResult, [&id](const Package& pkg) {
        return pkg.info.id == id;
    });

    if (it == installedResult->end()) {
        return Error::NotInstalled(id.name);
    }

    const Package& currentPackage = *it;
    plg::version currentVersion = currentPackage.info.version;

    // Find target version (latest if not specified)
    auto targetPackageResult = FindPackageInRepositories(id, targetVersion);
    if (!targetPackageResult) {
        return targetPackageResult.error();
    }

     const Package& targetPackage = *targetPackageResult;

    // Check if update is needed
    if (currentVersion == targetPackage.info.version) {
        return {}; // Already at target version
    }

    // Backup current installation
    fs::path backupPath = _config.cacheDirectory / std::format("backup_{}_{}", id.name, DateTime::Get("%Y_%m_%d_%H_%M_%S"));

    if (currentPackage.installPath) {
        try {
            fs::create_directories(backupPath);
            fs::copy(*currentPackage.installPath, backupPath,
                    fs::copy_options::recursive);
        } catch (const fs::filesystem_error& er) {
            return Error::FileSystem("create backup", er);
        }
    }

    // Remove current version
    auto removeResult = Remove(id, false);
    if (!removeResult) {
        // Restore backup on failure
        if (currentPackage.installPath && fs::exists(backupPath)) {
            try {
                fs::copy(backupPath, *currentPackage.installPath,
                        fs::copy_options::recursive);
            } catch (const fs::filesystem_error&) {}
        }
        return removeResult;
    }

    // Install new version
    auto installResult = Install(id, targetPackage.info.version, progress);
    if (!installResult) {
        // Restore backup on failure
        if (currentPackage.installPath && fs::exists(backupPath)) {
            try {
                fs::copy(backupPath, *currentPackage.installPath,
                        fs::copy_options::recursive);
            } catch (const fs::filesystem_error&) {}
        }
        return installResult;
    }

    // Clean up backup
    if (fs::exists(backupPath)) {
        try {
            fs::remove_all(backupPath);
        } catch (const fs::filesystem_error&) {}
    }

    return {};
}

Result<void> PackageManager::UpdateAll(
    const std::optional<std::unordered_map<PackageId, plg::version>>& targetVersions,
    ProgressCallback progress) {

    auto installedResult = ListInstalled();
    if (!installedResult) {
        return installedResult.error();
    }

    size_t totalPackages = installedResult->size();
    size_t currentPackage = 0;

    for (const auto& package : *installedResult) {
        if (progress) {
            float progressValue = static_cast<float>(currentPackage) / totalPackages;
            progress(std::format("Updating: {}", package.info.id.name), progressValue);
        }

        std::optional<plg::version> targetVersion;
        if (targetVersions) {
            auto it = targetVersions->find(package.info.id);
            if (it != targetVersions->end()) {
                targetVersion = it->second;
            }
        }

        auto updateResult = Update(package.info.id, targetVersion, nullptr);
        if (!updateResult) {
            // Log error but continue with other packages
        }

        currentPackage++;
    }

    if (progress) {
        progress("Update complete", 1.0f);
    }

    return {};
}

// ----------------------------------------------------------------------------
// Dependency & Conflict Management
// ----------------------------------------------------------------------------

Result<DependencyResolutionResult> PackageManager::CheckDependencies(const PackageId& id) {
    auto packageResult = GetPackageInfo(id);
    if (!packageResult) {
        return packageResult.error();
    }

    auto availableResult = ListAvailable();
    if (!availableResult) {
        return availableResult.error();
    }

    return _dependencyResolver->ResolveDependencies(*packageResult, *availableResult);
}

Result<std::vector<ConflictInfo>> PackageManager::CheckConflicts() {
    auto installedResult = ListInstalled();
    if (!installedResult) {
        return installedResult.error();
    }

    return _conflictResolver->DetectConflicts(*installedResult);
}

Result<void> PackageManager::ResolveConflicts(ConflictResolutionStrategy strategy) {
    auto conflictsResult = CheckConflicts();
    if (!conflictsResult) {
        return conflictsResult.error();
    }

    if (conflictsResult->empty()) {
        return {}; // No conflicts
    }

    auto resolvedResult = _conflictResolver->ResolveConflicts(*conflictsResult, strategy);

    if (!resolvedResult) {
        return resolvedResult.error();
    }

    // Apply resolution (install/remove packages as needed)
    for (const auto& package : *resolvedResult) {
        if (package.installed) {
            Install(package.info.id, package.info.version, nullptr);
        } else {
            Remove(package.info.id, false);
        }
    }

    return {};
}

// ----------------------------------------------------------------------------
// System Verification
// ----------------------------------------------------------------------------

Result<std::vector<std::pair<PackageId, bool>>> PackageManager::VerifyIntegrity() {
    auto installedResult = ListInstalled();
    if (!installedResult) {
        return installedResult.error();
    }

    std::vector<std::pair<PackageId, bool>> results;

    for (const auto& package : *installedResult) {
        bool valid = true;

        if (_config.verifyChecksums && package.installPath) {
            // Find the repository that can verify this package
            for (const auto& repo : _repositories) {
                auto verifyResult = repo->VerifyPackage(package, *package.installPath);
                if (verifyResult) {
                    valid = *verifyResult;
                    break;
                }
            }
        }

        results.emplace_back(package.info.id, valid);
    }

    return results;
}

Result<void> PackageManager::CleanCache() {
    try {
        // Remove all files in cache directory except subdirectories
        for (const auto& entry : fs::directory_iterator(_config.cacheDirectory)) {
            if (entry.is_regular_file()) {
                fs::remove(entry.path());
            }
        }

        // Remove old backup directories (older than 7 days)
        auto now = std::chrono::system_clock::now();
        for (const auto& entry : fs::directory_iterator(_config.cacheDirectory)) {
            if (entry.is_directory() && entry.path().filename().string().starts_with("backup_")) {
                auto lastWrite = fs::last_write_time(entry.path());
                auto age = now - fs::file_time_type::clock::to_sys(lastWrite);
                if (age > std::chrono::days(7)) {
                    fs::remove_all(entry.path());
                }
            }
        }
    } catch (const fs::filesystem_error& er) {
        return Error::FileSystem("clean cache", er);
    }

    return {};
}

// ----------------------------------------------------------------------------
// Additional Configuration Methods
// ----------------------------------------------------------------------------

void PackageManager::SetConflictResolver(std::unique_ptr<IConflictResolver> resolver) {
    _conflictResolver = std::move(resolver);
}

void PackageManager::SetDependencyResolver(std::unique_ptr<IDependencyResolver> resolver) {
    _dependencyResolver = std::move(resolver);
}

Result<void> PackageManager::RescanInstalledPackages() {
    InvalidateCache();
    auto result = ListInstalled(); // This will trigger a scan
    return result ? Result<void>{} : result.error();
}

// ----------------------------------------------------------------------------
// Private Helper Methods
// ----------------------------------------------------------------------------

Result<Package> PackageManager::FindPackageInRepositories(
    const PackageId& id,
    const std::optional<plg::version>& version) {

    Package bestMatch;
    bool found = false;

    for (const auto& repository : _repositories) {
		if (auto result = repository->GetPackage(id, version)) {
            if (!version) {
                // Looking for latest version
                if (!found || IsNewerVersion(result->info.version, bestMatch.info.version)) {
                    bestMatch = *result;
                    found = true;
                }
            } else {
                // Found exact version
                return *result;
            }
        }
    }

    if (found) {
        return bestMatch;
    }

    return Error::PackageNotFound(id.name);
}

Result<void> PackageManager::InstallPackageWithDependencies(
    const Package& package,
    ProgressCallback progress) {

    if (progress) {
        progress(std::format("Installing: {}", package.info.id.name), 0.0f);
    }

    // Determine installation path
    fs::path installPath = _config.installDirectory / package.info.id.name;

    // Download if remote package
    if (package.location.IsRemote()) {
        fs::path downloadPath = _config.cacheDirectory /
            std::format("{}_{}.pkg", package.info.id.name,
                       package.info.version.major);

        // Find repository that can download this package
        bool downloaded = false;
        for (const auto& repo : _repositories) {
            auto downloadResult = repo->DownloadPackage(package, downloadPath, progress);
            if (downloadResult) {
                // Extract/copy to installation directory
                try {
                    fs::create_directories(installPath);
                    // Here you would extract the package archive
                    // For now, just copy
                    fs::copy(*downloadResult, installPath,
                            fs::copy_options::recursive);
                    downloaded = true;
                    break;
                } catch (const fs::filesystem_error&) {
                    return ErrorCode::FileSystemError;
                }
            }
        }

        if (!downloaded) {
            return ErrorCode::DownloadFailed;
        }
    } else if (package.location.IsLocal()) {
        // Copy from local location
        try {
            const auto& sourcePath = std::get<fs::path>(package.location.source);
            fs::create_directories(installPath);
            fs::copy(sourcePath, installPath, fs::copy_options::recursive);
        } catch (const fs::filesystem_error&) {
            return ErrorCode::FileSystemError;
        }
    }

    // Verify installation
    if (_config.verifyChecksums) {
        auto verifyResult = ValidatePackageInstallation(package);
        if (!verifyResult) {
            // Rollback installation
            try {
                fs::remove_all(installPath);
            } catch (const fs::filesystem_error&) {}
            return verifyResult;
        }
    }

    InvalidateCache();

    if (progress) {
        progress(std::format("Installed: {}", package.info.id.name), 1.0f);
    }

    return {};
}

Result<void> PackageManager::ValidatePackageInstallation(const Package& package) {
    fs::path installPath = _config.installDirectory / package.info.id.name;

    // Check if manifest exists
    bool manifestFound = false;
    for (const auto& manifestName : {"package.json", "manifest.json", "plugin.json"}) {
        if (fs::exists(installPath / manifestName)) {
            manifestFound = true;
            break;
        }
    }

    if (!manifestFound) {
    	return Error::Verification(std::format("{}: no manifest found", package.info.id.name));
    }

    // Additional validation could be added here
    // - Check file permissions
    // - Verify checksums
    // - Validate directory structure

    return {};
}

Result<void> PackageManager::ScanAndUpdateInstalledPackages() {
    if (!_packageScanner) {
        return {};
    }

    auto scanResult = _packageScanner->ScanForPackages();
    if (!scanResult) {
        return scanResult.error();
    }

    std::vector<Package> installedPackages;
    for (const auto& scannedPkg : *scanResult) {
        if (!scannedPkg.error) {
            Package pkg = scannedPkg.package;
            pkg.installed = true;
            pkg.installPath = scannedPkg.manifestPath.parent_path();
            installedPackages.push_back(std::move(pkg));
        }
    }

    _installedPackagesCache = std::move(installedPackages);
    return {};
}

void PackageManager::InvalidateCache() {
    _installedPackagesCache.reset();
    _availablePackagesCache.reset();
}