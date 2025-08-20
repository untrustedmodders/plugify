#include "plugify/core/manager.hpp"
#include "plugify/core/package.hpp"


#include "json.cpp"
#include "module_manifest->hpp"
#include "plugify/_/plugin_manifest_handle.hpp"
#include "plugify/core/plugify.hpp"
#include "plugin_manifest->hpp"

#include "plugify/core/module.hpp"
#include "plugify/core/plugin.hpp"

#include "plugify/_/dependency_handle.hpp"
#include "plugify/core/manager.hpp"
#include <../../include/plugify/core/plugify.hpp"

using namespace plugify;

struct Manager::Impl {
	Plugify& plugify;
	// Configuration
	Config& config;

	// Discovered packages
	std::unordered_map<PackageId, ModuleInfo, plg::string_hash, std::equal_to<>> modules;
	std::unordered_map<PackageId, PluginInfo, plg::string_hash, std::equal_to<>> plugins;

	// Loaded instances
	std::unordered_map<PackageId, std::unique_ptr<Module>, plg::string_hash, std::equal_to<>> loadedModules;
	std::unordered_map<PackageId, std::unique_ptr<Plugin>, plg::string_hash, std::equal_to<>> loadedPlugins;

	// Dependency injection components
	std::unique_ptr<IPackageValidator> packageDiscovery;
	std::unique_ptr<IPackageValidator> packageValidator;
	std::unique_ptr<IModuleLoader> dependencyResolver;
	std::unique_ptr<IModuleLoader> moduleLoader;
	std::unique_ptr<IPluginLoader> pluginLoader;

	// Comprehensive initialization tracking
	struct InitializationState {
		ValidationReport validationReport;
		DependencyReport dependencyReport;
		InitializationReport initReport;
		DateTime startTime;
		DateTime endTime;
	} initState;

	// Event handling
	std::unordered_map<UniqueId, EventHandler> eventHandlers;
	UniqueId nextSubscriptionId = 1;
	mutable std::mutex eventMutex;

	// Dependency tracking
	std::unordered_map<PackageId, std::vector<PackageId>> pluginToModuleMap;
	std::unordered_map<PackageId, std::vector<PackageId>> moduleToPluginsMap;
};

Manager::Manager(Plugify& plugify) : _impl(std::make_unique<Impl>(plugify)) {
    _impl->config = plugify.GetConfig();
    
    // Initialize with default components if not set
    _impl->packageDiscovery = ManagerFactory::createDefaultDiscovery();
    _impl->packageValidator = ManagerFactory::createDefaultValidator();
    _impl->dependencyResolver = ManagerFactory::createDefaultResolver();
    _impl->moduleLoader = ManagerFactory::createDefaultModuleLoader();
    _impl->pluginLoader = ManagerFactory::createDefaultPluginLoader();
}

Manager::~Manager() {
    // Unload plugins before modules to maintain proper shutdown order
    for (auto& [id, plugin] : _impl->loadedPlugins | std::views::reverse) {
        EmitEvent({
            .type = EventType::PluginEnded,
            .timestamp = DateTime::Now(),
            .packageId = id
        });
        _impl->pluginLoader->unloadPlugin(std::move(plugin));
    }
    _impl->loadedPlugins.clear();
    
    // Then unload modules
    for (auto& [id, module] : _impl->loadedModules | std::views::reverse) {

        _impl->moduleLoader->unloadModule(std::move(module));
    }
    _impl->loadedModules.clear();
}

// ============================================================================
// Dependency Injection
// ============================================================================

void Manager::setPackageDiscovery(std::unique_ptr<IPackageValidator> discovery) {
    _impl->packageDiscovery = std::move(discovery);
}

void Manager::setPackageValidator(std::unique_ptr<IPackageValidator> validator) {
    _impl->packageValidator = std::move(validator);
}

void Manager::setDependencyResolver(std::unique_ptr<IModuleLoader> resolver) {
    _impl->dependencyResolver = std::move(resolver);
}

void Manager::setModuleLoader(std::unique_ptr<IModuleLoader> loader) {
    _impl->moduleLoader = std::move(loader);
}

void Manager::setPluginLoader(std::unique_ptr<IPluginLoader> loader) {
    _impl->pluginLoader = std::move(loader);
}

// ============================================================================
// Discovery Phase
// ============================================================================

Result<void> Manager::DiscoverPackages(
    std::span<const std::filesystem::path> searchPaths
) {
    // Combine provided paths with plugify.configured paths
    std::vector<std::filesystem::path> allPaths;
    allPaths.reserve(searchPaths.size() + _impl->config.searchPaths.size());
    allPaths.insert(allPaths.end(), searchPaths.begin(), searchPaths.end());
    allPaths.insert(allPaths.end(), 
                    _impl->config.searchPaths.begin(), 
                    _impl->config.searchPaths.end());

    // Discover modules first
	{
		auto moduleResult = _impl->packageDiscovery->discoverModules(allPaths);
    	if (!moduleResult) {
    		return plg::unexpected(moduleResult.error());
    	}

    	// Store discovered modules
    	for (auto& moduleInfo : moduleResult.value()) {
    		const auto& id = moduleInfo.manifest->id;

    		// Apply whitelist/blacklist filtering
    		if (_impl->config.whitelistedPackages &&
				!std::ranges::contains(*_impl->config.whitelistedPackages, id)) {
    			continue;
				}

    		if (_impl->config.blacklistedPackages &&
				std::ranges::contains(*_impl->config.blacklistedPackages, id)) {
    			UpdatePackageState(id, PackageState::Disabled);
    			continue;
				}

    		EmitEvent({
				.type = EventType::ModuleDiscovered,
				.timestamp = DateTime::Now(),
				.packageId = id
			});

    		_impl->modules[id] = std::move(moduleInfo);
    	}
	}
    
    // Discover plugins
	{
		auto pluginResult = _impl->packageDiscovery->discoverPlugins(allPaths);
    	if (!pluginResult) {
    		return plg::unexpected(pluginResult.error());
    	}

    	// Store discovered plugins and map to required language modules
    	for (auto& pluginInfo : pluginResult.value()) {
    		const auto& id = pluginInfo.manifest->id;

    		// Apply filtering
    		if (_impl->config.whitelistedPackages &&
				!std::ranges::contains(*_impl->config.whitelistedPackages, id)) {
    			continue;
				}

    		if (_impl->config.blacklistedPackages &&
				std::ranges::contains(*_impl->config.blacklistedPackages, id)) {
    			UpdatePackageState(id, PackageState::Disabled);
    			continue;
				}

    		EmitEvent({
				.type = EventType::PluginDiscovered,
				.timestamp = DateTime::Now(),
				.packageId = id
			});

    		_impl->plugins[id] = std::move(pluginInfo);
    	}
	}
}

// ============================================================================
// Initialization Sequence
// ============================================================================

Result<InitializationReport> Manager::Initialize() {
    _impl->initState = {};
    _impl->initState.startTime = DateTime::Now();
    
    std::cout << "Starting plugin system initialization...\n";
    
    // Step 1: Validate all manifests
    _impl->initState.validationReport = ValidateAllManifests();
    if (!_impl->initState.validationReport.allPassed() && !_impl->config.partialStartupMode) {
        auto error = EnhancedError::NonRetryable(
            ErrorCode::ValidationFailed,
            std::format("{} packages failed validation", 
                       _impl->initState.validationReport.failureCount()),
            ErrorCategory::Validation
        );
        return plg::unexpected(std::move(error));
    }
    
    // Step 2: Resolve dependencies and determine load order
    _impl->initState.dependencyReport = ResolveDependencies();
    if (_impl->initState.dependencyReport.hasBlockingIssues() && !_impl->config.partialStartupMode) {
        auto error = EnhancedError::NonRetryable(
            ErrorCode::MissingDependency,
            std::format("Dependency resolution failed: {} blocking issues found",
                       std::ranges::count_if(_impl->initState.dependencyReport.resolutions,
                           [](const auto& r) { return r.blockerCount() > 0; })),
            ErrorCategory::Dependency
        );
        return plg::unexpected(std::move(error));
    }
    
    // Step 3: Sort packages by dependency order
    _impl->sortPackagesByDependencies(_impl->initState.dependencyReport);
    
    // Step 4: Initialize language modules in dependency order
    auto moduleReport = InitializeModules();
    _impl->initState.initReport.moduleInits = moduleReport.moduleInits;
    
    // Step 5: Initialize plugins in dependency order
    auto pluginReport = InitializePlugins();
    _impl->initState.initReport.pluginInits = pluginReport.pluginInits;
    
    // Calculate total time
    _impl->initState.endTime = DateTime::Now();
    _impl->initState.initReport.totalTime = _impl->initState.endTime - _impl->initState.startTime;
    
    // Print comprehensive summary
    printInitializationSummary(_impl->initState.initReport);
    
    // Determine overall success
    if (_impl->config.partialStartupMode) {
        if (_impl->initState.initReport.successCount() > 0) {
            return _impl->initState.initReport;  // Partial success
        }
    }
    
    if (_impl->initState.initReport.failureCount() > 0 && !_impl->config.partialStartupMode) {
        auto error = EnhancedError::NonRetryable(
            ErrorCode::InitializationFailed,
            std::format("{} packages failed to initialize", 
                       _impl->initState.initReport.failureCount()),
            ErrorCategory::Runtime
        );
        return plg::unexpected(error);
    }
    
    return _impl->initState.initReport;
}

ValidationReport Manager::ValidateAllManifests() {
	ValidationReport report;

	// Validate module manifests
	{
		report.moduleResults.reserve(_impl->modules.size());

		for (auto& [id, moduleInfo] : _impl->modules) {
			if (moduleInfo.state == PackageState::Disabled) continue;

			ValidationReport::PackageValidation validation{.id = id};

			auto result = _impl->packageValidator->validateManifest(moduleInfo.manifest);
			if (!result) {
				validation.passed = false;
				validation.error = EnhancedError::NonRetryable(
					result.error().code,
					result.error().message,
					ErrorCategory::Validation
				);

				moduleInfo.state = PackageState::Error;
				moduleInfo.lastError = result.error();

				std::cerr << std::format("Module '{}' validation failed: {}\n",
										 id, result.error().message);
			} else {
				validation.passed = true;
				moduleInfo.state = PackageState::Validated;
				
				EmitEvent({
					.type = EventType::ModuleValidated,
					.timestamp = DateTime::Now(),
					.packageId = id
				});
			}

			report.moduleResults.emplace_back(std::move(validation));
		}
	}

	// Validate plugin manifests
	{
		report.pluginResults.reserve(_impl->modules.size());

		for (auto& [id, pluginInfo] : _impl->plugins) {
			if (pluginInfo.state == PackageState::Disabled) continue;

			ValidationReport::PackageValidation validation{.id = id};

			auto result = _impl->packageValidator->validateManifest(pluginInfo.manifest);
			if (!result) {
				validation.passed = false;
				validation.error = EnhancedError::NonRetryable(
					result.error().code,
					result.error().message,
					ErrorCategory::Validation
				);

				pluginInfo.state = PackageState::Error;
				pluginInfo.lastError = result.error();

				std::cerr << std::format("Plugin '{}' validation failed: {}\n",
										 id, result.error().message);
			} else {
				validation.passed = true;
				pluginInfo.state = PackageState::Validated;
				
				EmitEvent({
					.type = EventType::PluginValidated,
					.timestamp = DateTime::Now(),
					.packageId = id
				});
			}

			report.pluginResults.emplace_back(std::move(validation));
		}
	}

    return report;
}

DependencyReport Manager::ResolveDependencies() {
#if 0
	// Collect all validated packages
	std::vector<PluginInfo> validPlugins;
	std::vector<ModuleInfo> validModules;
    
	for (const auto& [id, info] : _impl->plugins) {
		if (info.state == PackageState::Validated) {
			validPlugins.push_back(info);
		}
	}
    
	for (const auto& [id, info] : _impl->modules) {
		if (info.state == PackageState::Validated) {
			validModules.push_back(info);
		}
	}
#endif

	// Delegate all complex dependency resolution to the resolver
	auto resolution = _impl->dependencyResolver->resolve(validPlugins, validModules);
    
	if (!resolution) {
		// Create a minimal report for the error case
		DependencyReport errorReport;
		// The resolver should provide detailed error information
		std::cerr << "Dependency resolution failed: " << resolution.error().message << "\n";
		return errorReport;
	}
    
	// Log summary if there are issues
	if (resolution->hasBlockingIssues() || !resolution->circularDependencies.empty()) {
		std::cout << "\n" << resolution->generateTextReport() << "\n";
	} else {
		std::cout << std::format("Dependencies resolved successfully. Load order determined for {} packages.\n",
								 resolution->loadOrder.size());
	}
    
	return resolution.value();
}

InitializationReport Manager::InitializeModules() {
	InitializationReport report;
    
    std::cout << "\n=== Initializing Language Modules ===\n";
    
    // Track successfully loaded modules for dependency checking
    std::unordered_set<PackageId> successfullyLoaded;
    
    // Use sorted order if available
    const auto& modulesToInit = _impl->config.respectDependencyOrder && !_impl->sortedModules.empty()
                               ? _impl->sortedModules
                               : [this]() {
                                     std::vector<ModuleInfo> unsorted;
                                     for (const auto& [id, info] : _impl->modules) {
                                         if (info.state == PackageState::Validated) {
                                             unsorted.push_back(info);
                                         }
                                     }
                                     return unsorted;
                                 }();
    
    for (const auto& moduleInfo : modulesToInit) {
        const auto& id = moduleInfo.manifest->id;
        
        // Update the actual module info in the map
        auto& actualModuleInfo = _impl->modules[id];
        
        auto initStart = DateTime::Now();
        InitializationReport::PackageInit init{.id = id};
        
        // Check if all dependencies are loaded (if configured to do so)
        bool allDependenciesLoaded = true;
        std::vector<PackageId> failedDependencies;
        
        if (_impl->config.skipDependentsOnFailure) {
            for (const auto& dep : actualModuleInfo.manifest->dependencies) {
                // Skip optional dependencies if configured
                if (dep.optional.value_or(false) && !_impl->config.failOnMissingDependencies) {
                    continue;
                }
                
                // Check if dependency is loaded
                if (!successfullyLoaded.contains(dep.name) && 
                    !_impl->loadedModules.contains(dep.name) &&
                    !_impl->loadedPlugins.contains(dep.name)) {
                    
                    allDependenciesLoaded = false;
                    failedDependencies.push_back(dep.name);
                }
            }
        }
        
        if (!allDependenciesLoaded) {
            // Skip this module due to failed dependencies
            actualModuleInfo.state = PackageState::Error;
            auto error = EnhancedError::NonRetryable(
                ErrorCode::MissingDependency,
                std::format("Cannot load module '{}' because dependencies failed to load: [{}]",
                           id, 
                           std::ranges::fold_left(failedDependencies, std::string{},
                               [](std::string acc, const std::string& dep) {
                                   return acc.empty() ? dep : acc + ", " + dep;
                               })),
                ErrorCategory::Dependency
            );
            actualModuleInfo.lastError = error;
            
            init.finalState = PackageState::Error;
            init.error = error;
            init.loadTime = DateTime::Now();
            report.moduleInits.emplace_back(std::move(init));
            
            EmitEvent({
                .type = EventType::ModuleFailed,
                .timestamp = DateTime::Now(),
                .packageId = id,
                .error = error
            });
            
            std::cerr << std::format("  ⚠️  Module '{}' skipped: dependencies not available [{}]\n", 
                                     id,
                                     std::ranges::fold_left(failedDependencies, std::string{},
                                         [](std::string acc, const std::string& dep) {
                                             return acc.empty() ? dep : acc + ", " + dep;
                                         }));
            
            // Continue to next module unless strict mode
            if (!_impl->config.partialStartupMode) {
                break;
            }
            continue;
        }
        
        actualModuleInfo.state = PackageState::Initializing;
        
        EmitEvent({
            .type = EventType::ModuleLoading,
            .timestamp = DateTime::Now(),
            .packageId = id
        });
        
        // Attempt to load with intelligent retry
        bool loadSuccess = false;
        std::size_t attemptCount = 0;
        EnhancedError lastError;
        
        while (!loadSuccess) {
            attemptCount++;
            init.retryAttempts = attemptCount - 1;
            
            if (attemptCount > 1) {
                auto delay = _impl->getRetryDelay(attemptCount - 2);
                std::cout << std::format("  Retry {}/{} for module '{}' (waiting {}ms)\n", 
                                        attemptCount - 1, 
                                        _impl->config.retryPolicy.maxAttempts,
                                        id, delay.count());
                std::this_thread::sleep_for(delay);
            }
            
            // Load the module
            auto moduleResult = _impl->moduleLoader->loadModule(actualModuleInfo.manifest);
            if (!moduleResult) {
                lastError = EnhancedError::Transient(
                    moduleResult.error().code,
                    moduleResult.error().message
                );
                
                if (!_impl->shouldRetry(lastError, attemptCount)) {
                    break;
                }
                continue;
            }
            
            // Initialize the module
            auto initResult = _impl->moduleLoader->initializeModule(
                *moduleResult.value(), actualModuleInfo.manifest);
            
            if (!initResult) {
                if (initResult.error().code == ErrorCode::InitializationFailed) {
                    lastError = EnhancedError::Transient(
                        initResult.error().code,
                        initResult.error().message
                    );
                } else {
                    lastError = EnhancedError::NonRetryable(
                        initResult.error().code,
                        initResult.error().message,
                        ErrorCategory::Configuration
                    );
                }
                
                if (!_impl->shouldRetry(lastError, attemptCount)) {
                    break;
                }
                continue;
            }
            
            // Success!
            _impl->loadedModules[id] = std::move(moduleResult.value());
            actualModuleInfo.state = PackageState::Ready;
            loadSuccess = true;
            init.finalState = PackageState::Ready;
            successfullyLoaded.insert(id);  // Mark as successfully loaded
            
            EmitEvent({
                .type = EventType::ModuleLoaded,
                .timestamp = DateTime::Now(),
                .packageId = id
            });
            
            std::cout << std::format("  ✓ Module '{}' loaded successfully", id);
            if (attemptCount > 1) {
                std::cout << std::format(" (after {} retries)", attemptCount - 1);
            }
            if (!actualModuleInfo.manifest->dependencies.empty()) {
                std::cout << std::format(" [deps: {}]",
                    std::ranges::fold_left(
                        actualModuleInfo.manifest->dependencies | std::views::transform([](const auto& d) { return d.name; }),
                        std::string{},
                        [](const std::string& acc, const std::string& dep) {
                            return acc.empty() ? dep : acc + ", " + dep;
                        }));
            }
            std::cout << "\n";
        }
        
        if (!loadSuccess) {
            actualModuleInfo.state = PackageState::Error;
            actualModuleInfo.lastError = lastError;
            init.finalState = PackageState::Error;
            init.error = lastError;
            
            EmitEvent({
                .type = EventType::ModuleFailed,
                .timestamp = DateTime::Now(),
                .packageId = id,
                .error = lastError
            });
            
            std::cerr << std::format("  ✗ Failed to load module '{}': {} ({})\n",
                                     id, lastError.message,
                                     lastError.isRetryable ? "gave up after retries" : "non-retryable");
            
            // Check if any packages depend on this failed module
            if (_impl->initState.dependencyReport.reverseDependencyGraph.contains(id)) {
                const auto& dependents = _impl->initState.dependencyReport.reverseDependencyGraph.at(id);
                if (!dependents.empty()) {
                    std::cerr << std::format("     ⚠️  This will prevent loading: [{}]\n",
                        std::ranges::fold_left(dependents, std::string{},
                            [](const std::string& acc, const std::string& dep) {
                                return acc.empty() ? dep : acc + ", " + dep;
                            }));
                }
            }
        }
        
        auto initEnd = DateTime::Now();
        init.loadTime = initEnd - initStart;
        report.moduleInits.emplace_back(std::move(init));
    }
    
    return report;
}

InitializationReport Manager::InitializePlugins() {
	InitializationReport report;
    
    std::cout << "\n=== Initializing Plugins ===\n";
    
    // Track successfully loaded plugins for dependency checking
    std::unordered_set<PackageId> successfullyLoaded;
    
    // Add all successfully loaded modules to the set
    for (const auto& [id, _] : _impl->loadedModules) {
        successfullyLoaded.insert(id);
    }
    
    // Use sorted order if available
    const auto& pluginsToInit = _impl->config.respectDependencyOrder && !_impl->sortedPlugins.empty()
                               ? _impl->sortedPlugins
                               : [this]() {
                                     std::vector<PluginInfo> unsorted;
                                     for (const auto& [id, info] : _impl->plugins) {
                                         if (info.state == PackageState::Validated) {
                                             unsorted.push_back(info);
                                         }
                                     }
                                     return unsorted;
                                 }();
    
    for (const auto& pluginInfo : pluginsToInit) {
        const auto& id = pluginInfo.manifest->id;
        
        // Update the actual plugin info in the map
        auto& actualPluginInfo = _impl->plugins[id];
        
        auto initStart = DateTime::Now();
        InitializationReport::PackageInit init{.id = id};
        
        // First check if all dependencies are loaded (if configured to do so)
        bool allDependenciesLoaded = true;
        std::vector<PackageId> failedDependencies;
        
        if (_impl->config.skipDependentsOnFailure) {
            for (const auto& dep : actualPluginInfo.manifest->dependencies) {
                // Skip optional dependencies if configured
                if (dep.optional.value_or(false) && !_impl->config.failOnMissingDependencies) {
                    continue;
                }
                
                // Check if dependency is loaded
                if (!successfullyLoaded.contains(dep.name) && 
                    !_impl->loadedPlugins.contains(dep.name)) {
                    
                    allDependenciesLoaded = false;
                    failedDependencies.push_back(dep.name);
                }
            }
        }
        
        if (!allDependenciesLoaded) {
            // Skip this plugin due to failed dependencies
            actualPluginInfo.state = PackageState::Error;
            auto error = EnhancedError::NonRetryable(
                ErrorCode::MissingDependency,
                std::format("Cannot load plugin '{}' because dependencies failed to load: [{}]",
                           id,
                           std::ranges::fold_left(failedDependencies, std::string{},
                               [](std::string acc, const std::string& dep) {
                                   return acc.empty() ? dep : acc + ", " + dep;
                               })),
                ErrorCategory::Dependency
            );
            actualPluginInfo.lastError = error;
            
            init.finalState = PackageState::Error;
            init.error = error;
            init.loadTime = DateTime::Now();
            report.pluginInits.emplace_back(std::move(init));
            
            EmitEvent({
                .type = EventType::PluginFailed,
                .timestamp = DateTime::Now(),
                .packageId = id,
                .error = error
            });
            
            std::cerr << std::format("  ⚠️  Plugin '{}' skipped: dependencies not available [{}]\n",
                                     id,
                                     std::ranges::fold_left(failedDependencies, std::string{},
                                         [](std::string acc, const std::string& dep) {
                                             return acc.empty() ? dep : acc + ", " + dep;
                                         }));
            
            // Continue to next plugin unless strict mode
            if (!_impl->config.partialStartupMode) {
                break;
            }
            continue;
        }
        
        // Check if required language module is loaded
        auto moduleIds = _impl->pluginToModuleMap.find(id);
        if (moduleIds == _impl->pluginToModuleMap.end() || moduleIds->second.empty()) {
            actualPluginInfo.state = PackageState::Error;
            auto error = EnhancedError::NonRetryable(
                ErrorCode::LanguageModuleNotLoaded,
                std::format("No language module available for plugin '{}'", id),
                ErrorCategory::Dependency
            );
            actualPluginInfo.lastError = error;
            
            init.finalState = PackageState::Error;
            init.error = error;
            init.loadTime = DateTime::Now();
            report.pluginInits.emplace_back(std::move(init));
            
            std::cerr << std::format("  ✗ Plugin '{}' skipped: no language module\n", id);
            continue;
        }
        
        // Find the loaded module for this plugin's language
        Module* languageModule = nullptr;
        std::string usedModuleId;
        
        for (const auto& moduleId : moduleIds->second) {
            auto it = _impl->loadedModules.find(moduleId);
            if (it != _impl->loadedModules.end()) {
                languageModule = it->second.get();
                usedModuleId = moduleId;
                break;
            }
        }
        
        if (!languageModule) {
            actualPluginInfo.state = PackageState::Error;
            auto error = EnhancedError::NonRetryable(
                ErrorCode::LanguageModuleNotLoaded,
                std::format("Language module not loaded for plugin '{}' (required module failed to initialize)", id),
                ErrorCategory::Dependency
            );
            actualPluginInfo.lastError = error;
            
            init.finalState = PackageState::Error;
            init.error = error;
            init.loadTime = DateTime::Now();
            report.pluginInits.emplace_back(std::move(init));
            
            std::cerr << std::format("  ⚠️  Plugin '{}' skipped: language module failed to load\n", id);
            
            // Show which language module was needed
            if (!moduleIds->second.empty()) {
                std::cerr << std::format("     Required module: '{}'\n", moduleIds->second[0]);
            }
            
            continue;
        }
        
        actualPluginInfo.state = PackageState::Initializing;
        
        EmitEvent({
            .type = EventType::PluginLoading,
            .timestamp = DateTime::Now(),
            .packageId = id
        });
        
        // Attempt to load with intelligent retry
        bool loadSuccess = false;
        std::size_t attemptCount = 0;
        EnhancedError lastError;
        
        while (!loadSuccess) {
            attemptCount++;
            init.retryAttempts = attemptCount - 1;
            
            if (attemptCount > 1) {
                auto delay = _impl->getRetryDelay(attemptCount - 2);
                std::cout << std::format("  Retry {}/{} for plugin '{}' (waiting {}ms)\n",
                                        attemptCount - 1,
                                        _impl->config.retryPolicy.maxAttempts,
                                        id, delay.count());
                std::this_thread::sleep_for(delay);
            }
            
            // Load the plugin through its language module
            auto pluginResult = _impl->pluginLoader->loadPlugin(
                actualPluginInfo.manifest, *languageModule);
            
            if (!pluginResult) {
                lastError = EnhancedError::Transient(
                    pluginResult.error().code,
                    pluginResult.error().message
                );
                
                if (!_impl->shouldRetry(lastError, attemptCount)) {
                    break;
                }
                continue;
            }
            
            // Initialize the plugin
            auto initResult = _impl->pluginLoader->initializePlugin(
                *pluginResult.value(), actualPluginInfo.manifest);
            
            if (!initResult) {
                if (initResult.error().code == ErrorCode::InitializationFailed) {
                    lastError = EnhancedError::Transient(
                        initResult.error().code,
                        initResult.error().message
                    );
                } else {
                    lastError = EnhancedError::NonRetryable(
                        initResult.error().code,
                        initResult.error().message,
                        ErrorCategory::Configuration
                    );
                }
                
                if (!_impl->shouldRetry(lastError, attemptCount)) {
                    break;
                }
                continue;
            }
            
            // Success!
            _impl->loadedPlugins[id] = std::move(pluginResult.value());
            actualPluginInfo.state = PackageState::Started;
            loadSuccess = true;
            init.finalState = PackageState::Started;
            successfullyLoaded.insert(id);  // Mark as successfully loaded
            
            EmitEvent({
                .type = EventType::PluginLoaded,
                .timestamp = DateTime::Now(),
                .packageId = id
            });

			// TODO

            EmitEvent({
                .type = EventType::PluginStarted,
                .timestamp = DateTime::Now(),
                .packageId = id
            });
            
            std::cout << std::format("  ✓ Plugin '{}' loaded successfully (module: '{}')", 
                                     id, usedModuleId);
            if (attemptCount > 1) {
                std::cout << std::format(" [{}  retries]", attemptCount - 1);
            }
            if (!actualPluginInfo.manifest->dependencies.empty()) {
                std::cout << std::format(" [deps: {}]",
                    std::ranges::fold_left(
                        actualPluginInfo.manifest->dependencies | std::views::transform([](const auto& d) { return d.name; }),
                        std::string{},
                        [](std::string acc, const std::string& dep) {
                            return acc.empty() ? dep : acc + ", " + dep;
                        }));
            }
            std::cout << "\n";
        }
        
        if (!loadSuccess) {
            actualPluginInfo.state = PackageState::Error;
            actualPluginInfo.lastError = lastError;
            init.finalState = PackageState::Error;
            init.error = lastError;
            
            EmitEvent({
                .type = EventType::PluginFailed,
                .timestamp = DateTime::Now(),
                .packageId = id,
                .error = lastError
            });
            
            std::cerr << std::format("  ✗ Failed to load plugin '{}': {} ({})\n",
                                     id, lastError.message,
                                     lastError.isRetryable ? "gave up after retries" : "non-retryable");
            
            // Check if any packages depend on this failed plugin
            if (_impl->initState.dependencyReport.reverseDependencyGraph.contains(id)) {
                const auto& dependents = _impl->initState.dependencyReport.reverseDependencyGraph.at(id);
                if (!dependents.empty()) {
                    std::cerr << std::format("     ⚠️  This will prevent loading: [{}]\n",
                        std::ranges::fold_left(dependents, std::string{},
                            [](const std::string& acc, const std::string& dep) {
                                return acc.empty() ? dep : acc + ", " + dep;
                            }));
                }
            }
        }
        
        auto initEnd = DateTime::Now();
        init.loadTime = initEnd - initStart;
        report.pluginInits.emplace_back(std::move(init));
    }
    
    // Final cascade check - mark any packages as skipped if their dependencies failed
    std::size_t cascadeSkipped = 0;
    for (const auto& [id, info] : _impl->plugins) {
        if (info->state == PackageState::Validated && !successfullyLoaded.contains(id)) {
            // This plugin was never attempted due to cascade failure
            cascadeSkipped++;
        }
    }
    
    if (cascadeSkipped > 0) {
        std::cout << std::format("\n  ℹ️  {} additional plugins were not loaded due to dependency cascade failures\n",
                                 cascadeSkipped);
    }
    
    return report;
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

bool Manager::IsModuleLoaded(std::string_view moduleId) const {
    return _impl->loadedModules.contains(moduleId);
}

bool Manager::IsPluginLoaded(std::string_view pluginId) const {
    return _impl->loadedPlugins.contains(pluginId);
}

// ============================================================================
// Event Handling
// ============================================================================

UniqueId Manager::Subscribe(EventHandler handler) {
    std::lock_guard lock(_impl->eventMutex);
    auto id = _impl->nextSubscriptionId++;
    _impl->eventHandlers.emplace(id, std::move(handler));
}

void Manager::Unsubscribe(UniqueId token) {
    std::lock_guard lock(_impl->eventMutex);
	_impl->eventHandlers.erase(token);
}

void Manager::EmitEvent(Event event) {
	std::vector<EventHandler> handlersCopy;
	{
		std::lock_guard lock(_impl->eventMutex);
		handlersCopy.reserve(_impl->eventHandlers.size());
		for (auto& [_, handler] : _impl->eventHandlers) {
			handlersCopy.push_back(handler);
		}
	}
	// Call outside lock (avoids blocking all subscribers)
	for (auto& handler : handlersCopy) {
		handler(event);
	}
}

// ============================================================================
// Helper Methods
// ============================================================================

void Manager::UpdatePackageState(const PackageId& id, PackageState newState) {
    if (auto itModule = _impl->modules.find(id); itModule != _impl->modules.end()) {
        itModule->second->state = newState;
    } else if (auto itPlugin = _impl->plugins.find(id); itPlugin != _impl->plugins.end()) {
        itPlugin->second->state = newState;
    }
}

std::vector<PackageId> Manager::GetPluginsForModule(std::string_view moduleId) const {
    auto it = _impl->moduleToPluginsMap.find(std::string(moduleId));
    if (it != _impl->moduleToPluginsMap.end()) {
        return it->second;
    }
    return {};
}

Result<void> Manager::HandleInitializationError(const PackageId& id, const Error& error) {
    UpdatePackageState(id, PackageState::Error);
    
    // Retry logic if plugify.configured
    if (_impl->config.maxInitializationRetries > 0) {
        // Implementation would track retry counts and attempt re-initialization
        // For brevity, returning the error here
    }
    
    return plg::unexpected(error);
}

bool Manager::IsAllowed(const std::string& id, bool isModule) const {
	const auto& whitelist = isModule ? _impl->config.moduleWhitelist : _impl->config.pluginWhitelist;
	const auto& blacklist = isModule ? _impl->config.moduleBlacklist : _impl->config.pluginBlacklist;
	return (whitelist.empty() || whitelist.contains(id)) && !blacklist.contains(id);
}