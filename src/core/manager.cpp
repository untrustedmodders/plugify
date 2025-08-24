#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "plugify/core/config.hpp"
#include "plugify/core/dependency_resolver.hpp"
#include "plugify/core/file_system.hpp"
#include "plugify/core/manifest.hpp"
#include "plugify/core/manifest_parser.hpp"
#include "plugify/core/package.hpp"
#include "plugify/core/plugify.hpp"
#include "plugify/core/thread_pool.hpp"

#include "asm/defer.hpp"
#include "plg/thread_pool.hpp"

using namespace plugify;

// Package Manager Implementation with Thread Pool for Discovery/Validation
// Sequential Loading/Starting on Main Thread for Simplicity
//
// Design Philosophy:
// - Parallel: Discovery and Validation (I/O bound, no runtime dependencies)
// - Sequential: Loading and Starting (respects dependencies, avoids threading issues)

struct PhaseStatistics {
    std::atomic<size_t> success{0};
    std::atomic<size_t> failed{0};
    std::chrono::milliseconds time{0};

    void AddSuccess(size_t op = 1) noexcept {
        success.fetch_add(op, std::memory_order_relaxed);
    }

    void AddFailed(size_t op = 1) noexcept {
        failed.fetch_add(op, std::memory_order_relaxed);
    }

    void SetTime(std::chrono::milliseconds t) noexcept {
        time = t;
    }

    size_t Total() const noexcept {
        return success.load(std::memory_order_relaxed) + failed.load(std::memory_order_relaxed);
    }

    float SuccessRate() const noexcept {
        auto total = Total();
        return total > 0 ? (100.0f * success.load()) / total : 0.0f;
    }

    std::string GetText(std::string_view name) const {
        return std::format("{:<12} ✓{:<6} ✗{:<6} {}ms ({:.1f}% success)",
            name, success.load(), failed.load(), time.count(), SuccessRate());
    }
};

// Initialization Statistics
struct InitializationState {
    PhaseStatistics discovery;
    PhaseStatistics parsing;
    PhaseStatistics resolution;
    PhaseStatistics load;
    PhaseStatistics start;
    PhaseStatistics update;

    std::deque<std::pair<std::string, std::string>> errorLog;
    std::deque<std::pair<std::string, std::string>> warningLog;
    mutable std::mutex logMutex;

    void AddError(std::string context, std::string message) {
        std::scoped_lock<std::mutex> lock(mutex);
        errorLog.emplace_back(std::move(context), std::move(message));
    }
    void AddWarning(std::string context, std::string message) {
        std::scoped_lock<std::mutex> lock(mutex);
        warningLog.emplace_back(std::move(context), std::move(message));
    }

    [[nodiscard]] std::chrono::milliseconds TotalTime() const noexcept {
        return discovery.time + parsing.time + resolution.time +
               load.time + start.time;
    }

    std::string GetTextSummary() const {
        std::string buffer;
        auto it = std::back_inserter(buffer);

        std::format_to(it, "=== Initialization Summary ===\n");
        std::format_to(it, "{}\n", discovery.GetText("Discovery"));
        std::format_to(it, "{}\n", parsing.GetText("Parsing"));
        std::format_to(it, "{}\n", resolution.GetText("Resolution"));
        std::format_to(it, "{}\n", load.GetText("Loading"));
        std::format_to(it, "{}\n", start.GetText("Starting"));
        std::format_to(it, "Total Time: {}ms\n", TotalTime().count());

        if (!errorLog.empty()) {
            std::format_to(it, "\n=== Errors ({}) ===\n", errorLog.size());
            for (const auto& [ctx, msg] : errorLog | std::views::take(10)) {
                std::format_to(it, "  [{}] {}\n", ctx, msg);
            }
        }

        if (!warningLog.empty()) {
            std::format_to(it, "\n=== Warnings ({}) ===\n", warningLog.size());
            for (const auto& [ctx, msg] : warningLog | std::views::take(10)) {
                std::format_to(it, "  [{}] {}\n", ctx, msg);
            }
        }

        return buffer;
    }
};

// Main Package Manager Class
struct Manager::Impl {
    // Plugify
    Plugify& plugify;

    // Configuration
    std::shared_ptr<Config> config;

    // Main initialization method - MUST be called from main thread
    InitializationState Initialize() {
        InitializationState state;

        // Phase 1: Discovery (parallel)
        DiscoverManifests(state);

        // Phase 2: Parsing (parallel)
        ParseManifests(state);

        // Phase 3: Dependency Resolution (sequential)
        ResolveDependencies(state);

        // Phase 4: Sequential Loading on main thread
        LoadPackagesSequentially(state);

        // Phase 5: Sequential Starting on main thread
        StartPackagesSequentially(state);

        return state;
    }

    // Terminate packages in reverse order
    void Terminate() {
        // Terminate in reverse order of initialization
        for (auto it = loadedPackages.rbegin(); it != loadedPackages.rend(); ++it) {
            try {
                auto& pkgInfo = packages[*it];
                if (pkgInfo.instance && pkgInfo.state == PackageState::Stalled) {
                    if (config.enableDetailedLogging) {
                        std::cout << "[Terminate] Stopping " << *it << std::endl;
                    }
                    pkgInfo.instance->stop();
                    pkgInfo.instance->unload();
                    pkgInfo.state = PackageState::Terminated;
                }
            } catch (const std::exception& e) {
                // Log but continue termination
                std::cerr << "[ERROR] Failed to terminate " << *it << ": " << e.what() << std::endl;
            }
        }
        loadedPackages.clear();
        packages.clear();
    }

    // Get current package states (for monitoring)
    std::unordered_map<std::string, PackageState> GetPackageStates() const {
        std::unordered_map<std::string, PackageState> states;
        states.reserve(packages.size());
        for (const auto& pkg : packages) {
            states[pkg.GetName()] = pkg.GetState();
        }
        return states;
    }

private:
    using Clock = std::chrono::steady_clock;

    // Clean phase execution with timing
    template<typename Func>
    void ExecutePhase(std::string_view name, PhaseStatistics& stats, Func&& func) {
        auto start = Clock::now();
        func();
        stats.time = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start);
    }

    // Step 1: Find all manifest files in parallel
    void DiscoverManifests(InitializationState& state) {
        auto startTime = clock::now();

        if (progressReporter) {
            progressReporter->OnPhaseStart("Discovery", config->searchPaths.size());
        }

        struct alignas(std::hardware_destructive_interference_size) Slot {
            std::vector<std::filesystem::path> v;
        };
        std::vector<Slot> combinedPaths;
        combinedPaths.resize(config->searchPaths.size());

        threadPool.submit_sequence(0, config->searchPaths.size(), [this, &combinedPaths, &state](const size_t i) {
            auto& searchPath = config->searchPaths[i];
            bool success = false;
            try {
                auto result = fileSystem->FindFiles(searchPath, {"*.pplugin", "*.pmodule"}, false);
                if (result) {
                    combinedPaths[i] = Slot{std::move(*result)};
                    success = true;
                } else {
                    state.AddError(
                        "Discovery: " + searchPath.string(),
                        result.error()
                    );
                }
            } catch (const std::exception& e) {
                state.AddError(
                    "Discovery: " + searchPath.string(),
                    e.what()
                );
            }
            if (success) {
                state.discovery.AddSuccess(combinedPaths[i].v.size());
            } else {
                state.discovery.AddFailed();
            }
            if (progressReporter) {
                progressReporter->OnItemComplete("Discovery", searchPath.string(), success);
            }
        }).wait();

        size_t total = 0;
        for (auto& paths : combinedPaths) {
            total += paths.v.size();
        }
        packages.reserve(total);
        for (auto& paths : combinedPaths) {
            for (auto& path : paths.v) {
                auto id = packages.size() + 1;
                PackageInfo& info = packages.emplace_back();
                info.SetId(id);
                info.SetPath(std::move(path));
                info.SetState(PackageState::Discovered);
            }
        }

        auto endTime = clock::now();
        state.discovery.time = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);

        if (progressReporter) {
            progressReporter->OnPhaseComplete("Discovery", state.discovery.success, state.discovery.failed);
        }
    }

    // Phase 2: Parse manifests in parallel
    void ParseManifests(InitializationState& state) {
        auto startTime = clock::now();

        if (progressReporter) {
            progressReporter->OnPhaseStart("Parsing", packages.size());
        }

        threadPool.submit_sequence(0, packages.size(), [this, &state](const size_t i) {
            auto& pkg = packages[i];

            bool success = false;
            try {
                pkg.StartOperation(PackageState::Parsing);

                auto readResult = fileSystem->ReadTextFile(pkg.GetPath());
                if (!readResult) {
                    state.AddError(
                        "Parsing: " + pkg.GetPath().string(),
                        readResult.error()
                    );
                    return;
                }

                auto parseResult = manifestParser->Parse(readResult.value(), pkg.GetPath());
                if (parseResult) {
                    pkg.EndOperation(PackageState::Parsed);
                    pkg.SetManifest(std::move(parseResult.value()));
                    success = true;
                } else {
                    pkg.EndOperation(PackageState::Corrupted);
                    pkg.AddError(parseResult.error());
                    state.AddError(
                        "Parsing: " + pkg.GetPath().string(),
                        parseResult.error()
                    );
                }
            } catch (const std::exception& e) {
                pkg.EndOperation(PackageState::Corrupted);
                pkg.AddError(e.what());
                state.AddError(
                    "Parsing: " + pkg.GetPath().string(),
                    e.what()
                );
            }
            if (success) {
                state.parsing.AddSuccess();
            } else {
                state.parsing.AddFailed();
            }
            if (progressReporter) {
                progressReporter->OnItemComplete("Parsing", success ? pkg.GetName() : pkg.GetPath().string(), success);
            }
        }).wait();

        auto endTime = clock::now();
        state.parsing.time = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        if (progressReporter) {
            progressReporter->OnPhaseComplete("Parsing", state.parsing.success, state.parsing.failed);
        }
    }

    // Phase 3: Dependency Resolution
    void ResolveDependencies(InitializationState& state) {
        auto startTime = clock::now();

        std::unordered_map<std::string, std::string, plg::case_insensitive_hash, plg::case_insensitive_equal> languages;

        for (const auto& pkg : packages) {
            if (pkg.GetState() == PackageState::Parsed) {
                if (pkg.GetType() == PackageType::Module) {
                    languages[pkg.GetLanguage()] = pkg.GetName();
                }
            }
        }

        std::unordered_map<UniqueId, ManifestPtr> validPackages;
        for (auto& pkg : packages) {
            if (pkg.GetState() == PackageState::Parsed) {
                // Apply filtering
                if (config->whitelistedPackages && !config->whitelistedPackages->contains(pkg.GetName()) ||
                    config->blacklistedPackages && config->blacklistedPackages->contains(pkg.GetName())
                ) {
                    pkg.SetState(PackageState::Disabled);
                    state.resolution.AddFailed();
                    continue;
                }

                // Always register language as dependency
                if (pkg.GetType() == PackageType::Plugin) {
                    auto it = languages.find(pkg.GetLanguage());
                    if (it != languages.end()) {
                        pkg.AddDependency(it->second);
                    }
                }

                pkg.StartOperation(PackageState::Resolving);
                validPackages[pkg.GetId()] = pkg.GetManifest();
            }
        }

        if (progressReporter) {
            progressReporter->OnPhaseStart("Resolution", validPackages.size());
        }

        try {
            auto depReport = resolver->Resolve(validPackages);

            std::unordered_set<UniqueId> resolvedSet(
                depReport.loadOrder.begin(),
                depReport.loadOrder.end());

            for (auto& pkg : packages) {
                if (pkg.GetState() == PackageState::Resolving) {
                    bool success = resolvedSet.contains(pkg.GetId());
                    if (success) {
                        pkg.EndOperation(PackageState::Resolved);
                        state.resolution.AddSuccess();
                    } else {
                        pkg.EndOperation(PackageState::Unresolved);
                        state.resolution.AddFailed();

                        auto it = depReport.issues.find(pkg.GetId());
                        if (it != depReport.issues.end()) {
                            for (const auto& issue : it->second) {
                                auto details = issue.GetDetailedDescription();
                                if (issue.isBlocking) {
                                    state.AddError("Resolution: " + pkg.GetName(), details);
                                    pkg.AddError(std::move(details));
                                } else {
                                    state.AddWarning("Resolution: " + pkg.GetName(), details);
                                    pkg.AddWarning(std::move(details));
                                }
                            }
                        }
                    }
                    if (progressReporter) {
                        progressReporter->OnItemComplete("Resolution", pkg.GetName(), success);
                    }
                }
            }

            loadOrder = std::move(depReport.loadOrder);
            dependencyGraph = std::move(depReport.dependencyGraph);
            reverseDependencyGraph = std::move(depReport.reverseDependencyGraph);

        } catch (const std::exception& e) {
            state.AddError("Resolution", e.what());
        }

        if (progressReporter) {
            progressReporter->OnPhaseComplete("Resolution", state.resolution.success, state.resolution.failed);
        }

        auto endTime = clock::now();
        state.resolution.time = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);
    }

    // Phase 4: Sequential Loading on Main Thread
    void LoadPackagesSequentially(InitializationState& state) {
        auto startTime = clock::now();
        std::unordered_set<UniqueId> failedPackages;

        if (progressReporter) {
            progressReporter->OnPhaseStart("Loading", loadOrder.size());
        }

        for (const auto& pkgId : loadOrder) {
            // Check if dependencies failed
            std::vector<std::string_view> depFailed;
            for (const auto& dep : dependencyGraph[pkgId]) {
                if (failedPackages.contains(dep)) {
                    depFailed.emplace_back(packages[dep].GetName());
                }
            }

            auto& pkg = packages[pkgId];
            if (!depFailed.empty()) {
                pkg.SetState(PackageState::Skipped);
                pkg.AddError("Dependency(s) failed: " + plg::join(depFailed, ", "));
                state.load.AddFailed();
                failedPackages.insert(pkgId);

                if (progressReporter) {
                    progressReporter->OnItemComplete("Loading", pkg.GetName(), false);
                }
                continue;
            }

            // Load the package
            pkg.StartOperation(PackageState::Loading);

            /*if (config.enableDetailedLogging) {
                std::cout << "[Load] Loading " << pkgId;
                if (pkg.isLanguageModule) {
                    std::cout << " (language module)";
                }
                std::cout << std::endl;
            }*/

            try {
                pkg.instance = loader->load(pkg.GetManifest());

                if (pkg.instance) {
                    pkg.EndOperation(PackageState::Loaded);
                    state.load.AddSuccess();
                    loadedPackages.push_back(pkgId);

                    if (progressReporter) {
                        progressReporter->OnItemComplete("Loading", pkgId, true);
                    }
                } else {
                    throw std::runtime_error("Loader returned null");
                }
            } catch (const std::exception& e) {
                pkg.EndOperation(PackageState::Failed);
                pkg.AddError(e.what());
                state.load.AddFailed();
                failedPackages.insert(pkgId);
                state.AddError("Loading", e.what());

                // Propagate failure to dependents
                PropagateFailure(pkgId, failedPackages);

                if (progressReporter) {
                    progressReporter->OnItemComplete("Loading", pkg.GetName(), false);
                }

                // Stop on critical failure
                if (config.stopOnCriticalFailure) {
                    PL_LOG_ERROR("[CRITICAL] {} failed to load: {}", pkg.GetName(), e.what());
                    break;
                }
            }
        }

        auto endTime = clock::now();
        state.load.time = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        if (progressReporter) {
            progressReporter->OnPhaseComplete("Loading", state.load.success, state.load.failed);
        }
    }

    // Phase 5: Sequential Starting on Main Thread
    void StartPackagesSequentially(InitializationState& state) {
        auto startTime = clock::now();

        if (progressReporter) {
            size_t toStart = 0;
            for (const auto& pkgId : loadOrder) {
                if (packages[pkgId].GetState() == PackageState::Loaded) {
                    toStart++;
                }
            }
            progressReporter->OnPhaseStart("Starting", toStart);
        }

        for (const auto& pkgId : loadOrder) {
            auto& pkg = packages[pkgId];

            if (pkg.GetState() != PackageState::Loaded) {
                continue;
            }

            pkg.StartOperation(PackageState::Starting);

            if (config.enableDetailedLogging) {
                std::cout << "[Start] Starting " << pkgId << std::endl;
            }

            try {
                if (pkg.instance->start()) {
                    pkg.EndOperation(PackageState::Stalled);
                    state.start.AddSuccess();

                    if (progressReporter) {
                        progressReporter->OnItemComplete("Starting", pkg.GetName(), true);
                    }
                } else {
                    throw std::runtime_error("Start returned false");
                }
            } catch (const std::exception& e) {
                pkg.EndOperation(PackageState::Stalled);
                pkg.AddError(e.what());
                state.start.AddFailed();
                state.AddError("Starting", e.what());

                if (progressReporter) {
                    progressReporter->OnItemComplete("Starting", pkg.GetName(), false);
                }

                // Note: Dependents are already loaded at this point
                // You might want to handle this differently
            }
        }

        auto endTime = clock::now();
        state.start.time = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);

        if (progressReporter) {
            progressReporter->OnPhaseComplete("Starting", state.start.success, state.start.failed);
        }
    }

    // Helper: Propagate failure to dependents
    void PropagateFailure(const UniqueId& failedPkg,
                         std::unordered_set<UniqueId>& failedSet) {
        std::stack<UniqueId> toVisit;
        toVisit.push(failedPkg);

        while (!toVisit.empty()) {
            UniqueId current = std::move(toVisit.top());
            toVisit.pop();

            for (const auto& dependent : reverseDependencyGraph[current]) {
                if (!failedSet.contains(dependent)) {
                    failedSet.insert(dependent);
                    toVisit.push(dependent);
                }
            }
        }
    }

private:
    std::shared_ptr<IFileSystem> fileSystem;
    std::shared_ptr<IManifestParser> manifestParser;
    std::shared_ptr<IDependencyResolver> resolver;
    std::shared_ptr<IPackageLoader> loader;
    std::shared_ptr<IProgressReporter> progressReporter;

    //ManagerConfig config;
    plg::synced_stream syncOut;
    plg::thread_pool<> threadPool;

    std::vector<PackageInfo> packages;
    //std::mutex mutex;

    std::vector<UniqueId> loadedPackages;

    std::unordered_map<UniqueId, std::vector<UniqueId>> dependencyGraph;
    std::unordered_map<UniqueId, std::vector<UniqueId>> reverseDependencyGraph;
    std::vector<UniqueId> loadOrder;
};

// Example implementations
class ConsoleProgressReporter : public IProgressReporter {
    plg::synced_stream& syncOut;
public:
    ConsoleProgressReporter(plg::synced_stream& out)
        : syncOut(out) {}

    void OnPhaseStart(std::string_view phase, size_t totalItems) override {
        syncOut.println("[", phase,  "] Starting (",  totalItems, " items)...");
    }

    void OnItemComplete(std::string_view phase, std::string_view itemId, bool success) override {
        if (!success) {
            syncOut.println("  [", phase, "] ✗ ", itemId);
        }
    }

    void OnPhaseComplete(std::string_view phase, size_t succeeded, size_t failed) override {
        std::cout << "[" << phase << "] Complete - ✓ " << succeeded << ", ✗ " << failed << std::endl;
    }
};

/*
// Usage Example
inline void exampleUsage() {
    // Configuration
    ManagerConfig config;
    config.threadPoolSize = std::thread::hardware_concurrency();
    config.stopOnCriticalFailure = true;
    config.enableDetailedLogging = true;
    config.validateInParallel = true;

    // Create services
    auto fileSystem = std::make_shared<FileSystem>();  // Your implementation
    auto manifestParser = std::make_shared<JsonManifestParser>();  // Your implementation
    auto validator = std::make_shared<PackageValidator>();  // Your implementation
    auto resolver = std::make_shared<LibSolvDependencyResolver>();  // Your libsolv wrapper
    auto loader = std::make_shared<DynamicPackageLoader>();  // Your implementation

    // Create package manager
    auto Manager = std::make_unique<Manager>(
        fileSystem, manifestParser, validator, resolver, loader, config
    );

    // Set progress reporter
    Manager->setProgressReporter(
        std::make_shared<ConsoleProgressReporter>()
    );

    // Initialize packages (MUST be called from main thread)
    std::cout << "=== Package Initialization Starting ===" << std::endl;

    auto state = Manager->initialize(
        {"/usr/local/packages", "/opt/plugins", "~/.local/packages"},
        {"*.manifest.json", "package.json"}
    );

    // Report results
    std::cout << "\n=== Initialization Summary ===" << std::endl;
    std::cout << "Discovered:    " << state.totalDiscovered << std::endl;
    std::cout << "Validated:     " << state.validationPassed << std::endl;
    std::cout << "Failed Valid:  " << state.validationFailed << std::endl;
    std::cout << "Resolved:      " << state.dependencyResolved << std::endl;
    std::cout << "Loaded:        " << state.loaded << std::endl;
    std::cout << "Started:       " << state.started << std::endl;
    std::cout << "Failed:        " << state.failed << std::endl;
    std::cout << "Skipped:       " << state.skipped << std::endl;

    std::cout << "\n=== Timing ===" << std::endl;
    std::cout << "Discovery:     " << state.totalDiscoveryTime.count() << "ms" << std::endl;
    std::cout << "Validation:    " << state.totalValidationTime.count() << "ms" << std::endl;
    std::cout << "Resolution:    " << state.totalResolutionTime.count() << "ms" << std::endl;
    std::cout << "Loading:       " << state.totalLoadTime.count() << "ms" << std::endl;
    std::cout << "Starting:      " << state.totalStartTime.count() << "ms" << std::endl;

    if (!state.errorLog.empty()) {
        std::cout << "\n=== Errors ===" << std::endl;
        for (const auto& [context, error] : state.errorLog) {
            std::cout << "[" << context << "] " << error << std::endl;
        }
    }

    // Run your application...

    // Cleanup
    std::cout << "\n=== Shutting down ===" << std::endl;
    Manager->terminate();
}
*/





// ============================================================================
// Dependency Injection
// ============================================================================

// Set progress reporter
void Manager::setProgressReporter(std::shared_ptr<IProgressReporter> reporter) {
    progressReporter = reporter;
}

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

