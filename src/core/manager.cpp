#include "plugify/core/manager.hpp"
#include "plugify/core/config.hpp"
#include "plugify/core/package.hpp"
#include "plugify/core/plugify.hpp"

#include "core/glaze.hpp"
#include <glaze/glaze.hpp>
#include <utility>

using namespace plugify;

struct InitResult {
    bool success;
    PackageState finalState;
    std::optional<Error> error;
    std::size_t retryAttempts;
    std::chrono::milliseconds loadTime;
};

struct LoadResult {
    bool success;
    std::optional<Error> error;
    std::size_t retryAttempts;
};

struct Manager::Impl {
	// Plugify
	Plugify& plugify;

	// Configuration
	std::shared_ptr<Config> config;

	// Discovered packages - now stored in sorted order
	//std::vector<ModuleInfo> sortedModules;  // Topologically sorted
	//std::vector<PluginInfo> sortedPlugins;  // Topologically sorted
    std::vector<PackageInfo> sortedPackages;

	// Original maps for quick lookup
	//std::unordered_map<PackageId, ModuleInfo, plg::string_hash, std::equal_to<>> modules;
	//std::unordered_map<PackageId, PluginInfo, plg::string_hash, std::equal_to<>> plugins;
	std::unordered_map<PackageId, PackageInfo, plg::string_hash, std::equal_to<>> packages;

	// Loaded instances
	//std::unordered_map<PackageId, std::shared_ptr<Module>, plg::string_hash, std::equal_to<> loadedModules;
	//std::unordered_map<PackageId, std::shared_ptr<Plugin>, plg::string_hash, std::equal_to<> loadedPlugins;
	//std::unordered_map<PackageId, bool, plg::string_hash, std::equal_to<>> loadedModules;
	//std::unordered_map<PackageId, bool, plg::string_hash, std::equal_to<>> loadedPlugins;

    std::unordered_set<PackageId> loadedPackages;
    std::unordered_map<std::string, bool> languageModules;  // language@ -> loaded

	// Dependency injection components
	//std::unique_ptr<IPackageDiscovery> packageDiscovery;
	//std::unique_ptr<IPackageValidator> packageValidator;
	std::unique_ptr<IDependencyResolver> dependencyResolver;
	//std::unique_ptr<IModuleLoader> moduleLoader;
	//std::unique_ptr<IPluginLoader> pluginLoader;

	// Event handling
	EventDispatcher eventDistpatcher;

	// Dependency tracking (maintained for quick lookup)
	//std::unordered_map<PackageId, std::vector<PackageId>, plg::string_hash, std::equal_to<>> pluginToModuleMap;
	//std::unordered_map<PackageId, std::vector<PackageId>, plg::string_hash, std::equal_to<>> moduleToPluginsMap;

	// Comprehensive initialization tracking
	//InitializationState initState;

	// Retry tracking
	//std::unordered_map<PackageId, std::size_t, plg::string_hash, std::equal_to<>> retryCounters;

	// Update performance tracking
    /*struct UpdateStatistics {
        std::chrono::microseconds lastUpdateTime{0};
        std::chrono::microseconds maxUpdateTime{0};
        std::chrono::microseconds totalUpdateTime{0};
        std::size_t totalUpdates{0};

        double AverageUpdateTime() const {
            return totalUpdates > 0 ?
                static_cast<double>(totalUpdateTime.count()) /  static_cast<double>(totalUpdates) : 0.0;
        }
    } updateStats;*/

    State<void> DiscoverPackages(std::span<const std::filesystem::path> searchPaths = {}) {
        // Combine provided paths with plugify.configured paths
        std::vector<std::filesystem::path> allPaths;
        allPaths.reserve(searchPaths.size() + config->searchPaths.size());
        allPaths.insert(allPaths.end(), searchPaths.begin(), searchPaths.end());
        allPaths.insert(allPaths.end(),
                        config->searchPaths.begin(),
                        config->searchPaths.end());

        auto packageResult = packageDiscovery->DiscoverPackages(allPaths);
        if (!packageResult) {
            return plg::unexpected(std::move(packageResult.error()));
        }

        // Store discovered modules
        for (const auto& package : *packageResult) {
            const auto& id = packageResult->id;

            if (config->whitelistedPackages && !std::ranges::contains(*config->whitelistedPackages, id) ||
                config->blacklistedPackages && std::ranges::contains(*config->blacklistedPackages, id)
            ) {
                package->state = PackageState::Disabled;

                EmitEvent({
                    .type = EventType::Disabled,
                    .timestamp = std::chrono::system_clock::now(),
                    .packageId = id
                });
            } else {
                package->state = PackageState::Discovered;

                EmitEvent({
                    .type = EventType::Discovered,
                    .timestamp = std::chrono::system_clock::now(),
                    .packageId = id
                });
            }

            packages[id] = std::move(package);
        }

        return {};
    }

    InitializationReport Initialize(const DependencyReport& depReport) {
        InitializationReport report;
        ReportTimer timer(report);

        report.inits.reserve(sortedPackages.size());

        // Single pass initialization
        std::cout << "\n=== Initializing All Packages ===\n";

        // Process each package in dependency order
        for (const auto& package : sortedPackages) {
            auto result = InitializePackage(depReport, package);

            InitializationReport::PackageInit init{
                .id = package->id,
                .finalState = result.finalState,
                .retryAttempts = result.retryAttempts,
                .error = std::move(result.error),
                .loadTime = result.loadTime
            };

            report.inits.push_back(std::move(init));

            // Early exit in strict mode
            if (!config->partialStartupMode && !result.success) {
                std::cout << "  ⚠️  Stopping initialization (strict mode)\n";
                break;
            }
        }

        // Mark remaining packages as skipped
        for (const auto& package : sortedPackages) {
            if (
                package->state == PackageState::Uninitialized ||
                package->state == PackageState::Validated
            ) {
                package->state = PackageState::Skipped;
                package->lastError = Error::Permanent(
                    ErrorCode::MissingDependency,
                    "Never attempted due to dependency cascade",
                    ErrorCategory::Dependency
                );

                EmitEvent({
                    .type = EventType::Skipped,
                    .timestamp = std::chrono::system_clock::now(),
                    .packageId = package->id,
                    .error = package->lastError,
                });

                InitializationReport::PackageInit init{
                    .id = package->id,
                    .finalState = PackageState::Skipped,
                    .retryAttempts = 0,
                    .error = package->lastError
                };

                report.inits.push_back(std::move(init));
            }
        }

        std::cout << std::format("Initialization complete: {}/{} passed\n",
                                         report.SuccessCount(), packages.size());
        return report;
    }

    ValidationReport ManifestValidation() {
        std::cout << "\n=== Manifests Validation ===\n";

        ValidationReport report;
        ReportTimer timer(report);

        report.results.reserve(packages.size());

        for (auto& [id, package] : packages) {
            if (package->state == PackageState::Disabled)
                continue;

            ValidationReport::PackageValidation validation{.id = id};

            auto result = packageValidator->validateManifest(package->manifest);
            if (!result) {
                validation.passed = false;
                validation.error = result.error();

                package->state = PackageState::Error;
                package->lastError = result.error();

                std::cerr << std::format("{} '{}' validation failed: {}\n",
                                         package->GetTypeName(), id, result.error().message);
            } else {
                validation.passed = true;
                package->state = PackageState::Validated;

                EmitEvent({
                    .type = EventType::Validated,
                    .timestamp = std::chrono::system_clock::now(),
                    .packageId = id
                });
            }

            report.results.push_back(std::move(validation));
        }

        std::cout << std::format("Validation complete: {}/{} passed\n", report.PassedCount(),packages.size());

        return report;
    }

    DependencyReport ResolveDependencies() {
        std::cout << "\n=== Resolve Dependencies ===\n";

        PackageCollection allPackages;
        allPackages.reserve(packages.size());

        for (auto& [id, info] : packages) {
            if (info->state != PackageState::Validated)
                continue;

            allPackages[id] = info->manifest;
        }

        return dependencyResolver->ResolveDependencies(allPackages);
    }

    void SortPackagesByDependencies(const DependencyReport& depReport) {
    	// Create ordered lists based on dependency resolution
    	sortedPackages.clear();

        if (!config->respectDependencyOrder || !depReport.isLoadOrderValid) {
            // Can't sort due to circular dependencies or config
            // Just copy as-is
            for (const auto& [id, info] : packages) {
                if (info->state == PackageState::Validated) {
                    sortedPackages.push_back(info);
                }
            }
            return;
        }

        // Use the load order from dependency resolution
        for (const auto& packageId : depReport.loadOrder) {
            if (auto it = packages.find(packageId); it != packages.end()) {
                if (it->second->state == PackageState::Validated) {
                    sortedPackages.push_back(it->second);
                }
            }
        }

        for (const auto& [id, info] : packages) {
            if (!std::ranges::contains(depReport.loadOrder, id)) {
                info->state = PackageState::Skipped;

                EmitEvent({
                    .type = EventType::Skipped,
                    .timestamp = std::chrono::system_clock::now(),
                    .packageId = id,
                    .error = Error::Permanent(
                        ErrorCode::VersionConflict,
                        "Version conflict or circular dependency detected",
                        ErrorCategory::Dependency
                    )
                });
            }
        }

        std::cout << std::format("Sorted {} packages by dependency order\n", sortedPackages.size());
    }

    InitResult InitializePackage(const DependencyReport& depReport, PackageInfo package) {
        auto initStart = std::chrono::steady_clock::now();

        const auto& id = package->id;

        std::cout << std::format("  Initializing {} '{}'", package->GetTypeName(), id);

        // Step 1: Check dependencies (same for all)
        if (auto depError = CheckDependencies(package)) {
            std::cout << " - skipped (missing dependencies)\n";
            return {
                .success = false,
                .finalState = PackageState::Skipped,
                .error = std::move(depError),
                .retryAttempts = 0,
                .loadTime = ElapsedSince(initStart)
            };
        }

        // Step 2: Check language requirement (only for plugins)
        if (package->IsPlugin()) {
            const auto& language = package->GetLanguageName();
            if (!languageModules[language]) {
                std::cout << std::format(" - skipped (language '{}' not loaded)\n", language);
                return {
                    .success = false,
                    .finalState = PackageState::Skipped,
                    .error = Error::Permanent(
                        ErrorCode::LanguageModuleNotLoaded,
                        std::format("Required language module '{}' is not loaded", language),
                        ErrorCategory::Dependency
                    ),
                    .retryAttempts = 0,
                    .loadTime = ElapsedSince(initStart)
                };
            }
        }

        // Step 3: Load with retry (same logic for all)
        package->state = PackageState::Initializing;
        EmitEvent({
            .type = EventType::Loading,
            .timestamp = std::chrono::system_clock::now(),
            .packageId = id
        });

        auto loadResult = LoadWithRetry(package);

        if (loadResult.success) {
            // Record success
            loadedPackages.insert(id);

            // If this is a module, mark its language as available
            if (package->IsModule()) {
                const auto& language = package->GetLanguageName();
                languageModules[language] = true;
            }

            package->state = PackageState::Ready;

            EmitEvent({
                .type = EventType::Loaded,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = package->id
            });

            std::cout << " ✓";
            if (loadResult.retryAttempts > 0) {
                std::cout << std::format(" (after {} retries)", loadResult.retryAttempts);
            }
            std::cout << "\n";
        } else {
            package->state = PackageState::Error;
            package->lastError = loadResult.error;

            EmitEvent({
                .type = EventType::Failed,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = package->id,
                .error = loadResult.error
            });

            std::cout << std::format(" ✗ {}\n", loadResult.error->message);

            // Check if any packages depend on this failed plugin
            auto& reverseDeps = depReport.reverseDependencyGraph;
            auto it = reverseDeps.find(id);
            if (it != reverseDeps.end()) {
                const auto& dependents = it->second;
                if (!dependents.empty()) {
                    std::cerr << std::format("     ⚠️  This will prevent loading: [{}]\n", plg::join(dependents, ", "));
                }
            }
        }

        loadResult.loadTime = ElapsedSince(initStart);
        return loadResult;
    }

    LoadResult LoadWithRetry(PackageInfo package) {
        const auto maxAttempts = config->retryPolicy.maxAttempts;

        for (std::size_t attempt = 1; attempt <= maxAttempts + 1; ++attempt) {
            // Wait before retry
            if (attempt > 1) {
                auto delay = GetRetryDelay(attempt - 2);
                std::cout << std::format("\n    Retry {}/{} (waiting {}ms)... ", attempt - 1, maxAttempts, delay.count());
                std::this_thread::sleep_for(delay);
            }

            // Attempt to load
            auto error = AttemptLoad(package);
            if (!error) {
                return {.success = true, .error = std::nullopt, .retryAttempts = attempt - 1};
            }

            // Check if we should retry
            if (!ShouldRetry(*error, attempt)) {
                return {.success = false, .error = std::move(error), .retryAttempts = attempt - 1};
            }
        }

        // Exhausted retries
        return {
            .success = false,
            .error = Error::Permanent(
                ErrorCode::MaxRetriesExceeded,
                "Maximum retry attempts exceeded",
                ErrorCategory::Runtime
            ),
            .retryAttempts = maxAttempts
        };
    }

    std::optional<Error> CheckDependencies(
        PackageInfo package,
        const std::unordered_set<PackageId>& loaded
    ) {
        if (!config->skipDependentsOnFailure || !package->manifest->dependencies || package->manifest->dependencies->empty()) {
            return std::nullopt;
        }

        std::vector<PackageId> missing;

        for (const auto& dependency : *package->manifest->dependencies) {
            const auto& [depName, constraints, optional] = *dependency._impl;
            if (!optional && !loaded.contains(depName)) {
                missing.push_back(depName);
            }
        }

        if (!missing.empty()) {
            return Error::Permanent(
                ErrorCode::MissingDependency,
                std::format("Missing dependencies: [{}]", plg::join(missing, ", ")),
                ErrorCategory::Dependency
            );
        }

        return std::nullopt;
    }

    std::optional<Error> AttemptLoad(PackageInfo package) {
        // Load phase
        std::expected<std::shared_ptr<void>, Error> loadResult;

        if (package->isModule()) {
            loadResult = _impl->moduleLoader->loadModule(package->manifest);
        } else {
            // Get the language module for this plugin
            auto langModule = GetLoadedLanguageModule(*package->requiredLanguageModule);
            if (!langModule) {
                return Error::NonRetryable(
                    ErrorCode::InternalError,
                    "Language module disappeared during initialization",
                    ErrorCategory::System
                );
            }
            loadResult = _impl->pluginLoader->loadPlugin(package->manifest, *langModule);
        }

        if (!loadResult) {
            return loadResult.error();
        }

        // Initialize phase
        std::expected<void, Error> initResult;

        if (package->isModule()) {
            initResult = _impl->moduleLoader->initializeModule(*loadResult.value(), package->manifest);
        } else {
            initResult = _impl->pluginLoader->initializePlugin(*loadResult.value(), package->manifest);
        }

        if (!initResult) {
            auto isTransient = initResult.error().code == ErrorCode::InitializationFailed;
            return isTransient
                ? EnhancedError::Transient(initResult.error().code, initResult.error().message)
                : EnhancedError::NonRetryable(initResult.error().code, initResult.error().message,
                    ErrorCategory::Configuration);
        }

        // Store the loaded package
        if (package->isModule()) {
            _impl->loadedModules[package->id] = true;
        } else {
            _impl->loadedPlugins[package->id] = true;
        }

        return std::nullopt;  // Success
    }

    // ============================================================================
    // Helper methods
    // ============================================================================
    bool ShouldRetry(const Error& error, std::size_t attemptCount) const {
        if (!error.retryable)
            return false;
        if (config->retryPolicy.retryOnlyTransient && error.category != ErrorCategory::Transient)
            return false;
        return attemptCount < config->retryPolicy.maxAttempts;
    }

    std::chrono::milliseconds GetRetryDelay(std::size_t attemptCount) const {
        if (!config->retryPolicy.exponentialBackoff)
            return config->retryPolicy.baseDelay;
        // TODO: clamp
        auto delay = config->retryPolicy.baseDelay * (1 << attemptCount);
        return std::min(delay, config->retryPolicy.maxDelay);
    }

    // ============================================================================
    // Event Handling
    // ============================================================================

    UniqueId Subscribe(EventHandler handler) {
        return eventDistpatcher.Subscribe(std::move(handler));
    }

    void Unsubscribe(UniqueId id) {
        eventDistpatcher.Unsubscribe(id);
    }

    void EmitEvent(const Event& event) {
        eventDistpatcher.Emit(event);
    }
};

Manager::Manager(Plugify& plugify) : _impl(std::make_unique<Impl>(plugify)) {
    // Initialize with default components if not set
    //_impl->packageDiscovery = ManagerFactory::CreateDefaultDiscovery();
    //_impl->packageValidator = ManagerFactory::CreateDefaultValidator();
    _impl->dependencyResolver = ManagerFactory::CreateDefaultResolver();
    //_impl->moduleLoader = ManagerFactory::CreateDefaultModuleLoader();
    //_impl->pluginLoader = ManagerFactory::CreateDefaultPluginLoader();
}

Manager::~Manager() {
	// Users must call terminate() explicitly before destruction
	if (!_impl->loadedPackages.empty()) {
		std::cerr << "WARNING: Manager destroyed without calling terminate()!\\n";
		std::cerr << "         Call terminate() explicitly for proper shutdown.\\n";
		std::cerr << std::format("         {} packages still loaded!\\n", _impl->loadedPackages.size());
	}

	auto _ = Terminate();
}

// ============================================================================
// Dependency Injection
// ============================================================================

/*void Manager::setPackageDiscovery(std::unique_ptr<IPackageDiscovery> discovery) {
    _impl->packageDiscovery = std::move(discovery);
}

void Manager::setPackageValidator(std::unique_ptr<IPackageValidator> validator) {
    _impl->packageValidator = std::move(validator);
}*/

void Manager::SetDependencyResolver(std::unique_ptr<IDependencyResolver> resolver) {
    _impl->dependencyResolver = std::move(resolver);
}

/*void Manager::setModuleLoader(std::unique_ptr<IModuleLoader> loader) {
    _impl->moduleLoader = std::move(loader);
}

void Manager::setPluginLoader(std::unique_ptr<IPluginLoader> loader) {
    _impl->pluginLoader = std::move(loader);
}*/

// ============================================================================
// Discovery Phase
// ============================================================================

/*template <typename T>
Result<std::shared_ptr<T>> ReadManifest(
		const std::shared_ptr<IFileSystem>& fs,
		const fs::path& path,
		PackageType type
) {
	auto json = fs->ReadTextFile(path);
	if (!json)
		return std::unexpected(json.error());

	auto parsed = glz::read_jsonc<std::shared_ptr<T>>(*json);
	if (!parsed)
		return std::unexpected(glz::format_error(parsed.error(), *json));

	auto& manifest = *parsed;
	manifest->location = path;
	manifest->type = type;
	manifest->id = manifest->name;
	return manifest;
}*/

// ============================================================================
// Initialization Sequence
// ============================================================================

State<InitializationState> Manager::Initialize() {
    InitializationState state;
    ReportTimer timer(state);

    _impl->config = _impl->plugify.GetConfig();
    if (!_impl->config) {
        return plg::unexpected(Error::Permanent(
            ErrorCode::ConfigurationMissing,
            "Plugify configuration is not set",
            ErrorCategory::Configuration
        ));
    }

	auto result = DiscoverPackages();
    if (!result) {
        return plg::unexpected(std::move(result.error()));
    }

    std::cout << "Starting plugin system initialization...\n";
    
    // Step 1: Validate all manifests
    state.validationReport = _impl->ManifestValidation();
    if (!state.validationReport.AllPassed() &&
        !_impl->config->partialStartupMode) {
        return plg::unexpected(Error::Permanent(
            ErrorCode::ValidationFailed,
            std::format("{} packages failed validation", state.validationReport.FailureCount()),
            ErrorCategory::Validation
        ));
    }
    
    // Step 2: Resolve dependencies and determine load order
    state.dependencyReport = _impl->ResolveDependencies();
    if (state.dependencyReport.HasBlockingIssues() &&
        !_impl->config->partialStartupMode) {
        return plg::unexpected(Error::Permanent(
            ErrorCode::MissingDependency,
            std::format("Dependency resolution failed: {} blocking issues found", state.dependencyReport.BlockerCount()),
            ErrorCategory::Dependency
        ));
    }
    
    // Step 3: Sort packages by dependency order
    _impl->SortPackagesByDependencies(state.dependencyReport);
    
    // Step 4: Initialize packages in dependency order
    state.initializationReport = _impl->Initialize(state.dependencyReport);

    // Step 5:


    // Print comprehensive summary (can be disabled via config if needed)
    if (_impl->config->printSummary) {  // Assuming we add this config option
        std::cout << "\n" << state.GenerateTextReport() << "\n";
    }
    
    // Determine overall success
    if (_impl->config->partialStartupMode) {
        if (state.initializationReport.SuccessCount() > 0) {
            return state;  // Partial success
        }
    }
    
    if (state.initializationReport.FailureCount() > 0 && !_impl->config->partialStartupMode) {
        return plg::unexpected(Error::Permanent(
            ErrorCode::InitializationFailed,
            std::format("{} packages failed to initialize", state.initializationReport.FailureCount()),
            ErrorCategory::Runtime
        ));
    }

    return state;
}

// Single initialization function for all packages





// Helper functions


// ============================================================================
// Termination Sequence
// ============================================================================
Result<void> Manager::Terminate() {
	// Unload in reverse dependency order for clean shutdown
	// Plugins depend on modules, so unload plugins first
    
	// If we have a valid load order, unload in reverse
	if (_impl->initState.dependencyReport.isLoadOrderValid) {
		const auto& loadOrder = _impl->initState.dependencyReport.loadOrder;
        
		// Reverse order for unloading
		for (auto it = loadOrder.rbegin(); it != loadOrder.rend(); ++it) {
			const auto& id = *it;
            
			// Unload plugin if loaded
			if (auto pluginIt = _impl->loadedPlugins.find(id); 
				pluginIt != _impl->loadedPlugins.end()) {
				EmitEvent({
					.type = EventType::PluginEnded,
					.timestamp = std::chrono::system_clock::now(),
					.packageId = id
				});
				//_impl->pluginLoader->unloadPlugin(std::move(pluginIt->second));
				_impl->loadedPlugins.erase(pluginIt);
			}
            
			// Unload module if loaded
			if (auto moduleIt = _impl->loadedModules.find(id); 
				moduleIt != _impl->loadedModules.end()) {
				//_impl->moduleLoader->unloadModule(std::move(moduleIt->second));
				_impl->loadedModules.erase(moduleIt);
			}
		}
	} else {
		// Fallback: unload all plugins first, then modules
		for (auto& [id, plugin] : _impl->loadedPlugins) {
			EmitEvent({
				.type = EventType::PluginEnded,
				.timestamp = std::chrono::system_clock::now(),
				.packageId = id
			});
			//_impl->pluginLoader->unloadPlugin(std::move(plugin));
		}
		_impl->loadedPlugins.clear();
        
		for (auto& [id, module] : _impl->loadedModules) {
			//_impl->moduleLoader->unloadModule(std::move(module));
		}
		_impl->loadedModules.clear();
	}

	return {};
}

// ============================================================================
// Query Methods
// ============================================================================

ModuleInfo Manager::GetModule(std::string_view moduleId) const {
    auto it = _impl->modules.find(moduleId);
    if (it != _impl->modules.end()) {
        return it->second;
    }
    return {};
}

PluginInfo Manager::GetPlugin(std::string_view pluginId) const {
    auto it = _impl->plugins.find(pluginId);
    if (it != _impl->plugins.end()) {
        return it->second;
    }
    return {};
}

std::vector<ModuleInfo> Manager::GetModules() const {
    std::vector<ModuleInfo> result;
    result.reserve(_impl->modules.size());
    
    for (const auto& [id, info] : _impl->modules) {
    	result.push_back(info);
    }
    
    return result;
}

std::vector<PluginInfo> Manager::GetPlugins() const {
    std::vector<PluginInfo> result;
    result.reserve(_impl->plugins.size());
    
    for (const auto& [id, info] : _impl->plugins) {
    	result.push_back(info);
    }
    
    return result;
}

std::vector<ModuleInfo> Manager::GetModules(PackageState state) const {
    std::vector<ModuleInfo> result;
    result.reserve(_impl->modules.size());
    
    for (const auto& [id, info] : _impl->modules) {
    	if (info->state == state) {
    		result.push_back(info);
    	}
    }
    
    return result;
}

std::vector<PluginInfo> Manager::GetPlugins(PackageState state) const {
    std::vector<PluginInfo> result;
    result.reserve(_impl->plugins.size());
    
    for (const auto& [id, info] : _impl->plugins) {
    	if (info->state == state) {
    		result.push_back(info);
    	}
    }
    
    return result;
}

bool Manager::IsModuleLoaded(std::string_view moduleId) const {
    return _impl->loadedModules.contains(moduleId);
}

bool Manager::IsPluginLoaded(std::string_view pluginId) const {
    return _impl->loadedPlugins.contains(pluginId);
}



// ============================================================================
// Helper Methods
// ============================================================================

std::vector<PackageId> Manager::GetPluginsForModule(std::string_view moduleId) const {
    auto it = _impl->moduleToPluginsMap.find(moduleId);
    if (it != _impl->moduleToPluginsMap.end()) {
        return it->second;
    }
    return {};
}

std::unique_ptr<IDependencyResolver> ManagerFactory::CreateDefaultResolver() {
    return std::make_unique<LibsolvDependencyResolver>();
}