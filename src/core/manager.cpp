#pragma once

#include <any>

#include "plugify/asm/assembly_loader.hpp"
#include "plugify/core/config.hpp"
#include "plugify/core/manifest.hpp"
#include "plugify/core/module.hpp"
#include "plugify/core/package.hpp"
#include "plugify/core/plugify.hpp"
#include "plugify/core/plugin.hpp"
#include "plugify/core/progress_reporter.hpp"

#include "core/loader.hpp"
#include "asm/defer.hpp"
#include "plg/thread_pool.hpp"

using namespace plugify;
using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

// ============================================================================
// Stage Interfaces
// ============================================================================
enum class StageType {
    Transform,    // Parallel processing, no order changes
    Barrier,      // Can reorder/filter the container
    Sequential,   // Processes in container order
    Batch        // Parallel processing in controlled batches
};

// Execution context
template<typename T>
struct ExecutionContext {
    plg::thread_pool<>& threadPool;
    std::shared_ptr<IProgressReporter> reporter;
    //std::shared_ptr<MetricsCollector> metrics;
    std::shared_ptr<Config> config;
};

// Base interface
template<typename T>
class IStage {
public:
    virtual ~IStage() = default;
    virtual std::string_view GetName() const = 0;
    virtual StageType GetType() const = 0;
};

// Transform stage - processes items in place
template<typename T>
class ITransformStage : public IStage<T> {
public:
    StageType GetType() const override { return StageType::Transform; }

    // Process single item
    virtual Result<void> ProcessItem(
        T& item,
        const ExecutionContext<T>& ctx) = 0;

    // Optional: filter predicate
    virtual bool ShouldProcess([[maybe_unused]] const T& item) const { return true; }

    // Optional: setup/teardown
    virtual void Setup(
        [[maybe_unused]] std::span<T> items,
        [[maybe_unused]] const ExecutionContext<T>& ctx
    ) {}
    virtual void Teardown(
        [[maybe_unused]] std::span<T> items,
        [[maybe_unused]] const ExecutionContext<T>& ctx
    ) {}
};

// Barrier stage - can reorder and filter the container
template<typename T>
class IBarrierStage : public IStage<T> {
public:
    StageType GetType() const override { return StageType::Barrier; }

    // Process all items, return new container
    // This allows the stage to reorder, filter, or even add items
    virtual Result<std::vector<T>> ProcessAll(
        std::vector<T> items,
        const ExecutionContext<T>& ctx) = 0;
};

// Sequential stage - processes in container order
template<typename T>
class ISequentialStage : public IStage<T> {
public:
    StageType GetType() const override { return StageType::Sequential; }

    // Process item with knowledge of its position
    virtual Result<void> ProcessItem(
        T& item,
        size_t position,
        size_t total,
        const ExecutionContext<T>& ctx) = 0;

    // Should we continue after an error?
    virtual bool ContinueOnError() const { return true; }

    // Optional: filter
    virtual bool ShouldProcess([[maybe_unused]] const T& item) const { return true; }
};

// Batch stage - processes items in parallel batches
template <typename T>
class IBatchStage : public IStage<T> {
public:

    StageType GetType() const override {
        return StageType::Batch;
    }

    // Process a batch of items (called for each batch)
    virtual Result<void> ProcessBatch(
        std::span<T> batch,
        size_t batchIndex,
        size_t totalBatches,
        const ExecutionContext<T>& ctx
    ) = 0;

    // Get batch size
    virtual size_t GetBatchSize() const {
        return 10;
    }

    // Process items within batch in parallel?
    virtual bool ProcessBatchInParallel() const {
        return true;
    }

    // Optional: Process individual item within batch (for parallel processing)
    virtual Result<void> ProcessItem(
        [[maybe_unused]] T& item,
        [[maybe_unused]] const ExecutionContext<T>& ctx
    ) {
        return {};  // Default: no per-item processing
    }

    // Optional: filter
    virtual bool ShouldProcess([[maybe_unused]] const T& item) const {
        return true;
    }

    // Optional: Setup/teardown for entire stage
    virtual void Setup(
        [[maybe_unused]] std::span<T> items,
        [[maybe_unused]] const ExecutionContext<T>& ctx
    ) {
    }

    virtual void Teardown(
        [[maybe_unused]] std::span<T> items,
        [[maybe_unused]] const ExecutionContext<T>& ctx
    ) {
    }

    // Optional: Setup/teardown for each batch
    virtual void SetupBatch(
        [[maybe_unused]] std::span<T> batch,
        [[maybe_unused]] size_t batchIndex,
        [[maybe_unused]] const ExecutionContext<T>& ctx
    ) {
    }

    virtual void TeardownBatch(
        [[maybe_unused]] std::span<T> batch,
        [[maybe_unused]] size_t batchIndex,
        [[maybe_unused]] const ExecutionContext<T>& ctx
    ) {
    }
};

// ============================================================================
// FlexiblePipeline Implementation
// ============================================================================

// Stage statistics
struct StageStatistics {
    size_t itemsIn = 0;
    size_t itemsOut = 0;
    size_t succeeded = 0;
    size_t failed = 0;
    Duration elapsed{0};
    std::vector<std::string> errors;
};

template<typename T>
class FlexiblePipeline {
public:
    struct PipelineReport {
        std::vector<std::pair<std::string_view, StageStatistics>> stages;
        Duration totalTime{0};
        size_t initialItems = 0;
        size_t finalItems = 0;

        void Print() const {
            std::println("=== Pipeline Report ===");
            std::println("Items: {} -> {} ({}ms total)",
                       initialItems, finalItems, totalTime.count());

            for (size_t i = 0; i < stages.size(); ++i) {
                const auto& [n, s] = stages[i];
                std::println("  Stage {} ({}): {} -> {} items, {} succeeded, {} failed ({}ms)",
                           i, n, s.itemsIn, s.itemsOut, s.succeeded, s.failed, s.elapsed.count());

                if (!s.errors.empty()) {
                    for (const auto& err : s.errors) {
                        std::println("    ERROR: {}", err);
                    }
                }
            }
        }
    };

    // Builder pattern for pipeline construction
    class Builder {
        friend class FlexiblePipeline;

        struct StageEntry {
            std::unique_ptr<IStage<T>> stage;
            bool required = true;
        };

        std::vector<StageEntry> stages_;
        std::shared_ptr<IProgressReporter> reporter_;
        //std::shared_ptr<MetricsCollector> metrics_;
        size_t threadPoolSize_ = std::thread::hardware_concurrency();
        std::shared_ptr<Config> config_;

    public:
        template<typename StageType>
        Builder& AddStage(std::unique_ptr<StageType> stage, bool required = true) {
            static_assert(std::is_base_of_v<IStage<T>, StageType>);
            stages_.push_back({std::move(stage), required});
            return *this;
        }

        Builder& WithReporter(std::shared_ptr<IProgressReporter> reporter) {
            reporter_ = std::move(reporter);
            return *this;
        }

        /*Builder& WithMetrics(std::shared_ptr<MetricsCollector> metrics) {
            metrics_ = std::move(metrics);
            return *this;
        }*/

        Builder& WithThreadPoolSize(size_t size) {
            threadPoolSize_ = size;
            return *this;
        }

        Builder& WithConfig(std::shared_ptr<Config> config) {
            config_ = std::move(config);
            return *this;
        }

        std::unique_ptr<FlexiblePipeline> Build() {
            return std::unique_ptr<FlexiblePipeline>(new FlexiblePipeline(std::move(*this)));
        }
    };

    static Builder Create() { return Builder{}; }

private:
    FlexiblePipeline(Builder&& builder)
        : threadPool_(builder.threadPoolSize_)
        , reporter_(std::move(builder.reporter_))
        //, metrics_(std::move(builder.metrics_))
        , config_(std::move(builder.config_)) {

        for (auto& [stage, required] : builder.stages_) {
            stages_.push_back({std::move(stage), required});
        }
    }

public:
    // Execute pipeline - container is modified in place
    std::pair<std::vector<T>, PipelineReport> Execute(std::vector<T> items) {
        PipelineReport report;
        report.stages.reserve(stages_.size());
        report.initialItems = items.size();

        auto pipelineStart = Clock::now();

        // Create execution context
        ExecutionContext<T> ctx{
            .threadPool = threadPool_,
            .reporter = reporter_,
            //.metrics = metrics_,
            .config = config_
        };

        // Execute each stage
        for (const auto& [stage, required] : stages_) {
            auto stats = ExecuteStage(stage.get(), items, ctx);
            report.stages.emplace_back(stage->GetName(), stats);

            // Check if we should continue
            if (stats.failed > 0 && required) {
                PL_LOG_ERROR("Required stage '{}' had failures, stopping pipeline",
                           stage->GetName());
                break;
            }
        }

        report.finalItems = items.size();
        report.totalTime = std::chrono::duration_cast<Duration>(
            Clock::now() - pipelineStart
        );

        return {std::move(items), report};
    }

private:
    StageStatistics ExecuteStage(
        IStage<T>* stage,
        std::vector<T>& items,
        const ExecutionContext<T>& ctx) {

        StageStatistics stats;
        stats.itemsIn = items.size();
        auto startTime = Clock::now();

        if (reporter_) {
            reporter_->OnPhaseStart(stage->GetName(), items.size());
        }

        // Execute based on stage type
        switch (stage->GetType()) {
            case StageType::Transform:
                ExecuteTransform(static_cast<ITransformStage<T>*>(stage), items, stats, ctx);
                break;

            case StageType::Barrier:
                ExecuteBarrier(static_cast<IBarrierStage<T>*>(stage), items, stats, ctx);
                break;

            case StageType::Sequential:
                ExecuteSequential(static_cast<ISequentialStage<T>*>(stage), items, stats, ctx);
                break;

            case StageType::Batch:
                ExecuteBatch(static_cast<IBatchStage<T>*>(stage), items, stats, ctx);
                break;
        }

        stats.itemsOut = items.size();
        stats.elapsed = std::chrono::duration_cast<Duration>(Clock::now() - startTime);

        if (reporter_) {
            reporter_->OnPhaseComplete(stage->GetName(), stats.succeeded, stats.failed);
        }

        return stats;
    }

    void ExecuteTransform(
        ITransformStage<T>* stage,
        std::vector<T>& items,
        StageStatistics& stats,
        const ExecutionContext<T>& ctx) {

        // Setup
        std::span<T> itemSpan(items);
        stage->Setup(itemSpan, ctx);

        // Process items in parallel
        std::atomic<size_t> succeeded{0};
        std::atomic<size_t> failed{0};
        std::mutex errorMutex;

        threadPool_.submit_sequence(0, items.size(), [&](size_t i) {
            auto& item = items[i];

            if (!stage->ShouldProcess(item)) {
                return;
            }

            auto result = stage->ProcessItem(item, ctx);

            if (result.has_value()) {
                succeeded.fetch_add(1, std::memory_order_relaxed);
                ReportItemComplete(stage->GetName(), item, true);
            } else {
                failed.fetch_add(1, std::memory_order_relaxed);
                {
                    std::lock_guard lock(errorMutex);
                    stats.errors.push_back(
                        std::format("{}: {}", GetItemName(item), result.error())
                    );
                }
                ReportItemComplete(stage->GetName(), item, false);
            }
        }).wait();

        stats.succeeded = succeeded.load();
        stats.failed = failed.load();

        // Teardown
        stage->Teardown(itemSpan, ctx);
    }

    void ExecuteBarrier(
        IBarrierStage<T>* stage,
        std::vector<T>& items,
        StageStatistics& stats,
        const ExecutionContext<T>& ctx) {

        // Process all items together
        auto result = stage->ProcessAll(std::move(items), ctx);

        if (result.has_value()) {
            // Replace container with processed result
            items = std::move(*result);
            stats.succeeded = items.size();

            // Report all items as processed
            for (const auto& item : items) {
                ReportItemComplete(stage->GetName(), item, true);
            }
        } else {
            stats.failed = items.size();
            stats.errors.push_back(result.error());

            // Report all items as failed
            for (const auto& item : items) {
                ReportItemComplete(stage->GetName(), item, false);
            }
        }
    }

    void ExecuteSequential(
        ISequentialStage<T>* stage,
        std::vector<T>& items,
        StageStatistics& stats,
        const ExecutionContext<T>& ctx) {

        bool continueOnError = stage->ContinueOnError();
        size_t total = items.size();

        for (size_t i = 0; i < items.size(); ++i) {
            auto& item = items[i];

            if (!stage->ShouldProcess(item)) {
                continue;
            }

            // Stop if error occurred and stage doesn't continue on error
            if (stats.failed > 0 && !continueOnError) {
                break;
            }

            auto result = stage->ProcessItem(item, i, total, ctx);

            if (result.has_value()) {
                stats.succeeded++;
                ReportItemComplete(stage->GetName(), item, true);
            } else {
                stats.failed++;
                stats.errors.push_back(
                    std::format("{}: {}", GetItemName(item), result.error())
                );
                ReportItemComplete(stage->GetName(), item, false);
            }
        }
    }

    void ExecuteBatch(
        IBatchStage<T>* stage,
        std::vector<T>& items,
        StageStatistics& stats,
        const ExecutionContext<T>& ctx) {

        // Setup
        std::span<T> itemSpan(items);
        stage->Setup(itemSpan, ctx);

        size_t batchSize = stage->GetBatchSize();
        size_t numBatches = (items.size() + batchSize - 1) / batchSize;
        bool processInParallel = stage->ProcessBatchInParallel();

        std::atomic<size_t> succeeded{0};
        std::atomic<size_t> failed{0};
        std::mutex errorMutex;

        // Process each batch
        for (size_t batchIdx = 0; batchIdx < numBatches; ++batchIdx) {
            size_t start = batchIdx * batchSize;
            size_t end = std::min(start + batchSize, items.size());
            std::span<T> batch(&items[start], end - start);

            // Setup batch
            stage->SetupBatch(batch, batchIdx, ctx);

            // Process batch
            auto batchResult = stage->ProcessBatch(batch, batchIdx, numBatches, ctx);

            if (batchResult.has_value()) {
                // If batch processing succeeded, optionally process items within batch
                if (processInParallel) {
                    // Process items within batch in parallel
                    threadPool_.submit_sequence(0, batch.size(), [&](size_t i) {
                        auto& item = batch[i];

                        if (!stage->ShouldProcess(item)) {
                            return;
                        }

                        auto result = stage->ProcessItem(item, ctx);

                        if (result.has_value()) {
                            succeeded.fetch_add(1, std::memory_order_relaxed);
                            ReportItemComplete(stage->GetName(), item, true);
                        } else {
                            failed.fetch_add(1, std::memory_order_relaxed);
                            {
                                std::lock_guard lock(errorMutex);
                                stats.errors.push_back(
                                    std::format("{}: {}", GetItemName(item), result.error())
                                );
                            }
                            ReportItemComplete(stage->GetName(), item, false);
                        }
                    }).wait();
                } else {
                    // Process items within batch sequentially
                    for (auto& item : batch) {
                        if (!stage->ShouldProcess(item)) {
                            continue;
                        }

                        auto result = stage->ProcessItem(item, ctx);

                        if (result.has_value()) {
                            succeeded.fetch_add(1, std::memory_order_relaxed);
                            ReportItemComplete(stage->GetName(), item, true);
                        } else {
                            failed.fetch_add(1, std::memory_order_relaxed);
                            stats.errors.push_back(
                                std::format("{}: {}", GetItemName(item), result.error())
                            );
                            ReportItemComplete(stage->GetName(), item, false);
                        }
                    }
                }
            } else {
                // Batch processing failed
                failed.fetch_add(batch.size());
                {
                    std::lock_guard lock(errorMutex);
                    stats.errors.push_back(
                        std::format("Batch {} failed: {}", batchIdx, batchResult.error())
                    );
                }

                // Report all items in batch as failed
                for (const auto& item : batch) {
                    ReportItemComplete(stage->GetName(), item, false);
                }
            }

            // Teardown batch
            stage->TeardownBatch(batch, batchIdx, ctx);

            // Optional: Add delay between batches if needed
            if (batchIdx < numBatches - 1) {
                // Could add a virtual method for inter-batch delay
                // std::this_thread::sleep_for(stage->GetInterBatchDelay());
            }
        }

        stats.succeeded = succeeded.load();
        stats.failed = failed.load();

        // Teardown
        stage->Teardown(itemSpan, ctx);
    }

    void ReportItemComplete(std::string_view stage, const T& item, bool success) {
        if (reporter_) {
            reporter_->OnItemComplete(stage, GetItemName(item), success);
        }
    }

    static std::string_view GetItemName(const T& item) {
        if constexpr (requires { item.GetName(); }) {
            return item.GetName();
        } else if constexpr (requires { item.name; }) {
            return item.name;
        } else {
            return "item";
        }
    }

private:
    struct StageEntry {
        std::unique_ptr<IStage<T>> stage;
        bool required;
    };

    std::vector<StageEntry> stages_;
    plg::thread_pool<> threadPool_;
    std::shared_ptr<IProgressReporter> reporter_;
    //std::shared_ptr<MetricsCollector> metrics_;
    std::shared_ptr<Config> config_;
};

// ============================================================================
// Concrete Stage Implementations
// ============================================================================

// Parsing Stage - Transform type
class ParsingStage : public ITransformStage<Package> {
    std::shared_ptr<IManifestParser> parser_;
    std::shared_ptr<IFileSystem> fileSystem_;

public:
    ParsingStage(std::shared_ptr<IManifestParser> parser,
                 std::shared_ptr<IFileSystem> fs)
        : parser_(std::move(parser)), fileSystem_(std::move(fs)) {}

    std::string_view GetName() const override { return "Parsing"; }

    bool ShouldProcess(const Package& item) const override {
        return item.GetState() == PackageState::Discovered;
    }

    std::expected<void, std::string> ProcessItem(
        Package& pkg,
        [[maybe_unused]] const ExecutionContext<Package>& ctx) override {

        pkg.StartOperation(PackageState::Parsing);

        auto content = fileSystem_->ReadTextFile(pkg.GetPath());
        if (!content) {
            pkg.AddError(content.error());
            pkg.EndOperation(PackageState::Corrupted);
            return std::unexpected(std::move(content.error()));
        }

        auto manifest = parser_->Parse(*content, pkg.GetType());
        if (!manifest) {
            pkg.AddError(manifest.error());
            pkg.EndOperation(PackageState::Corrupted);
            return std::unexpected(std::move(manifest.error()));
        }

        pkg.SetManifest(std::move(*manifest));
        pkg.EndOperation(PackageState::Parsed);

        return {};
    }
};

// Resolution Stage - Barrier type (reorders and filters)
class ResolutionStage : public IBarrierStage<Package> {
    std::shared_ptr<IDependencyResolver> resolver_;
    plg::flat_map<UniqueId, std::vector<UniqueId>>* dependencyGraph_;
    plg::flat_map<UniqueId, std::vector<UniqueId>>* reverseDependencyGraph_;
    std::optional<std::unordered_set<std::string>> whitelist_;
    std::optional<std::unordered_set<std::string>> blacklist_;
    
public:
    ResolutionStage(std::shared_ptr<IDependencyResolver> resolver,
                    plg::flat_map<UniqueId, std::vector<UniqueId>>* depGraph,
                    plg::flat_map<UniqueId, std::vector<UniqueId>>* reverseDepGraph)
        : resolver_(std::move(resolver))
        , dependencyGraph_(depGraph)
        , reverseDependencyGraph_(reverseDepGraph) {}
    
    std::string_view GetName() const override { return "Resolution"; }
    
    void SetFilters(std::optional<std::unordered_set<std::string>> white,
                    std::optional<std::unordered_set<std::string>> black) {
        whitelist_ = std::move(white);
        blacklist_ = std::move(black);
    }
    
    std::expected<std::vector<Package>, std::string> ProcessAll(
        std::vector<Package> items,
        [[maybe_unused]] const ExecutionContext<Package>& ctx
    ) override {
        // Step 1: Filter packages based on whitelist/blacklist
        std::vector<Package> filtered;
        std::vector<Package> excluded;

        filtered.reserve(items.size());
        //excluded.reserve(items.size());
        
        for (auto&& pkg : items) {
            bool include = true;
            
            // Check parsed state
            if (pkg.GetState() != PackageState::Parsed) {
                include = false;
            }
            
            // Check whitelist
            if (include && whitelist_ && 
                !whitelist_->contains(pkg.GetName())) {
                include = false;
            }
            
            // Check blacklist
            if (include && blacklist_ && 
                blacklist_->contains(pkg.GetName())) {
                include = false;
            }
            
            if (include) {
                pkg.SetState(PackageState::Resolving);
                filtered.push_back(std::move(pkg));
            } else {
                pkg.SetState(PackageState::Disabled);
                pkg.AddWarning("Excluded due to policy");
                excluded.push_back(std::move(pkg));
            }
        }
        
        if (filtered.empty()) {
            return std::unexpected("No valid packages to resolve");
        }
        
        // Step 2: Build language registry and add language dependencies
        {
            plg::flat_map<std::string, UniqueId> languages;

            for (const auto& pkg : filtered) {
                if (pkg.GetType() == PackageType::Module) {
                    languages[pkg.GetLanguage()] = pkg.GetId();
                }
            }

            for (auto& pkg : filtered) {
                if (pkg.GetType() == PackageType::Plugin) {
                    if (auto it = languages.find(pkg.GetLanguage()); it != languages.end()) {
                        pkg.AddDependency(GetPackageName(filtered, it->second));
                    }
                }
            }
        }

        // Step 3: Resolve dependencies
        auto report = resolver_->Resolve(filtered);
        
        // Store dependency graphs
        *dependencyGraph_ = std::move(report.dependencyGraph);
        *reverseDependencyGraph_ = std::move(report.reverseDependencyGraph);
        
        // Step 4: Create ID to package mapping for efficient lookup
        plg::flat_map<UniqueId, Package> idToPackage;
        idToPackage.reserve(idToPackage.size());

        for (auto&& pkg : filtered) {
            idToPackage[pkg.GetId()] = std::move(pkg);
        }
        
        // Step 5: Build result in dependency order
        std::vector<Package> result;
        result.reserve(report.loadOrder.size() + excluded.size());
        
        // Add resolved packages in load order
        for (auto id : report.loadOrder) {
            if (auto it = idToPackage.find(id); it != idToPackage.end()) {
                auto& pkg = it->second;
                pkg.SetState(PackageState::Resolved);
                result.push_back(std::move(pkg));
                idToPackage.erase(it);
            }
        }
        
        // Add unresolved packages at the end
        for (auto& [id, pkg] : idToPackage) {
            pkg.SetState(PackageState::Unresolved);
            
            // Add resolution errors if any
            if (auto it = report.issues.find(id); it != report.issues.end()) {
                for (const auto& issue : it->second) {
                    if (issue.isBlocking) {
                        pkg.AddError(issue.GetDetailedDescription());
                    } else {
                        pkg.AddWarning(issue.GetDetailedDescription());
                    }
                }
            }
            
            result.push_back(std::move(pkg));
        }
        
        // Add excluded packages at the very end
        result.insert(result.end(), 
                     std::make_move_iterator(excluded.begin()),
                     std::make_move_iterator(excluded.end()));
        
        return result;
    }
    
private:
    std::string GetPackageName(const std::vector<Package>& packages, UniqueId id) const {
        auto it = std::find_if(packages.begin(), packages.end(), [&](const Package& p) {
            return p.GetId() == id;
        });
        return it != packages.end() ? it->GetName() : "";
    }
};

// Example: Initialization Stage - Batch type for resource-limited operations
/*class InitializationStage : public IBatchStage<Package> {
    Plugify& plugify_;
    std::atomic<size_t> activeInitializations_{0};
    static constexpr size_t MAX_CONCURRENT_INITS = 4;

public:
    InitializationStage(Plugify& p) : plugify_(p) {}

    std::string_view GetName() const override { return "Initialization"; }

    size_t GetBatchSize() const override {
        return MAX_CONCURRENT_INITS;  // Limit concurrent initializations
    }

    bool ProcessBatchInParallel() const override { return true; }

    void SetupBatch(std::span<Package> batch, size_t batchIndex,
                    const ExecutionContext<Package>& ctx) override {
        PL_LOG_DEBUG("Initializing batch {} with {} packages", batchIndex, batch.size());
        activeInitializations_ = 0;
    }

    std::expected<void, std::string> ProcessBatch(
        std::span<Package> batch,
        size_t batchIndex,
        size_t totalBatches,
        const ExecutionContext<Package>& ctx) override {

        // Could perform batch-level resource allocation here
        // For example, pre-allocate shared resources for the batch

        return {};
    }

    void TeardownBatch(std::span<Package> batch, size_t batchIndex,
                      const ExecutionContext<Package>& ctx) override {
        // Wait for all initializations in batch to complete
        while (activeInitializations_.load() > 0) {
            std::this_thread::yield();
        }
    }
};*/

// Loading Stage - Sequential type
class LoadingStage : public ISequentialStage<Package> {
    Plugify& plugify_;
    plg::flat_map<std::string, std::shared_ptr<Module>> loadedModules_;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>* dependencyGraph_;
    
public:
    LoadingStage(Plugify& plugify,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>* deps)
        : plugify_(plugify), dependencyGraph_(deps) {}
    
    std::string_view GetName() const override { return "Loading"; }
    
    bool ContinueOnError() const override { return true; }
    
    bool ShouldProcess(const Package& item) const override {
        return item.GetState() == PackageState::Resolved;
    }
    
    std::expected<void, std::string> ProcessItem(
        Package& pkg,
        size_t position,
        size_t total,
        const ExecutionContext<Package>& ctx) override {
        
        pkg.StartOperation(PackageState::Loading);
        
        try {
            std::shared_ptr<void> instance;

            switch (pkg.GetType()) {
                case PackageType::Module: {
                    auto module = std::make_shared<Module>(pkg.GetId(), pkg.GetManifest());

                    auto result = Loader::LoadModule(provider, module);
                    if (!result) {
                        throw std::runtime_error(result.error());
                    }

                    loadedModules_[pkg.GetLanguage()] = module;
                    instance = std::move(module);
                    break;
                }

                case PackageType::Plugin: {
                    auto it = loadedModules_.find(pkg.GetLanguage());
                    if (it == loadedModules_.end()) {
                        throw std::runtime_error(
                            std::format("Language module '{}' not found", pkg.GetLanguage())
                        );
                    }

                    auto plugin = std::make_shared<Plugin>(pkg.GetId(), pkg.GetManifest());

                    auto result = Loader::LoadPlugin(it->second, plugin);
                    if (!result) {
                        throw std::runtime_error(result.error());
                    }

                    instance = std::move(plugin);
                    break;
                }

                default:
                    throw std::runtime_error("Unknown package type");
            }

            pkg.SetInstance(std::move(instance));

            pkg.EndOperation(PackageState::Loaded);
            
            // Log progress if verbose
            std::println("[{}/{}] Loaded {}", position + 1, total, pkg.GetName());
            
            return {};
            
        } catch (const std::exception& e) {
            pkg.AddError(e.what());
            pkg.EndOperation(PackageState::Failed);
            // TODO: propagate deps
            return std::unexpected(e.what());
        }
    }
};

struct Manager::Impl {
    Impl(std::shared_ptr<Context> context) {
        // Create services
        assemblyLoader = context->Get<IAssemblyLoader>();
        fileSystem = context->Get<IFileSystem>();
        manifestParser = context->Get<IManifestParser>();
        //validator = context->Get<PackageValidator>();
        resolver = context->Get<IDependencyResolver>();  // Your libsolv wrapper
        //loader = context->Get<DynamicPackageLoader>();
        progressReporter = context->Get<ConsoleProgressReporter>();
        //metricsCollector = context->Get<MetricsCollector>();

    }
    Plugify& plugify;
    std::shared_ptr<Config> config;
    
    // Services
    //std::shared_ptr<IAssemblyLoader> assemblyLoader;
    std::shared_ptr<IFileSystem> fileSystem;
    std::shared_ptr<IManifestParser> manifestParser;
    std::shared_ptr<IDependencyResolver> resolver;
    //std::shared_ptr<IPackageValidator> validator;
    //std::shared_ptr<IPackageRegistry> registry;
    std::shared_ptr<IProgressReporter> progressReporter;
    //std::shared_ptr<MetricsCollector> metricsCollector;
    
    // Dependency graphs (filled by resolution stage)
    plg::flat_map<UniqueId, std::vector<UniqueId>> dependencyGraph;
    plg::flat_map<UniqueId, std::vector<UniqueId>> reverseDependencyGraph;
    
public:
    std::shared_ptr<InitializationState> Initialize() {
        auto state = std::make_shared<InitializationState>();
        
        // Step 1: Quick discovery
        std::vector<Package> packages = DiscoverPackages();
        
        // Step 2: Create resolution stage with filters
        auto resolutionStage = std::make_unique<ResolutionStage>(
            resolver, &dependencyGraph, &reverseDependencyGraph
        );
        resolutionStage->SetFilters(config->whitelistedPackages, config->blacklistedPackages);
        
        // Step 3: Build pipeline
        auto pipeline = FlexiblePipeline<Package>::Create()
            // Parse manifests in parallel (Transform stage)
            .AddStage(std::make_unique<ParsingStage>(manifestParser, fileSystem))
            
            // Validate packages in batches (Batch stage)
            //.AddStage(std::make_unique<ValidationStage>(validator, 20), false)
            
            // Fetch metadata in controlled batches (Batch stage with rate limiting)
            //.AddStage(std::make_unique<NetworkFetchStage>(registry), false)
            
            // Resolve dependencies and reorder container (Barrier stage)
            .AddStage(std::move(resolutionStage))
            
            // Optional: Remove failed packages to save processing
            //.AddStage(std::make_unique<FilterFailedStage>(), false)
            
            // Initialize packages in batches (Batch stage for resource control)
            //.AddStage(std::make_unique<InitializationStage>(assemblyLoader), false)
            
            // Load packages in dependency order (Sequential stage)
            .AddStage(std::make_unique<LoadingStage>(plugify, &dependencyGraph))
            
            // Start packages in order (Sequential stage)
            //.AddStage(std::make_unique<StartingStage>())
            
            .WithReporter(progressReporter)
            //.WithMetrics(metricsCollector)
            .WithConfig(config)
            .WithThreadPoolSize(std::thread::hardware_concurrency())
            .Build();
        
        // Step 4: Execute pipeline
        auto [finalPackages, report] = pipeline->Execute(std::move(packages));
        
        // Step 5: Store results
        packages_ = std::move(finalPackages);
        
        // Convert report to state
        //ConvertReportToState(report, state);
        
        // Print reports if configured
        if (config->printReport) {
            report.Print();
        }
        
        return state;
    }
    
private:
    std::vector<Package> packages_;
    
    static constexpr std::array<std::string_view, 2> MANIFEST_EXTENSIONS = {
        "*.pplugin"sv, "*.pmodule"sv
    };

    // Collect manifest paths from all search directories
    auto CollectManifestPaths() {
        auto files = fileSystem->FindFiles(
            searchPath,
            MANIFEST_EXTENSIONS,
            true
        );

        if (!files) {
            PL_LOG_ERROR("Error searching '{}': {}", searchPath.string(), files.error());
            return {};
        }

        return *files;
    }

    // Step 1: Find all manifest files in parallel
    std::vector<Package> DiscoverPackages() {
        std::vector<Package> packages;

        // Collect all paths
        auto paths = CollectManifestPaths();

        // Create package entries
        packages.reserve(paths.size());
        for (auto&& path : paths) {
            packages.emplace_back(id++, std::move(path));
        }

        return packages;
    }
};

Manager::Manager(std::shared_ptr<Context> context)
    : _impl(std::make_unique<Impl>(std::move(context))) {}

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

