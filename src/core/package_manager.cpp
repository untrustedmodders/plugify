#include "package_manager.hpp"

using namespace plugify;


PackageManager::PackageManager(
    std::unique_ptr<IPackageScanner> scanner,
    std::unique_ptr<IDependencyResolver> depResolver,
    std::unique_ptr<IConflictResolver> conflictResolver,
    std::shared_ptr<IHTTPDownloader> downloader)
    : _scanner(std::move(scanner))
    , _dependencyResolver(std::move(depResolver))
    , _conflictResolver(std::move(conflictResolver))
    , _httpDownloader(std::move(downloader))
    , _packageRoot(fs::current_path() / "packages") {
    
    if (!_scanner) {
        throw std::invalid_argument("PackageScanner cannot be null");
    }
    if (!_dependencyResolver) {
        throw std::invalid_argument("DependencyResolver cannot be null");
    }
    if (!_conflictResolver) {
        throw std::invalid_argument("ConflictResolver cannot be null");
    }
}

void PackageManager::AddRepository(std::unique_ptr<IPackageRepository> repository) {
    if (!repository) {
        return;
    }
    
    std::unique_lock lock(_repositoryMutex);
    
    auto identifier = repository->GetIdentifier();
    
    // Check if repository with same identifier already exists
    auto it = std::ranges::find(_repositories, identifier,
        [](const auto& repo) { return repo->GetIdentifier(); });
    
    if (it != _repositories.end()) {
        // Replace existing repository
        *it = std::move(repository);
    } else {
        _repositories.push_back(std::move(repository));
    }
    
    // Invalidate remote cache when repositories change
    _remoteCacheValid = false;
}

bool PackageManager::RemoveRepository(std::string_view identifier) {
    std::unique_lock lock(_repositoryMutex);
    
    auto it = std::ranges::find(_repositories, identifier,
        [](const auto& repo) { return repo->GetIdentifier(); });
    
    if (it != _repositories.end()) {
        _repositories.erase(it);
        _remoteCacheValid = false;
        return true;
    }
    
    return false;
}

Result<void> PackageManager::RefreshRepositories() {
    std::unique_lock lock(_repositoryMutex);
    
    _availablePackages.clear();
    std::vector<Error> errors;
    
    // Fetch packages from all repositories in parallel if possible
    for (auto& repository : _repositories) {
        if (!repository->IsAvailable()) {
            errors.push_back({
                ErrorCode::NetworkError,
                std::format("Repository '{}' is not available", repository->GetIdentifier())
            });
            continue;
        }
        
        auto result = repository->FetchPackages();
        if (result) {
            // Merge packages from this repository
            _availablePackages.insert(_availablePackages.end(),
                std::make_move_iterator(result->begin()),
                std::make_move_iterator(result->end()));
        } else {
            errors.push_back({
            	result.error(),
            	std::format("Failed to fetch packages from repository '{}'", repository->GetIdentifier())
            });
        }
    }
    
    _remoteCacheValid = true;
    
    // Return error if all repositories failed
    if (!errors.empty() && _availablePackages.empty()) {
        return errors.front().code;
    }
    
    return {};
}

std::vector<LocalPackage> PackageManager::GetInstalledPackages() const {
    std::shared_lock lock(_cacheMutex);
    
    if (!_localCacheValid) {
        // Need to upgrade to unique lock for modification
        lock.unlock();
        std::unique_lock writeLock(_cacheMutex);
        
        // Double-check after acquiring write lock
        if (!_localCacheValid) {
            const_cast<PackageManager*>(this)->ScanLocalPackages();
        }
    }
    
    return _installedPackages;
}

std::vector<RemotePackage> PackageManager::GetAvailablePackages() const {
    std::shared_lock lock(_cacheMutex);
    
    if (!_remoteCacheValid) {
        lock.unlock();
        std::unique_lock writeLock(_cacheMutex);
        
        if (!_remoteCacheValid) {
            const_cast<PackageManager*>(this)->RefreshRepositories();
        }
    }
    
    return _availablePackages;
}

std::vector<LocalPackage> PackageManager::GetInstalledPackagesByType(std::string_view type) const {
    auto packages = GetInstalledPackages();
    
    // Filter packages by type using ranges
    auto filtered = packages | std::views::filter([type](const LocalPackage& pkg) {
        return pkg.type == type;
    });
    
    return std::vector<LocalPackage>(filtered.begin(), filtered.end());
}

InstallResult PackageManager::Install(std::span<const std::string> packageNames) {
    InstallResult result{};
    
    if (packageNames.empty()) {
        result.errors.push_back({ErrorCode::PackageNotFound, "No packages specified"});
        return result;
    }
    
    // First, ensure we have fresh repository data
    auto refreshResult = RefreshRepositories();
    if (!refreshResult && _availablePackages.empty()) {
        result.errors.push_back({refreshResult.error(), "Failed to refresh repositories"});
        return result;
    }
    
    // Get current installed packages
    auto installedPackages = GetInstalledPackages();
    
    // Calculate installation order considering dependencies
    auto orderResult = _dependencyResolver->CalculateInstallOrder(
        packageNames,
        installedPackages,
        _availablePackages
    );
    
    if (!orderResult) {
        result.errors.push_back({orderResult.error(), "Failed to calculate installation order"});
        return result;
    }
    
    // Install packages in calculated order
    for (const auto& packageName : *orderResult) {
        // Skip if already installed
        if (IsPackageInstalled(packageName)) {
            continue;
        }
        
        // Find package in available packages
        auto it = std::ranges::find(_availablePackages, packageName, &RemotePackage::name);
        
        if (it == _availablePackages.end()) {
            result.errors.push_back({
                ErrorCode::PackageNotFound,
                std::format("Package '{}' not found in repositories", packageName)
            });
            continue;
        }
        
        // Find best version that satisfies constraints
        auto versionResult = FindBestVersion(*it, std::nullopt);
        if (!versionResult) {
            result.errors.push_back({
            	versionResult.error(),
				std::format("No suitable version found for package '{}'", packageName)
            });
            continue;
        }
        
        // Check for conflicts before installation
        if (_conflictResolver->WouldConflict(*it, **versionResult, installedPackages)) {
            // Try to resolve based on strategy
            std::vector<ConflictInfo> conflicts;
            // Detect specific conflicts
            auto conflictResult = _conflictResolver->ResolveConflicts(conflicts, _conflictStrategy);
            
            if (!conflictResult) {
                result.errors.push_back({
                    ErrorCode::VersionConflict,
                    std::format("Package '{}' conflicts with installed packages", packageName)
                });
                continue;
            }
        }
        
        // Install the package
        auto installResult = InstallPackageVersion(*it, **versionResult);
        if (installResult) {
            result.installedPackages.push_back(packageName);
            // Invalidate local cache
            _localCacheValid = false;
        } else {
            result.errors.push_back({installResult.error(), std::format("Failed to install package '{}'", packageName)});});
        }
    }
    
    result.success = !result.installedPackages.empty();
    return result;
}

RemoveResult PackageManager::Remove(std::span<const std::string> packageNames) {
    RemoveResult result{};
    
    if (packageNames.empty()) {
        result.errors.push_back({ErrorCode::PackageNotFound, "No packages specified"});
        return result;
    }
    
    auto installedPackages = GetInstalledPackages();
    
    for (const auto& packageName : packageNames) {
        // Find package in installed packages
        auto it = std::ranges::find(installedPackages, packageName, &LocalPackage::name);
        
        if (it == installedPackages.end()) {
            result.errors.push_back({
                ErrorCode::PackageNotFound,
                std::format("Package '{}' is not installed", packageName)
            });
            continue;
        }
        
        // Check if other packages depend on this one
        bool hasDependents = false;
        for (const auto& pkg : installedPackages) {
            if (pkg.name == packageName) continue;
            
            // Check if pkg depends on packageName
            if (pkg.descriptor) {
                // For plugins, check dependencies
                if (auto pluginDesc = std::dynamic_pointer_cast<PluginDescriptor>(pkg.descriptor)) {
                    if (pluginDesc->dependencies) {
                        auto depIt = std::ranges::find_if(*pluginDesc->dependencies,
                            [&packageName](const auto& dep) {
                                return dep.name == packageName && !dep.optional.value_or(false);
                            });
                        
                        if (depIt != pluginDesc->dependencies->end()) {
                            hasDependents = true;
                            break;
                        }
                    }
                }
            }
        }
        
        if (hasDependents) {
            result.errors.push_back({
                ErrorCode::DependencyMissing,
                std::format("Cannot remove '{}': other packages depend on it", packageName)
            });
            continue;
        }
        
        // Remove the package
        auto removeResult = RemoveLocalPackage(*it);
        if (removeResult) {
            result.removedPackages.push_back(packageName);
            // Invalidate local cache
            _localCacheValid = false;
        } else {
            result.errors.push_back(removeResult.error());
        }
    }
    
    result.success = !result.removedPackages.empty();
    return result;
}

UpdateResult PackageManager::Update(std::span<const std::string> packageNames) {
    UpdateResult result{};
    
    // Refresh repository data first
    auto refreshResult = RefreshRepositories();
    if (!refreshResult) {
        result.errors.push_back(refreshResult.error());
        return result;
    }
    
    auto installedPackages = GetInstalledPackages();
    
    // If no specific packages specified, update all
    std::vector<std::string> packagesToUpdate;
    if (packageNames.empty()) {
        packagesToUpdate.reserve(installedPackages.size());
        for (const auto& pkg : installedPackages) {
            packagesToUpdate.push_back(pkg.name);
        }
    } else {
        packagesToUpdate.assign(packageNames.begin(), packageNames.end());
    }
    
    for (const auto& packageName : packagesToUpdate) {
        // Find installed package
        auto installedIt = std::ranges::find(installedPackages, packageName, &LocalPackage::name);
        
        if (installedIt == installedPackages.end()) {
            result.errors.push_back({
                ErrorCode::PackageNotFound,
                std::format("Package '{}' is not installed", packageName)
            });
            continue;
        }
        
        // Find available package
        auto availableIt = std::ranges::find(_availablePackages, packageName, &RemotePackage::name);
        
        if (availableIt == _availablePackages.end()) {
            // No update available
            continue;
        }
        
        // Find newer version
        PackageVersion* newerVersion = nullptr;
        for (auto& version : availableIt->versions) {
            if (version.version > installedIt->version) {
                if (!newerVersion || version.version > newerVersion->version) {
                    newerVersion = &version;
                }
            }
        }
        
        if (!newerVersion) {
            // Already at latest version
            continue;
        }
        
        // Check conflicts with new version
        if (_conflictResolver->WouldConflict(*availableIt, *newerVersion, installedPackages)) {
            result.errors.push_back({
                ErrorCode::VersionConflict,
                std::format("Update of '{}' would cause conflicts", packageName)
            });
            continue;
        }
        
        // Remove old version
        auto removeResult = RemoveLocalPackage(*installedIt);
        if (!removeResult) {
            result.errors.push_back(removeResult.error());
            continue;
        }
        
        // Install new version
        auto installResult = InstallPackageVersion(*availableIt, *newerVersion);
        if (installResult) {
            result.updatedPackages.emplace_back(packageName, newerVersion->version);
            _localCacheValid = false;
        } else {
            result.errors.push_back(installResult.error());
            // Try to rollback by reinstalling old version
            // (Implementation would go here)
        }
    }
    
    result.success = !result.updatedPackages.empty();
    return result;
}

std::vector<std::variant<LocalPackage, RemotePackage>> PackageManager::Search(std::string_view query) {
    std::vector<std::variant<LocalPackage, RemotePackage>> results;
    
    // Search in installed packages
    auto installedPackages = GetInstalledPackages();
    for (const auto& pkg : installedPackages) {
        if (pkg.name.find(query) != std::string::npos ||
            (pkg.descriptor && pkg.descriptor->friendlyName.find(query) != std::string::npos)) {
            results.emplace_back(pkg);
        }
    }
    
    // Search in remote repositories
    std::shared_lock lock(_repositoryMutex);
    for (auto& repository : _repositories) {
        auto searchResult = repository->SearchPackages(query);
        if (searchResult) {
            for (auto& pkg : *searchResult) {
                results.emplace_back(std::move(pkg));
            }
        }
    }
    
    return results;
}

std::optional<std::variant<LocalPackage, RemotePackage>> PackageManager::GetPackageInfo(std::string_view name) {
    // First check installed packages
    auto installedPackages = GetInstalledPackages();
    auto installedIt = std::ranges::find(installedPackages, name, &LocalPackage::name);
    
    if (installedIt != installedPackages.end()) {
        return *installedIt;
    }
    
    // Then check available packages
    auto availablePackages = GetAvailablePackages();
    auto availableIt = std::ranges::find(availablePackages, name, &RemotePackage::name);
    
    if (availableIt != availablePackages.end()) {
        return *availableIt;
    }
    
    return std::nullopt;
}

Result<std::vector<PackageConstraint>> PackageManager::CheckDependencies(std::string_view packageName) {
    // Find package info
    auto packageInfo = GetPackageInfo(packageName);
    if (!packageInfo) {
        return std::unexpected(Error{
            ErrorCode::PackageNotFound,
            std::format("Package '{}' not found", packageName)
        });
    }
    
    // Get dependencies based on package type
    std::vector<PackageConstraint> dependencies;
    
    std::visit([&dependencies](const auto& pkg) {
        using T = std::decay_t<decltype(pkg)>;
        
        if constexpr (std::is_same_v<T, LocalPackage>) {
            if (pkg.descriptor) {
                if (auto pluginDesc = std::dynamic_pointer_cast<PluginDescriptor>(pkg.descriptor)) {
                    if (pluginDesc->dependencies) {
                        for (const auto& dep : *pluginDesc->dependencies) {
                            dependencies.push_back({
                                dep.name,
                                dep.version ? std::vector{*dep.version} : std::vector<VersionConstraint>{},
                                dep.optional
                            });
                        }
                    }
                }
            }
        } else if constexpr (std::is_same_v<T, RemotePackage>) {
            // For remote packages, check the latest version's dependencies
            if (!pkg.versions.empty()) {
                const auto& latestVersion = *std::ranges::max_element(pkg.versions,
                    {}, &PackageVersion::version);
                
                if (latestVersion.dependencies) {
                    dependencies = *latestVersion.dependencies;
                }
            }
        }
    }, *packageInfo);
    
    return dependencies;
}

Result<void> PackageManager::VerifySystemIntegrity() {
    auto installedPackages = GetInstalledPackages();
    
    // Check for conflicts
    auto conflicts = _conflictResolver->DetectConflicts(installedPackages);
    if (!conflicts.empty()) {
        // Try to resolve conflicts based on strategy
        auto resolveResult = _conflictResolver->ResolveConflicts(conflicts, _conflictStrategy);
        if (!resolveResult) {
            return std::unexpected(Error{
                ErrorCode::VersionConflict,
                std::format("System has {} unresolved conflicts", conflicts.size())
            });
        }
        
        // Apply resolutions
        _installedPackages = std::move(*resolveResult);
        _localCacheValid = true;
    }
    
    // Check all dependencies are satisfied
    for (const auto& pkg : installedPackages) {
        if (!pkg.descriptor) continue;
        
        std::vector<PackageConstraint> dependencies;
        
        // Extract dependencies based on package type
        if (auto pluginDesc = std::dynamic_pointer_cast<PluginDescriptor>(pkg.descriptor)) {
            if (pluginDesc->dependencies) {
                for (const auto& dep : *pluginDesc->dependencies) {
                    dependencies.push_back({
                        dep.name,
                        dep.version ? std::vector{*dep.version} : std::vector<VersionConstraint>{},
                        dep.optional
                    });
                }
            }
        }
        
        if (!dependencies.empty()) {
            if (!_dependencyResolver->AreDependenciesSatisfied(dependencies, installedPackages)) {
                return std::unexpected(Error{
                    ErrorCode::DependencyMissing,
                    std::format("Package '{}' has unsatisfied dependencies", pkg.name)
                });
            }
        }
    }
    
    return {};
}

void PackageManager::SetConflictStrategy(ConflictResolutionStrategy strategy) {
    _conflictStrategy = strategy;
}

void PackageManager::SetPackageRoot(const fs::path& path) {
    std::unique_lock lock(_cacheMutex);
    _packageRoot = path;
    _localCacheValid = false;
    
    // Create directory if it doesn't exist
    std::error_code ec;
    if (!fs::exists(_packageRoot, ec)) {
        fs::create_directories(_packageRoot, ec);
        // Error is ignored here, will be caught when trying to scan
    }
}

fs::path PackageManager::GetPackageRoot() const {
    std::shared_lock lock(_cacheMutex);
    return _packageRoot;
}

// Private helper methods

Result<void> PackageManager::ScanLocalPackages() {
    auto scanResult = _scanner->ScanDirectory(_packageRoot);
    if (!scanResult) {
        return std::unexpected(scanResult.error());
    }
    
    _installedPackages = std::move(*scanResult);
    _localCacheValid = true;
    
    return {};
}

Result<PackageVersion*> PackageManager::FindBestVersion(
    RemotePackage& package,
    const std::optional<VersionConstraint>& constraint) {
    
    if (package.versions.empty()) {
        return std::unexpected(Error{
            ErrorCode::PackageNotFound,
            std::format("No versions available for package '{}'", package.name)
        });
    }
    
    // Sort versions in descending order
    std::ranges::sort(package.versions, std::greater{}, &PackageVersion::version);
    
    // Find best matching version
    for (auto& version : package.versions) {
        // Check platform compatibility
        if (version.platforms) {
            // Check if current platform is supported
            // (Platform detection implementation would go here)
            bool platformSupported = true; // Placeholder
            if (!platformSupported) {
                continue;
            }
        }
        
        // Check version constraint if provided
        if (constraint) {
            bool satisfies = false;
            switch (constraint->type) {
                case VersionConstraint::Type::Equal:
                    satisfies = version.version == constraint->version;
                    break;
                case VersionConstraint::Type::GreaterEqual:
                    satisfies = version.version >= constraint->version;
                    break;
                case VersionConstraint::Type::Greater:
                    satisfies = version.version > constraint->version;
                    break;
                case VersionConstraint::Type::LessEqual:
                    satisfies = version.version <= constraint->version;
                    break;
                case VersionConstraint::Type::Less:
                    satisfies = version.version < constraint->version;
                    break;
                case VersionConstraint::Type::NotEqual:
                    satisfies = version.version != constraint->version;
                    break;
                case VersionConstraint::Type::Compatible:
                    // Compatible means same major version
                    satisfies = version.version.major == constraint->version.major &&
                               version.version >= constraint->version;
                    break;
                case VersionConstraint::Type::Any:
                    satisfies = true;
                    break;
            }
            
            if (!satisfies) {
                continue;
            }
        }
        
        return &version;
    }
    
    return Error{
        ErrorCode::PackageNotFound,
        std::format("No suitable version found for package '{}'", package.name)
    };
}

Result<void> PackageManager::InstallPackageVersion(
    const RemotePackage& package,
    const PackageVersion& version) {
    
    // Find repository that has this package
    std::shared_lock lock(_repositoryMutex);
    
    for (auto& repository : _repositories) {
        auto packages = repository->FetchPackages();
        if (!packages) continue;
        
        auto it = std::ranges::find(*packages, package.name, &RemotePackage::name);
        
        if (it != packages->end()) {
            // Download package
            auto downloadResult = repository->DownloadPackage(
                package,
                version,
                [](uint32_t done, uint32_t total) {
                    // Progress callback
                    // Could emit progress events here
                    return true; // Continue download
                }
            );
            
            if (!downloadResult) {
                return std::unexpected(downloadResult.error());
            }
            
            // Extract package to package root
            auto packageDir = _packageRoot / package.name;
            
            // Create package directory
            std::error_code ec;
            fs::create_directories(packageDir, ec);
            if (ec) {
                return std::unexpected(Error{
                    ErrorCode::FileSystemError,
                    std::format("Failed to create directory for package '{}': {}", 
                        package.name, ec.message())
                });
            }
            
            // Extract downloaded package to directory
            // (Extraction implementation would go here)
            
            // Verify package integrity
            LocalPackage localPkg{
                package.name,
                package.type,
                packageDir,
                version.version,
                nullptr // Descriptor will be loaded by scanner
            };
            
            auto verifyResult = _scanner->VerifyPackage(localPkg);
            if (!verifyResult || !*verifyResult) {
                // Cleanup on failure
                fs::remove_all(packageDir, ec);
                return std::unexpected(Error{
                    ErrorCode::ChecksumMismatch,
                    std::format("Package '{}' verification failed", package.name)
                });
            }
            
            return {};
        }
    }
    
    return std::unexpected(Error{
        ErrorCode::PackageNotFound,
        std::format("Package '{}' not found in any repository", package.name)
    });
}

Result<void> PackageManager::RemoveLocalPackage(const LocalPackage& package) {
    // Remove package directory
    std::error_code ec;
    if (fs::exists(package.path, ec)) {
        fs::remove_all(package.path, ec);
        if (ec) {
            return std::unexpected(Error{
                ErrorCode::FileSystemError,
                std::format("Failed to remove package '{}': {}", package.name, ec.message())
            });
        }
    }
    
    // Remove from cache
    auto it = std::ranges::find(_installedPackages, package.name, &LocalPackage::name);
    
    if (it != _installedPackages.end()) {
        _installedPackages.erase(it);
    }
    
    return {};
}

bool PackageManager::IsPackageInstalled(std::string_view name) const {
    auto packages = GetInstalledPackages();
    return std::ranges::any_of(packages,
        [name](const LocalPackage& pkg) {
            return pkg.name == name;
        });
}