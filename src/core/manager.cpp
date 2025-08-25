#pragma once

#include "plugify/core/config.hpp"
#include "plugify/core/manifest.hpp"
#include "plugify/core/package.hpp"
#include "plugify/core/plugify.hpp"

#include "asm/defer.hpp"
#include "plg/thread_pool.hpp"

using namespace plugify;
using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

// Package Manager Implementation with Thread Pool for Discovery/Validation
// Sequential Loading/Starting on Main Thread for Simplicity
//
// Design Philosophy:
// - Parallel: Discovery and Validation (I/O bound, no runtime dependencies)
// - Sequential: Loading and Starting (respects dependencies, avoids threading issues)

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
        syncOut.println("[", phase, "] Complete - ✓ ", succeeded, ", ✗ ", failed);
    }
};

class PhaseScope {
    IProgressReporter* reporter;
    std::string_view phase;
    PhaseStatistics& stats;
public:
    PhaseScope(IProgressReporter* r, std::string_view p, PhaseStatistics& s, size_t total)
        : reporter(r), phase(p), stats(s) {
        if (reporter) reporter->OnPhaseStart(phase, total);
    }
    ~PhaseScope() {
        if (reporter) reporter->OnPhaseComplete(phase, stats.success.load(), stats.failed.load());
    }
    void ReportItem(std::string_view item, bool success) {
        if (reporter) reporter->OnItemComplete(phase, item, success);
    }
};

// Main Package Manager Class
struct Manager::Impl {
    Impl(Plugify& p) : plugify(p) {
        fileSystem = std::make_shared<StandardFileSystem>();
        manifestParser = std::make_shared<GlazeManifestParser>();
        resolver = std::make_shared<LibsolvDependencyResolver>();
        //loader = std::make_shared<DefaultPackageLoader>(plugify.GetAssemblyLoader(),
        progressReporter = std::make_shared<ConsoleProgressReporter>(syncOut);
    }

    // Plugify
    Plugify& plugify;

    // Configuration
    std::shared_ptr<Config> config;

    // Initialization state
    std::shared_ptr<InitializationState> state;
    
    // Main initialization method - MUST be called from main thread
    std::shared_ptr<InitializationState> Initialize() {
        state = std::make_shared<InitializationState>();

        // Execute phases with timing
        ExecutePhase("Discovery", state->discovery,
            [this] { DiscoverManifests(); });

        ExecutePhase("Parsing", state->parsing,
            [this] { ParseManifests(); });

        ExecutePhase("Resolution", state->resolution,
            [this] { ResolveDependencies(); });

        /*ExecutePhase("Loading", state->load,
            [this] { LoadPackagesSequentially(); });

        ExecutePhase("Starting", state->start,
            [this] { StartPackagesSequentially(); });*/

        return state;
    }

    // Terminate packages in reverse order
    void Terminate() {
        // Terminate in reverse order of initialization
        /*for (auto it = loadedPackages.rbegin(); it != loadedPackages.rend(); ++it) {
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
        }*/
        loadedPackages.clear();
        packages.clear();
    }

    // Get current package states (for monitoring)
    plg::flat_map<std::string, PackageState> GetPackageStates() const {
        plg::flat_map<std::string, PackageState> states;
        states.reserve(packages.size());
        for (const auto& pkg : packages) {
            states[pkg.GetName()] = pkg.GetState();
        }
        return states;
    }

    static constexpr std::array<std::string_view, 2> MANIFEST_EXTENSIONS = {
        "*.pplugin"sv, "*.pmodule"sv
    };

    // Clean phase execution with timing
    template<typename Func>
    static void ExecutePhase(std::string_view name, PhaseStatistics& stats, Func&& func) {
        PL_LOG_VERBOSE("Entering phase: {}", name);
        auto start = Clock::now();
        func();
        stats.time = std::chrono::duration_cast<Duration>(Clock::now() - start);
        PL_LOG_VERBOSE("  {}", stats.GetText(name));
    }

    PLUGIFY_WARN_PUSH()
#if PLUGIFY_COMPILER_MSVC
    PLUGIFY_WARN_IGNORE(4324)
#endif
    struct alignas(std::hardware_destructive_interference_size) PathBatch {
        std::vector<std::filesystem::path> v{};

        auto begin() { return v.begin(); }
        auto end() { return v.end(); }
        auto begin() const { return v.begin(); }
        auto end() const { return v.end(); }
        auto size() const { return v.size(); }
        auto empty() const { return v.empty(); }
    };
    PLUGIFY_WARN_POP()

    // Collect manifest paths from all search directories
    auto CollectManifestPaths() {
        std::vector<PathBatch> batches;
        batches.resize(config->searchPaths.size());

        threadPool.submit_sequence(0, config->searchPaths.size(),
        [this, &batches](size_t i) {
            const auto& searchPath = config->searchPaths[i];

            if (auto result = fileSystem->FindFiles(searchPath, MANIFEST_EXTENSIONS, true)) {
                batches[i].v = std::move(*result);
            } else {
                state->AddError(std::format("Discovery: {}", searchPath.string()), std::move(result.error()));
            }
        }).wait();

        return batches;
    }

    // Step 1: Find all manifest files in parallel
    void DiscoverManifests() {
        PhaseScope scope(progressReporter.get(), "Discovery", state->discovery, config->searchPaths.size());

        // Collect all paths in parallel
        auto allPaths = CollectManifestPaths();

        // Create package entries
        auto paths = allPaths | std::views::join;
        packages.reserve(std::ranges::distance(paths));
        for (auto&& path : paths) {
            auto& pkg = packages.emplace_back();
            pkg.SetId(static_cast<UniqueId>(packages.size() - 1));
            pkg.SetPath(std::move(path));
            pkg.SetState(PackageState::Discovered);
            scope.ReportItem(pkg.GetPath().string(), true);
        }
    }

    // Phase 2: Parse manifests in parallel
    void ParseManifests() {
        PhaseScope scope(progressReporter.get(), "Discovery", state->parsing, packages.size());

        threadPool.submit_sequence(0, packages.size(),
        [this, &scope](size_t i) {
            auto& pkg = packages[i];
            bool success = ParseSingleManifest(pkg);
            scope.ReportItem(pkg.GetName(), success);
        }).wait();
    }

    // Parse a single manifest with proper error handling
    bool ParseSingleManifest(PackageInfo& pkg) {
        pkg.StartOperation(PackageState::Parsing);

        if (auto manifest = ParseManifest(pkg.GetPath())) {
            pkg.SetManifest(std::move(*manifest));
            pkg.EndOperation(PackageState::Parsed);
            state->parsing.AddSuccess();
            return true;
        } else {
            pkg.AddError(manifest.error());
            pkg.EndOperation(PackageState::Corrupted);
            state->parsing.AddFailed();
            return false;
        }
    }

    std::expected<ManifestPtr, std::string> ParseManifest(const std::filesystem::path& path) const {
        auto content = fileSystem->ReadTextFile(path);
        if (!content) return std::unexpected(content.error());

        auto parsed = manifestParser->Parse(content.value(), path);
        if (!parsed) return std::unexpected(parsed.error());

        return std::move(parsed.value());
    }

    // Build registry of language modules
    auto BuildLanguageRegistry() const {
        plg::flat_map<std::string, std::string, plg::case_insensitive_hash, plg::case_insensitive_equal> languages;

        for (const auto& pkg : packages) {
            if (pkg.GetState() == PackageState::Parsed && pkg.GetType() == PackageType::Module) {
                languages[pkg.GetLanguage()] = pkg.GetName();
            }
        }

        return languages;
    }

    // Prepare valid packages with filtering
    auto PrepareValidPackages(const auto& languages) {
        PackageCollection validPackages;
        validPackages.reserve(packages.size());

        for (auto& pkg : packages) {
            if (pkg.GetState() != PackageState::Parsed) {
                continue;
            }

            // Apply filters
            if (IsPackageFiltered(pkg)) {
                pkg.SetState(PackageState::Disabled);
                state->resolution.AddFailed();
                continue;
            }

            // Add language dependency for plugins
            if (pkg.GetType() == PackageType::Plugin) {
                auto it = languages.find(pkg.GetLanguage());
                if (it != languages.end()) {
                    pkg.AddDependency(it->second);
               }
            }

            pkg.StartOperation(PackageState::Resolving);
            validPackages.emplace_back(pkg.GetId(), pkg.GetManifest());
        }

        return validPackages;
    }

    // Check if package is filtered
    bool IsPackageFiltered(const PackageInfo& pkg) const {
        const auto& name = pkg.GetName();

        // Whitelist check
        if (config->whitelistedPackages &&
            !config->whitelistedPackages->contains(name)) {
            return true;
        }

        // Blacklist check
        if (config->blacklistedPackages &&
            config->blacklistedPackages->contains(name)) {
            return true;
        }

        return false;
    }

    // Phase 3: Dependency Resolution
    void ResolveDependencies() {
        PhaseScope scope(progressReporter.get(), "Resolution", state->resolution, packages.size());

        // Build language registry
        auto languages = BuildLanguageRegistry();

        // Apply filters and prepare valid packages
        auto validPackages = PrepareValidPackages(languages);

        // Resolve dependencies
        if (!validPackages.empty()) {
            ResolveDependencyGraph(validPackages);
        }
    }

    // Resolve dependency graph using resolver
    void ResolveDependencyGraph(const auto& validPackages) {
        auto processResult = [this](PackageInfo& pkg, bool success, const auto& issues) {
            if (success) {
                pkg.EndOperation(PackageState::Resolved);
                state->resolution.AddSuccess();
            } else {
                pkg.EndOperation(PackageState::Unresolved);
                state->resolution.AddFailed();

                // Add resolution issues if any
                auto it = issues.find(pkg.GetId());
                if (it != issues.end()) {
                    for (const auto& issue : it->second) {
                        auto details = issue.GetDetailedDescription();
                        if (issue.isBlocking) {
                            pkg.AddError(details);
                            state->AddError(std::format("Resolution: {}", pkg.GetName()), std::move(details));
                        } else {
                            pkg.AddWarning(details);
                            state->AddWarning(std::format("Resolution: {}", pkg.GetName()), std::move(details));
                        }
                    }
                }
            }

            ReportItemComplete("Resolution", pkg.GetName(), success);
        };
        // TODO: Catch???
        //try {
        // Call dependency resolver
        auto depReport = resolver->Resolve(validPackages);

        // Create resolved set for quick lookup
        std::unordered_set<UniqueId> resolvedSet(
            depReport.loadOrder.begin(),
            depReport.loadOrder.end()
        );

        // Update package states based on resolution
        for (auto& pkg : packages) {
            if (pkg.GetState() != PackageState::Resolving) {
                continue;
            }

            processResult(pkg, resolvedSet.contains(pkg.GetId()), depReport.issues);
        }

        // Store resolution results
        loadOrder = std::move(depReport.loadOrder);
        dependencyGraph = std::move(depReport.dependencyGraph);
        reverseDependencyGraph = std::move(depReport.reverseDependencyGraph);
        /*} catch (const std::exception& e) {
            // Resolution failed completely
            state->AddError("Resolution", e.what());

            // Mark all resolving packages as unresolved
            for (auto& pkg : packages) {
                if (pkg.GetState() == PackageState::Resolving) {
                    pkg.EndOperation(PackageState::Unresolved);
                    state->resolution.AddFailed();
                }
            }
        }*/

        // Debug print
        if (config->printLoadOrder) {
            PrintLoadOrder();
        }
        if (config->printDependencyGraph) {
            PrintDependencyGraph();
        }
        if (config->exportDigraphDot) {
            ExportDependencyGraphDOT(*config->exportDigraphDot);
        }
    }

    void PrintLoadOrder() const {
        std::string buffer;
        buffer.reserve(1024); // preallocate some space to reduce reallocs

        std::format_to(std::back_inserter(buffer), "=== Load Order ===\n");

        if (loadOrder.empty()) {
            std::format_to(std::back_inserter(buffer), "(empty)\n\n");
            LogSystem::Log(buffer, Color::None);
            return;
        }

        for (size_t i = 0; i < loadOrder.size(); ++i) {
            auto id = loadOrder[i];
            std::format_to(std::back_inserter(buffer), "{:3}: {} (id={})\n", i, packages[id].GetName(), id);
        }

        buffer.push_back('\n');
        LogSystem::Log(buffer, Color::None);
    }

    void PrintDependencyGraph() const {
        std::string buffer;
        buffer.reserve(2048);

        std::format_to(std::back_inserter(buffer), "=== Dependency Graph (pkg -> [deps]) ===\n");

        if (dependencyGraph.empty()) {
            std::format_to(std::back_inserter(buffer), "(empty)\n\n");
            LogSystem::Log(buffer, Color::None);
            return;
        }

        for (const auto& [pkgId, deps] : dependencyGraph) {
            std::format_to(std::back_inserter(buffer), "{} (id={}) -> ", packages[pkgId].GetName(), pkgId);
            if (deps.empty()) {
                std::format_to(std::back_inserter(buffer), "[]\n");
                continue;
            }

            buffer.push_back('[');
            bool first = true;
            for (auto d : deps) {
                if (!first) {
                    std::format_to(std::back_inserter(buffer), ", ");
                }
                std::format_to(std::back_inserter(buffer), "{} (id={})", packages[d].GetName(), d);
                first = false;
            }
            std::format_to(std::back_inserter(buffer), "]\n");
        }

        buffer.push_back('\n');
        LogSystem::Log(buffer, Color::None);
    }

    void ExportDependencyGraphDOT(const std::filesystem::path& outPath) const {
        std::string buffer;
        buffer.reserve(4096);

        std::format_to(std::back_inserter(buffer), "digraph packages {{\n");
        std::format_to(std::back_inserter(buffer), "  rankdir=LR;\n");

        // nodes
        for (size_t i = 0; i < packages.size(); ++i) {
            std::format_to(std::back_inserter(buffer),
                           "  node{} [label=\"{}\\n(id={})\"];\n",
                           i, packages[i].GetName(), i);
        }

        // edges
        for (const auto& [pkgId, deps] : dependencyGraph) {
            for (auto dep : deps) {
                std::format_to(std::back_inserter(buffer), "  node{} -> node{};\n", pkgId, dep);
            }
        }

        std::format_to(std::back_inserter(buffer), "}}\n");

        auto writeResult = fileSystem->WriteTextFile(outPath, buffer);
        if (!writeResult) {
            state->AddError("Export DOT", std::move(writeResult.error()));
        }
    }

    // Phase 4: Loading packages sequentially
    void LoadPackagesSequentially() {
        PhaseScope scope(progressReporter.get(), "Loading", state->load, loadOrder.size());

        std::unordered_set<UniqueId> failedPackages;

        for (const auto& pkgId : loadOrder) {
            LoadSinglePackage(packages[pkgId], failedPackages);
            // Stop on critical failure if configured
            if (config_->stopOnCriticalFailure &&
                failedPackages.contains(pkgId) &&
                packages[pkgId].GetType() == PackageType::Module) {

                PL_LOG_ERROR("[CRITICAL] Language module {} failed to load", packages[pkgId].GetName());
                break;
            }
        }
    }

    // Load a single package with dependency checking
    void LoadSinglePackage(
        PackageInfo& pkg,
        std::unordered_set<UniqueId>& failedPackages) {

        // Check if dependencies failed
        auto failedDeps = GetFailedDependencies(pkg.GetId(), failedPackages);
        if (!failedDeps.empty()) {
            HandleSkippedPackage(pkg, failedDeps, failedPackages);
            return;
        }

        // Start loading operation
        pkg.StartOperation(PackageState::Loading);

        try {
            // Attempt to load the package
            auto loadResult = LoadPackage(pkg);

            // Success
            pkg.SetInstance(std::move(loadResult));
            pkg.EndOperation(PackageState::Loaded);
            state->load.AddSuccess();
            loadedPackages.push_back(pkg.GetId());

            // Check for slow loading
            if (config->enableDetailedLogging) {
                auto startTime = pkg.GetOperationTime(PackageState::Loading);
                if (startTime > 1000ms) {
                    PL_LOG_WARNING("Package {} took {}ms to start",
                        pkg.GetName(), startTime.count());
                }
            }

            ReportItemComplete("Loading", pkg.GetName(), true);

        } catch (const std::exception& e) {
            HandleFailedPackage(pkg, e.what(), failedPackages);
        }
    }

    // Get list of failed dependencies
    std::vector<std::string_view> GetFailedDependencies(
        UniqueId pkgId,
        const std::unordered_set<UniqueId>& failedPackages) const {

        std::vector<std::string_view> failedDeps;

        if (auto it = dependencyGraph.find(pkgId); it != dependencyGraph.end()) {
            for (const auto& depId : it->second) {
                if (failedPackages.contains(depId)) {
                    failedDeps.push_back(packages[depId].GetName());
                }
            }
        }

        return failedDeps;
    }

    // Handle skipped package due to failed dependencies
    void HandleSkippedPackage(
        PackageInfo& pkg,
        const std::vector<std::string_view>& failedDeps,
        std::unordered_set<UniqueId>& failedPackages) {

        pkg.SetState(PackageState::Skipped);

        for (const auto& dep : failedDeps) {
            pkg.AddError(std::format("Dependency failed: {}", dep));
        }

        state->load.AddFailed();
        failedPackages.insert(pkg.GetId());

        ReportItemComplete("Loading", pkg.GetName(), false);
    }

    // Handle failed package loading
    void HandleFailedPackage(
        PackageInfo& pkg,
        std::string_view error,
        std::unordered_set<UniqueId>& failedPackages) {

        pkg.EndOperation(PackageState::Failed);

        auto errorMsg = std::format("Load failed: {}", error);
        pkg.AddError(errorMsg);
        state->AddError(std::format("Loading: {}", pkg.GetName()), errorMsg);

        state->load.AddFailed();
        failedPackages.insert(pkg.GetId());

        // before loading package I check failed deps via GetFailedDependencies
        //PropagateFailure(pkg.GetId(), failedPackages);

        ReportItemComplete("Loading", pkg.GetName(), false);
    }

    std::shared_ptr<IPackage> LoadPackage(PackageInfo& pkg) {
        switch (pkg.GetType()) {
            case PackageType::Module:
                auto module = std::shared_ptr<Module>(pkg.GetId(), pkg.GetManifest());

                // Load module
                auto result = module->Initialize(plugify);
                if (!result) {
                    throw std::runtime_error(result.error());
                }

                // Store for plugin loading
                loadedModules[pkg.GetLanguage()] = module;
                return module;

            case PackageType::Plugin:
                // Find language module (must exist due to dependency order)
                auto moduleIt = loadedModules.find(pkg.GetLanguage());
                if (moduleIt == loadedModules.end()) {
                    throw std::runtime_error(
                        std::format("Language module '{}' not found", pkg.GetLanguage()));
                }

                auto plugin = std::dynamic_pointer_cast<Plugin>(pkg.GetId(), pkg.GetManifest());

                // Set the language module
                plugin->SetModule(moduleIt->second);

                // Load plugin
                auto result = plugin->Intitialize(plugify);
                if (!result) {
                    throw std::runtime_error(result.error());
                }

                return plugin;

            default:
                throw std::runtime_error("Unknown package type");
        }
    }

    // Phase 5: Starting packages sequentially
    /*void StartPackagesSequentially() {
        ReportPhaseStart("Starting", loadedPackages.size());

        for (const auto& pkgId : loadedPackages) {
            StartSinglePackage(packages[pkgId]);
        }

        ReportPhaseComplete("Starting", state->start);
    }

    // Start a single package
    void StartSinglePackage(PackageInfo& pkg) {
        pkg.StartOperation(PackageState::Starting);

        try {
            // Attempt to start the package
            if (!pkg.GetInstance()) {
                throw std::runtime_error("Package instance is null");
            }

            bool started = pkg.GetInstance()->Start();

            if (!started) {
                throw std::runtime_error("Start() returned false");
            }

            // Success
            pkg.EndOperation(PackageState::Started);
            state->start.AddSuccess();

            // Check for slow start
            if (config->enableDetailedLogging) {
                auto startTime = pkg.GetOperationTime(PackageState::Starting);
                if (startTime > 500ms) {
                    PL_LOG_WARNING("Package {} took {}ms to start",
                        pkg.GetName(), startTime.count());
                }
            }

            ReportItemComplete("Starting", pkg.GetName(), true);

        } catch (const std::exception& e) {
            HandleFailedStart(pkg, e.what());
        }
    }

    // Handle failed package start
    void HandleFailedStart(
        PackageInfo& pkg,
        std::string_view error) {

        pkg.EndOperation(PackageState::Stalled);

        auto errorMsg = std::format("Start failed: {}", error);
        pkg.AddError(errorMsg);
        state->AddError(std::format("Starting: {}", pkg.GetName()), errorMsg);

        state->start.AddFailed();

        // Note: We don't propagate failure here because dependents
        // are already loaded. You might want to handle this differently
        // based on your requirements.

        ReportItemComplete("Starting", pkg.GetName(), false);

        // Optionally unload the package if it failed to start
        if (config->unloadOnStartFailure && pkg.GetInstance()) {
            try {
                pkg.GetInstance()->Unload();
                pkg.SetInstance(nullptr);
            } catch (const std::exception& e) {
                PL_LOG_ERROR("Failed to unload stalled package {}: {}",
                           pkg.GetName(), e.what());
            }
        }
    }*/

    // Propagate failure to dependent packages
    /*void PropagateFailure(
        UniqueId failedPkg,
        std::unordered_set<UniqueId>& failedSet) {

        // Use iterative approach to avoid stack overflow
        std::stack<UniqueId> toProcess;
        toProcess.push(failedPkg);

        while (!toProcess.empty()) {
            UniqueId current = toProcess.top();
            toProcess.pop();

            // Find all packages that depend on the current failed package
            if (auto it = reverseDependencyGraph.find(current);
                it != reverseDependencyGraph.end()) {

                for (const auto& dependent : it->second) {
                    auto [iter, inserted] = failedSet.insert(dependent);
                    // Mark as failed if not already
                    if (inserted) {
                        toProcess.push(dependent);

                        // Update package state if it hasn't been processed yet
                        auto& pkg = packages[dependent];
                        if (pkg.GetState() == PackageState::Resolved) {
                            pkg.SetState(PackageState::Skipped);
                            pkg.AddError(
                                std::format("Dependency {} failed", packages[current].GetName())
                            );
                        }
                    }
                }
            }
        }
    }*/

    // Helper methods for reporting
    /*void ReportPhaseStart(std::string_view phase, size_t count) {
        if (progressReporter) {
            progressReporter->OnPhaseStart(phase, count);
        }
    }

    void ReportPhaseComplete(std::string_view phase, const PhaseStatistics& stats) {
        if (progressReporter) {
            progressReporter->OnPhaseComplete(phase, stats.success.load(), stats.failed.load());
        }
    }

    void ReportItemComplete(std::string_view phase, std::string_view item, bool success) {
        if (progressReporter) {
            progressReporter->OnItemComplete(phase, item, success);
        }
    }*/

private:
    std::shared_ptr<IFileSystem> fileSystem;
    std::shared_ptr<IManifestParser> manifestParser;
    std::shared_ptr<IDependencyResolver> resolver;
    //std::shared_ptr<IPackageLoader> loader;
    std::shared_ptr<IProgressReporter> progressReporter;

    //ManagerConfig config;
    plg::synced_stream syncOut;
    plg::thread_pool<> threadPool;

    std::vector<PackageInfo> packages;
    //std::mutex mutex;

    std::unordered_set<UniqueId> failedPackages;
    std::vector<UniqueId> loadedPackages;

    // Track loaded language modules for plugin loading
    plg::flat_map<std::string, std::shared_ptr<Module>, plg::case_insensitive_hash, plg::case_insensitive_equal> loadedModules;

    plg::flat_map<UniqueId, std::vector<UniqueId>> dependencyGraph;
    plg::flat_map<UniqueId, std::vector<UniqueId>> reverseDependencyGraph;

    std::vector<UniqueId> loadOrder;
    std::vector<UniqueId> startOrder;
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
    std::cout << "Discovered:    " << state->totalDiscovered << std::endl;
    std::cout << "Validated:     " << state->validationPassed << std::endl;
    std::cout << "Failed Valid:  " << state->validationFailed << std::endl;
    std::cout << "Resolved:      " << state->dependencyResolved << std::endl;
    std::cout << "Loaded:        " << state->loaded << std::endl;
    std::cout << "Started:       " << state->started << std::endl;
    std::cout << "Failed:        " << state->failed << std::endl;
    std::cout << "Skipped:       " << state->skipped << std::endl;

    std::cout << "\n=== Timing ===" << std::endl;
    std::cout << "Discovery:     " << state->totalDiscoveryTime.count() << "ms" << std::endl;
    std::cout << "Validation:    " << state->totalValidationTime.count() << "ms" << std::endl;
    std::cout << "Resolution:    " << state->totalResolutionTime.count() << "ms" << std::endl;
    std::cout << "Loading:       " << state->totalLoadTime.count() << "ms" << std::endl;
    std::cout << "Starting:      " << state->totalStartTime.count() << "ms" << std::endl;

    if (!state->errorLog.empty()) {
        std::cout << "\n=== Errors ===" << std::endl;
        for (const auto& [context, error] : state->errorLog) {
            std::cout << "[" << context << "] " << error << std::endl;
        }
    }

    // Run your application...

    // Cleanup
    std::cout << "\n=== Shutting down ===" << std::endl;
    Manager->terminate();
}
*/


Manager::Manager(Plugify& plugify) : _impl(std::make_unique<Impl>(plugify)) {}

Manager::~Manager() = default;

std::shared_ptr<InitializationState> Manager::Initialize() {
    _impl->config = _impl->plugify.GetConfig();
    return _impl->Initialize();
}



// ============================================================================
// Dependency Injection
// ============================================================================

// Set progress reporter
/*void Manager::setProgressReporter(std::shared_ptr<IProgressReporter> reporter) {
    progressReporter = reporter;
}

void Manager::setPackageDiscovery(std::unique_ptr<IPackageDiscovery> discovery) {
    _impl->packageDiscovery = std::move(discovery);
}

void Manager::setPackageValidator(std::unique_ptr<IPackageValidator> validator) {
    _impl->packageValidator = std::move(validator);
}

void Manager::SetDependencyResolver(std::unique_ptr<IDependencyResolver> resolver) {
    _impl->dependencyResolver = std::move(resolver);
}

/*void Manager::setModuleLoader(std::unique_ptr<IModuleLoader> loader) {
    _impl->moduleLoader = std::move(loader);
}

void Manager::setPluginLoader(std::unique_ptr<IPluginLoader> loader) {
    _impl->pluginLoader = std::move(loader);
}*/

