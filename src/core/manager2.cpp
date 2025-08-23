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

#include "plugify/core/dependency_resolver.hpp"
#include "plugify/core/file_system.hpp"
#include "plugify/core/manifest.hpp"
#include "plugify/core/manifest_parser.hpp"
#include "plugify/core/package.hpp"
#include "plugify/core/thread_pool.hpp"

#include "asm/defer.hpp"

using namespace plugify;

// Package Manager Implementation with Thread Pool for Discovery/Validation
// Sequential Loading/Starting on Main Thread for Simplicity
//
// Design Philosophy:
// - Parallel: Discovery and Validation (I/O bound, no runtime dependencies)
// - Sequential: Loading and Starting (respects dependencies, avoids threading issues)

// Initialization Statistics
struct InitializationState {
    size_t totalDiscovered = 0;
    size_t totalParsed = 0;
    size_t validationPassed = 0;
    size_t validationFailed = 0;
    size_t dependencyResolved = 0;
    size_t skipped = 0;
    size_t loaded = 0;
    size_t started = 0;
    size_t failed = 0;

    std::chrono::milliseconds totalDiscoveryTime;
    std::chrono::milliseconds totalParsingTime;
    std::chrono::milliseconds totalValidationTime;
    std::chrono::milliseconds totalResolutionTime;
    std::chrono::milliseconds totalLoadTime;
    std::chrono::milliseconds totalStartTime;

    std::deque<std::pair<std::string, std::string>> errorLog;
    std::deque<std::pair<std::string, std::string>> warningLog;

    void addError(std::string context, std::string message) {
        errorLog.emplace_back(std::move(context), std::move(message));
    }
    void addWarning(std::string context, std::string message) {
        warningLog.emplace_back(std::move(context), std::move(message));
    }
};

// Progress Reporter Interface
class IProgressReporter {
public:
    virtual ~IProgressReporter() = default;
    virtual void onPhaseStart(const std::string& phase, size_t totalItems) = 0;
    virtual void onItemComplete(const std::string& phase, const std::string& itemId, bool success) = 0;
    virtual void onPhaseComplete(const std::string& phase, size_t succeeded, size_t failed) = 0;
};

// Main Package Manager Class
class PackageManager {
    using clock = std::chrono::steady_clock;
public:
    PackageManager(
        std::shared_ptr<IFileSystem> fileSystem,
        std::shared_ptr<IManifestParser> manifestParser,
        std::shared_ptr<IPackageValidator> validator,
        std::shared_ptr<IDependencyResolver> resolver,
        std::shared_ptr<IPackageLoader> loader,
        const PackageManagerConfig& config = PackageManagerConfig())
        : m_fileSystem(fileSystem)
        , m_manifestParser(manifestParser)
        , m_validator(validator)
        , m_resolver(resolver)
        , m_loader(loader)
        , m_config(config)
        , m_threadPool(config.threadPoolSize) {}

    // Set progress reporter
    void setProgressReporter(std::shared_ptr<IProgressReporter> reporter) {
        m_progressReporter = reporter;
    }

    // Main initialization method - MUST be called from main thread
    InitializationState initialize() {
        InitializationState state;

        // Phase 1: Discovery and Parsing (parallel, using thread pool)
        discoverAndParseManifests(state);

        // Phase 2: Validation (parallel, using thread pool)
        validatePackages(state);

        // Phase 3: Dependency Resolution (sequential)
        resolveDependencies(state);

        // Phase 4: Sequential Loading on main thread
        loadPackagesSequentially(state);

        // Phase 5: Sequential Starting on main thread
        startPackagesSequentially(state);

        return state;
    }

    // Terminate packages in reverse order
    void terminate() {
        // Terminate in reverse order of initialization
        for (auto it = m_loadedPackages.rbegin(); it != m_loadedPackages.rend(); ++it) {
            try {
                auto& pkgInfo = m_packages[*it];
                if (pkgInfo.instance && pkgInfo.state == PackageState::Started) {
                    if (m_config.enableDetailedLogging) {
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
        m_loadedPackages.clear();
        m_packages.clear();
    }

    // Get current package states (for monitoring)
    std::unordered_map<std::string, PackageState> getPackageStates() const {
        std::unordered_map<std::string, PackageState> states;
        for (const auto& [id, info] : m_packages) {
            states[id] = info.state;
        }
        return states;
    }

private:
    // Step 1: Find all manifest files in parallel
    std::vector<std::filesystem::path> discoverManifests(InitializationState& state) {
        auto startTime = clock::now();

        if (m_progressReporter) {
            m_progressReporter->onPhaseStart("Discovery", searchPaths.size());
        }

        std::mutex mutex;
        std::latch latch(static_cast<ptrdiff_t>(searchPaths.size()));
        std::vector<std::filesystem::path> manifestPaths;

        for (const auto& searchPath : searchPaths) {
            m_threadPool.Enqueue([this, searchPath, &manifestPaths, &latch, &mutex, &state]() {
                bool success = false;
                defer {
                    if (m_progressReporter) {
                        m_progressReporter->onItemComplete("Discovery", searchPath.string(), success);
                    }
                    latch.count_down();
                };
                try {
                    auto result = m_fileSystem->FindFiles(searchPath, {"*.pplugin", "*.pmodule"}, false);
                    if (result) {
                        std::scoped_lock<std::mutex> lock(mutex);
                        state.totalDiscovered += result->size();
                        manifestPaths.insert(manifestPaths.end(),
                                        std::make_move_iterator(result->begin()),
                                        std::make_move_iterator(result->end()));
                        success = true;
                    } else {
                        std::scoped_lock<std::mutex> lock(mutex);
                        state.addError(
                            "Discovery: " + searchPath.string(),
                            result.error()
                        );
                    }
                } catch (const std::exception& e) {
                    std::scoped_lock<std::mutex> lock(mutex);
                    state.addError(
                        "Discovery: " + searchPath.string(),
                        e.what()
                    );
                }
            });
        }
        latch.wait();

        auto endTime = clock::now();
        state.totalDiscoveryTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);

        if (m_progressReporter) {
            m_progressReporter->onPhaseComplete("Discovery", state.totalDiscovered,
                manifestPaths.size() - state.totalDiscovered);
        }

        return manifestPaths;
    }

    void parseManifests(std::vector<std::filesystem::path> manifestPaths, InitializationState& state) {
        auto startTime = clock::now();

        if (m_progressReporter) {
            m_progressReporter->onPhaseStart("Parsing", manifestPaths.size());
        }

        std::mutex mutex;
        std::latch latch(static_cast<ptrdiff_t>(manifestPaths.size()));

        for (const auto& manifestPath : manifestPaths) {
            m_threadPool.Enqueue([this, manifestPath, &latch, &mutex, &state]() {
                    bool success = false;
                    defer {
                        if (m_progressReporter) {
                            m_progressReporter->onItemComplete("Parsing", manifestPath.string(), success);
                        }
                        latch.count_down();
                    };
                    try {
                        auto readResult = m_fileSystem->ReadTextFile(manifestPath);
                        if (!readResult) {
                            std::scoped_lock<std::mutex> lock(mutex);
                            state.addError(
                                "Parsing:" + manifestPath.string(),
                                readResult.error()
                            );
                            return;
                        }

                        auto parseResult = m_manifestParser->Parse(readResult.value(), manifestPath);
                        if (parseResult) {
                            auto& manifest = parseResult.value();

                            PackageInfo info;
                            info.manifest = std::move(manifest);
                            info.state = PackageState::Initialized;
                            info.initializedTime = clock::now();

                            std::scoped_lock<std::mutex> lock(mutex);

                            if (m_packages.contains(info.id)) {
                                state.addWarning(
                                    "Parsing: " + info.id,
                                    "Duplicate package at: " + info.path.string()
                                );
                            } else {
                                success = true;
                                m_packages[info.id] = std::move(info);
                                state.totalParsed++;
                            }
                        } else {
                            std::scoped_lock<std::mutex> lock(mutex);
                            state.addError(
                                "Parsing: " + manifestPath.string(),
                                parseResult.error()
                            );
                        }
                    } catch (const std::exception& e) {
                        std::scoped_lock<std::mutex> lock(mutex);
                        state.addError(
                            "Parsing: " + manifestPath.string(),
                            e.what()
                        );
                    }
                }
            );
        }
        latch.wait();

        auto endTime = clock::now();
        state.totalParsingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);

        if (m_progressReporter) {
            m_progressReporter->onPhaseComplete("Parsed", state.totalParsed,
                manifestPaths.size() - state.totalParsed);
        }
    }

    // Phase 3: Dependency Resolution
    void resolveDependencies(InitializationState& state) {
        auto startTime = clock::now();

        std::unordered_map<PackageId, ManifestPtr> validPackages;
        for (const auto& [pkgId, pkg] : m_packages) {
            if (pkg.state == PackageState::ValidationPassed) {
                validPackages[pkgId] = pkg.manifest;
            }
        }

        if (m_progressReporter) {
            m_progressReporter->onPhaseStart("Resolution", validPackages.size());
        }

        try {
            auto depReport = m_resolver->Resolve(validPackages);

            std::unordered_set<std::string> resolvedSet(
                depReport.loadOrder.begin(), depReport.loadOrder.end());

            for (auto& [pkgId, pkg] : m_packages) {
                if (pkg.state == PackageState::ValidationPassed) {
                    bool success = resolvedSet.contains(pkgId);
                    if (success) {
                        pkg.state = PackageState::DependencyResolved;
                        state.dependencyResolved++;
                    } else {
                        pkg.state = PackageState::Skipped;
                        state.skipped++;

                        auto it = depReport.issues.find(pkgId);
                        if (it != depReport.issues.end()) {
                            for (const auto& issue : it->second) {
                                auto details = issue.GetDetailedDescription();
                                if (issue.isBlocking) {
                                    pkg.errors.push_back(details);
                                    state.errorLog.push_back({pkgId, details});
                                } else {
                                    pkg.warnings.push_back(details);
                                    state.warningLog.push_back({pkgId, details});
                                }
                            }
                        }
                    }

                    if (m_progressReporter) {
                        m_progressReporter->onItemComplete("Resolution", pkgId, success);
                    }
                }
            }

            m_loadOrder = std::move(depReport.loadOrder);
            m_dependencyGraph = std::move(depReport.dependencyGraph);
            m_reverseDependencyGraph = std::move(depReport.reverseDependencyGraph);

            if (m_progressReporter) {
                m_progressReporter->onPhaseComplete("Resolution",
                    state.dependencyResolved, state.skipped);
            }

            auto endTime = clock::now();
            state.totalResolutionTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime);

        } catch (const std::exception& e) {
            state.errorLog.push_back({"Resolution", e.what()});
        }
    }

    // Phase 4: Sequential Loading on Main Thread
    void loadPackagesSequentially(InitializationState& state) {
        auto startTime = clock::now();
        std::unordered_set<std::string> failedPackages;

        if (m_progressReporter) {
            m_progressReporter->onPhaseStart("Loading", m_loadOrder.size());
        }

        for (const auto& pkgId : m_loadOrder) {
            // Check if dependencies failed
            bool depFailed = false;
            for (const auto& dep : m_dependencyGraph[pkgId]) {
                if (failedPackages.contains(dep)) {
                    depFailed = true;
                    break;
                }
            }

            auto& pkg = m_packages[pkgId];
            if (depFailed) {
                pkg.state = PackageState::Skipped;
                pkg.errors.push_back("Dependency failed");
                state.skipped++;
                failedPackages.insert(pkgId);

                if (m_progressReporter) {
                    m_progressReporter->onItemComplete("Loading", pkgId, false);
                }
                continue;
            }

            // Load the package
            pkg.state = PackageState::Loading;
            pkg.loadStartTime = clock::now();

            if (m_config.enableDetailedLogging) {
                std::cout << "[Load] Loading " << pkgId;
                if (pkg.isLanguageModule) {
                    std::cout << " (language module)";
                }
                std::cout << std::endl;
            }

            try {
                pkg.instance = m_loader->load(pkg.manifest);

                if (pkg.instance) {
                    pkg.state = PackageState::Loaded;
                    pkg.loadEndTime = clock::now();
                    state.loaded++;
                    m_loadedPackages.push_back(pkgId);

                    if (m_progressReporter) {
                        m_progressReporter->onItemComplete("Loading", pkgId, true);
                    }
                } else {
                    throw std::runtime_error("Loader returned null");
                }
            } catch (const std::exception& e) {
                pkg.state = PackageState::Failed;
                pkg.errors.push_back("Load failed: " + std::string(e.what()));
                pkg.loadEndTime = clock::now();
                state.failed++;
                failedPackages.insert(pkgId);
                state.errorLog.push_back({pkgId, "Load failed: " + std::string(e.what())});

                // Propagate failure to dependents
                propagateFailure(pkgId, failedPackages);

                if (m_progressReporter) {
                    m_progressReporter->onItemComplete("Loading", pkgId, false);
                }

                // Stop on critical failure
                if (m_config.stopOnCriticalFailure && pkg.isLanguageModule) {
                    std::cerr << "[CRITICAL] Language module failed to load: " << pkgId << std::endl;
                    break;
                }
            }
        }

        auto endTime = clock::now();
        state.totalLoadTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);

        if (m_progressReporter) {
            m_progressReporter->onPhaseComplete("Loading", state.loaded, state.failed);
        }
    }

    // Phase 5: Sequential Starting on Main Thread
    void startPackagesSequentially(InitializationState& state) {
        auto startTime = clock::now();

        if (m_progressReporter) {
            size_t toStart = 0;
            for (const auto& pkgId : m_loadOrder) {
                if (m_packages[pkgId].state == PackageState::Loaded) {
                    toStart++;
                }
            }
            m_progressReporter->onPhaseStart("Starting", toStart);
        }

        for (const auto& pkgId : m_loadOrder) {
            auto& pkg = m_packages[pkgId];

            if (pkg.state != PackageState::Loaded) {
                continue;
            }

            pkg.state = PackageState::Starting;

            if (m_config.enableDetailedLogging) {
                std::cout << "[Start] Starting " << pkgId << std::endl;
            }

            try {
                if (pkg.instance->start()) {
                    pkg.state = PackageState::Started;
                    pkg.startTime = clock::now();
                    state.started++;

                    if (m_progressReporter) {
                        m_progressReporter->onItemComplete("Starting", pkgId, true);
                    }
                } else {
                    throw std::runtime_error("Start returned false");
                }
            } catch (const std::exception& e) {
                pkg.state = PackageState::Failed;
                pkg.errors.push_back("Start failed: " + std::string(e.what()));
                state.failed++;
                state.errorLog.push_back({pkgId, "Start failed: " + std::string(e.what())});

                if (m_progressReporter) {
                    m_progressReporter->onItemComplete("Starting", pkgId, false);
                }

                // Note: Dependents are already loaded at this point
                // You might want to handle this differently
            }
        }

        auto endTime = clock::now();
        state.totalStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);

        if (m_progressReporter) {
            m_progressReporter->onPhaseComplete("Starting", state.started,
                state.failed - state.loaded);
        }
    }

    // Helper: Propagate failure to dependents
    void propagateFailure(const std::string& failedPkg,
                         std::unordered_set<std::string>& failedSet) {
        std::stack<std::string> toVisit;
        toVisit.push(failedPkg);

        while (!toVisit.empty()) {
            std::string current = std::move(toVisit.top());
            toVisit.pop();

            for (const auto& dependent : m_reverseDependencyGraph[current]) {
                if (!failedSet.contains(dependent)) {
                    failedSet.insert(dependent);
                    toVisit.push(dependent);
                }
            }
        }
    }

private:
    std::shared_ptr<IFileSystem> m_fileSystem;
    std::shared_ptr<IManifestParser> m_manifestParser;
    std::shared_ptr<IPackageValidator> m_validator;
    std::shared_ptr<IDependencyResolver> m_resolver;
    std::shared_ptr<IPackageLoader> m_loader;

    PackageManagerConfig m_config;
    ThreadPool m_threadPool;
    std::shared_ptr<IProgressReporter> m_progressReporter;

    std::unordered_map<PackageId, PackageInfo> m_packages;
    std::vector<PackageId> m_loadedPackages;

    std::unordered_map<PackageId, std::vector<PackageId>> m_dependencyGraph;
    std::unordered_map<PackageId, std::vector<PackageId>> m_reverseDependencyGraph;
    std::vector<PackageId> m_loadOrder;
};

// Example implementations
class ConsoleProgressReporter : public IProgressReporter {
public:
    void onPhaseStart(const std::string& phase, size_t totalItems) override {
        std::cout << "[" << phase << "] Starting (" << totalItems << " items)..." << std::endl;
    }

    void onItemComplete(const std::string& phase, const std::string& itemId, bool success) override {
        if (!success) {
            std::cout << "  [" << phase << "] ✗ " << itemId << std::endl;
        }
    }

    void onPhaseComplete(const std::string& phase, size_t succeeded, size_t failed) override {
        std::cout << "[" << phase << "] Complete - ✓ " << succeeded << ", ✗ " << failed << std::endl;
    }
};

/*
// Usage Example
inline void exampleUsage() {
    // Configuration
    PackageManagerConfig config;
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
    auto packageManager = std::make_unique<PackageManager>(
        fileSystem, manifestParser, validator, resolver, loader, config
    );

    // Set progress reporter
    packageManager->setProgressReporter(
        std::make_shared<ConsoleProgressReporter>()
    );

    // Initialize packages (MUST be called from main thread)
    std::cout << "=== Package Initialization Starting ===" << std::endl;

    auto state = packageManager->initialize(
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
    packageManager->terminate();
}
*/




