#pragma once

#include <any>

#include "plugify/asm/assembly_loader.hpp"
#include "plugify/core/config.hpp"
#include "plugify/core/manifest.hpp"
#include "plugify/core/extension.hpp"
#include "plugify/core/plugify.hpp"
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
    //std::shared_ptr<Config> config;
};

// Base interface
template<typename T>
class IStage {
public:
    virtual ~IStage() = default;

    virtual std::string_view GetName() const = 0;

    virtual StageType GetType() const = 0;

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

// Shared failure tracker that can be passed between stages
class FailureTracker {
    std::unordered_set<UniqueId> failedExtensions_;
    mutable std::shared_mutex mutex_;

public:
    FailureTracker(size_t capacity) {
        failedExtensions_.reserve(capacity);
    }

    void MarkFailed(UniqueId id) {
        std::unique_lock lock(mutex_);
        failedExtensions_.insert(id);
    }

    bool HasFailed(UniqueId id) const {
        std::shared_lock lock(mutex_);
        return failedExtensions_.contains(id);
    }

    bool HasAnyDependencyFailed(
        const Extension& ext,
        const plg::flat_map<UniqueId, std::vector<UniqueId>>& reverseDeps) const {

        std::shared_lock lock(mutex_);

        // Check if any of this extension's dependencies have failed
        if (auto it = reverseDeps.find(ext.GetId()); it != reverseDeps.end()) {
            for (const auto& depId : it->second) {
                if (failedExtensions_.contains(depId)) {
                    return true;
                }
            }
        }
        return false;
    }

    std::string_view GetFailedDependencyName(
        const Extension& ext,
        const plg::flat_map<UniqueId, std::vector<UniqueId>>& reverseDeps,
        std::span<const Extension> allExtensions) const {

        std::shared_lock lock(mutex_);

        if (auto it = reverseDeps.find(ext.GetId()); it != reverseDeps.end()) {
            for (const auto& depId : it->second) {
                if (failedExtensions_.contains(depId)) {
                    // Find the name of the failed dependency
                    auto extIt = std::find_if(allExtensions.begin(), allExtensions.end(),
                        [&](const Extension& e) { return e.GetId() == depId; });
                    if (extIt != allExtensions.end()) {
                        return extIt->GetName();
                    }
                    return "unknown";
                }
            }
        }
        return "";
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
        std::vector<std::pair<std::string, StageStatistics>> stages;
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

        std::vector<StageEntry> _stages;
        std::shared_ptr<IProgressReporter> _reporter;
        //std::shared_ptr<MetricsCollector> metrics_;
        size_t threadPoolSize_ = std::thread::hardware_concurrency();
        std::shared_ptr<Config> config_;

    public:
        template<typename StageType>
        Builder& AddStage(std::unique_ptr<StageType> stage, bool required = true) {
            static_assert(std::is_base_of_v<IStage<T>, StageType>);
            _stages.push_back({std::move(stage), required});
            return *this;
        }

        Builder& WithReporter(std::shared_ptr<IProgressReporter> reporter) {
            _reporter = std::move(reporter);
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
        : _threadPool(builder.threadPoolSize_)
        , _reporter(std::move(builder._reporter))
        //, metrics_(std::move(builder.metrics_))
        , config_(std::move(builder.config_)) {

        for (auto& [stage, required] : builder._stages) {
            _stages.push_back({std::move(stage), required});
        }
    }

public:
    // Execute pipeline - container is modified in place
    std::pair<std::vector<T>, PipelineReport> Execute(std::vector<T> items) {
        PipelineReport report;
        report.stages.reserve(_stages.size());
        report.initialItems = items.size();

        auto pipelineStart = Clock::now();

        // Create execution context
        ExecutionContext<T> ctx{
            .threadPool = _threadPool,
            .reporter = _reporter
        };

        // Execute each stage
        for (const auto& [stage, required] : _stages) {
            auto title = std::format("{} [()]", stage->GetName(), plg::enum_to_string(stage->GetType()));
            auto stats = ExecuteStage(stage.get(), items, ctx);
            bool failed = stats.failed > 0 && required;
            report.stages.emplace_back(std::move(title), std::move(stats));

            // Check if we should continue
            if (failed)
                break;
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

        if (_reporter) {
            _reporter->OnStageStart(stage->GetName(), items.size());
        }

        // Setup
        stage->Setup(items, ctx);

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

        // Teardown
        stage->Teardown(items, ctx);

        stats.itemsOut = items.size();
        stats.elapsed = std::chrono::duration_cast<Duration>(Clock::now() - startTime);

        if (_reporter) {
            _reporter->OnStageComplete(stage->GetName(), stats.succeeded, stats.failed);
        }

        return stats;
    }

    void ExecuteTransform(
        ITransformStage<T>* stage,
        std::vector<T>& items,
        StageStatistics& stats,
        const ExecutionContext<T>& ctx) {

        // Process items in parallel
        std::atomic<size_t> succeeded{0};
        std::atomic<size_t> failed{0};
        std::mutex errorMutex;

        _threadPool.submit_sequence(0, items.size(), [&](size_t i) {
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
            stats.errors.push_back(std::move(result.error()));

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
                    _threadPool.submit_sequence(0, batch.size(), [&](size_t i) {
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
    }

    void ReportItemComplete(std::string_view stage, const T& item, bool success) {
        if (_reporter) {
            _reporter->OnItemComplete(stage, GetItemName(item), success);
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

    std::vector<StageEntry> _stages;
    plg::thread_pool<> _threadPool;
    std::shared_ptr<IProgressReporter> _reporter;
    //std::shared_ptr<MetricsCollector> _metrics;
};

// ============================================================================
// Concrete Stage Implementations
// ============================================================================

// Parsing Stage - Transform type
class ParsingStage : public ITransformStage<Extension> {
    std::shared_ptr<IManifestParser> _parser;
    std::shared_ptr<IFileSystem> _fileSystem;

public:
    ParsingStage(std::shared_ptr<IManifestParser> parser,
                 std::shared_ptr<IFileSystem> fileSystem)
        : _parser(std::move(parser)), _fileSystem(std::move(fileSystem)) {}

    std::string_view GetName() const override { return "Parsing"; }

    bool ShouldProcess(const Extension& item) const override {
        return item.GetState() == ExtensionState::Discovered;
    }

    Result<void> ProcessItem(
        Extension& ext,
        [[maybe_unused]] const ExecutionContext<Extension>& ctx) override {

        ext.StartOperation(ExtensionState::Parsing);

        auto content = _fileSystem->ReadTextFile(ext.GetLocation());
        if (!content) {
            ext.AddError(content.error());
            ext.EndOperation(ExtensionState::Corrupted);
            return plg::unexpected(std::move(content.error()));
        }

        auto manifest = _parser->Parse(*content, ext.GetType());
        if (!manifest) {
            ext.AddError(manifest.error());
            ext.EndOperation(ExtensionState::Corrupted);
            return plg::unexpected(std::move(manifest.error()));
        }

        ext.SetManifest(std::move(*manifest));
        ext.EndOperation(ExtensionState::Parsed);

        return {};
    }
};

// Resolution Stage - Barrier type (reorders and filters)
class ResolutionStage : public IBarrierStage<Extension> {
    std::shared_ptr<IDependencyResolver> _resolver;
    plg::flat_map<UniqueId, std::vector<UniqueId>>* _dependencyGraph;
    plg::flat_map<UniqueId, std::vector<UniqueId>>* _reverseDependencyGraph;
    std::optional<std::unordered_set<std::string>> _whitelist;
    std::optional<std::unordered_set<std::string>> _blacklist;

public:
    ResolutionStage(std::shared_ptr<IDependencyResolver> resolver,
                    plg::flat_map<UniqueId, std::vector<UniqueId>>* depGraph,
                    plg::flat_map<UniqueId, std::vector<UniqueId>>* reverseDepGraph)
        : _resolver(std::move(resolver))
        , _dependencyGraph(depGraph)
        , _reverseDependencyGraph(reverseDepGraph) {}

    std::string_view GetName() const override { return "Resolution"; }

    void SetFilters(std::optional<std::unordered_set<std::string>> white,
                    std::optional<std::unordered_set<std::string>> black) {
        _whitelist = std::move(white);
        _blacklist = std::move(black);
    }

    Result<std::vector<Extension>> ProcessAll(
        std::vector<Extension> items,
        [[maybe_unused]] const ExecutionContext<Extension>& ctx
    ) override {
        // Step 1: Filter extensions based on whitelist/blacklist
        std::vector<Extension> filtered;
        std::vector<Extension> excluded;

        filtered.reserve(items.size());
        //excluded.reserve(items.size());

        for (auto&& ext : items) {
            bool include = true;

            // Check parsed state
            if (ext.GetState() != ExtensionState::Parsed) {
                include = false;
            }

            // Check whitelist
            if (include && _whitelist &&
                !_whitelist->contains(ext.GetName())) {
                include = false;
            }

            // Check blacklist
            if (include && _blacklist &&
                _blacklist->contains(ext.GetName())) {
                include = false;
            }

            if (include) {
                ext.SetState(ExtensionState::Resolving);
                filtered.push_back(std::move(ext));
            } else {
                ext.SetState(ExtensionState::Disabled);
                ext.AddWarning("Excluded due to policy");
                excluded.push_back(std::move(ext));
            }
        }

        if (filtered.empty()) {
            return plg::unexpected("No valid extensions to resolve");
        }

        // Step 2: Build language registry and add language dependencies
        {
            plg::flat_map<std::string, UniqueId> languages;

            for (const auto& ext : filtered) {
                if (ext.GetType() == ExtensionType::Module) {
                    languages[ext.GetLanguage()] = ext.GetId();
                }
            }

            for (auto& ext : filtered) {
                if (ext.GetType() == ExtensionType::Plugin) {
                    if (auto it = languages.find(ext.GetLanguage()); it != languages.end()) {
                        ext.AddDependency(GetExtensionName(filtered, it->second));
                    }
                }
            }
        }

        // Step 3: Resolve dependencies
        auto report = _resolver->Resolve(filtered);

        // Store dependency graphs
        *_dependencyGraph = std::move(report.dependencyGraph);
        *_reverseDependencyGraph = std::move(report.reverseDependencyGraph);

        // Step 4: Create ID to extension mapping for efficient lookup
        std::unordered_map<UniqueId, Extension> idToExtension;
        idToExtension.reserve(idToExtension.size());

        for (auto&& ext : filtered) {
            idToExtension.emplace(ext.GetId(), std::move(ext));
        }

        // Step 5: Build result in dependency order
        std::vector<Extension> result;
        result.reserve(report.loadOrder.size() + excluded.size());

        // Add resolved extensions in load order
        for (auto id : report.loadOrder) {
            if (auto it = idToExtension.find(id); it != idToExtension.end()) {
                auto& ext = it->second;
                ext.SetState(ExtensionState::Resolved);
                result.push_back(std::move(ext));
                idToExtension.erase(it);
            }
        }

        // Add unresolved extensions at the end
        for (auto& [id, ext] : idToExtension) {
            ext.SetState(ExtensionState::Unresolved);

            // Add resolution errors if any
            if (auto it = report.issues.find(id); it != report.issues.end()) {
                for (const auto& issue : it->second) {
                    if (issue.isBlocking) {
                        ext.AddError(issue.GetDetailedDescription());
                    } else {
                        ext.AddWarning(issue.GetDetailedDescription());
                    }
                }
            }

            result.push_back(std::move(ext));
        }

        // Add excluded extensions at the very end
        result.insert(result.end(),
                     std::make_move_iterator(excluded.begin()),
                     std::make_move_iterator(excluded.end()));

        return result;
    }

private:
    std::string GetExtensionName(std::span<const Extension> extensions, UniqueId id) const {
        auto it = std::find_if(extensions.begin(), extensions.end(), [&](const Extension& p) {
            return p.GetId() == id;
        });
        return it != extensions.end() ? it->GetName() : "";
    }
};

// Example: Initialization Stage - Batch type for resource-limited operations
/*class InitializationStage : public IBatchStage<Extension> {
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

    void SetupBatch(std::span<Extension> batch, size_t batchIndex,
                    const ExecutionContext<Extension>& ctx) override {
        PL_LOG_DEBUG("Initializing batch {} with {} extensions", batchIndex, batch.size());
        activeInitializations_ = 0;
    }

    Result<void> ProcessBatch(
        std::span<Extension> batch,
        size_t batchIndex,
        size_t totalBatches,
        const ExecutionContext<Extension>& ctx) override {

        // Could perform batch-level resource allocation here
        // For example, pre-allocate shared resources for the batch

        return {};
    }

    void TeardownBatch(std::span<Extension> batch, size_t batchIndex,
                      const ExecutionContext<Extension>& ctx) override {
        // Wait for all initializations in batch to complete
        while (activeInitializations_.load() > 0) {
            std::this_thread::yield();
        }
    }
};*/
#if 0
// Loading Stage - Sequential type
class LoadingStage : public ISequentialStage<Extension> {
    Loader& _loader;
    plg::flat_map<std::string, Extension*> _loadedModules;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>* _dependencyGraph;

public:
    LoadingStage(Loader& loader,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>* dependencyGraph)
        : _loader(loader), _dependencyGraph(dependencyGraph) {}

    std::string_view GetName() const override { return "Loading"; }

    bool ContinueOnError() const override { return true; }

    bool ShouldProcess(const Extension& item) const override {
        return item.GetState() == ExtensionState::Resolved;
    }

    Result<void> ProcessItem(
        Extension& ext,
        size_t position,
        size_t total,
        const ExecutionContext<Extension>& ctx) override {

        ext.StartOperation(ExtensionState::Loading);

        switch (ext.GetType()) {
            case ExtensionType::Module: {
                auto result = _loader.LoadModule(ext);
                if (!result) {
                    ext.AddError(result.error());
                    ext.EndOperation(ExtensionState::Failed);
                    // TODO: propagate deps
                    return result;
                }

                _loadedModules[ext.GetLanguage()] = &ext;
                break;
            }

            case ExtensionType::Plugin: {
                auto it = _loadedModules.find(ext.GetLanguage());
                if (it == _loadedModules.end()) {
                    return plg::unexpected(std::format("Language module '{}' not found", ext.GetLanguage()));
                }

                auto result = _loader.LoadPlugin(*it->second, ext);
                if (!result) {
                    ext.AddError(result.error());
                    ext.EndOperation(ExtensionState::Failed);
                    // TODO: propagate deps
                    return result;
                }
                break;
            }

            default:
                throw std::runtime_error("Unknown extension type");
        }

        ext.EndOperation(ExtensionState::Loaded);

        // Log progress if verbose
        //std::println("[{}/{}] Loaded {}", position + 1, total, ext.GetName());
        return {};
    }


};

// Starting Stage - Sequential type
class StartingStage : public ISequentialStage<Extension> {
    Loader& _loader;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>* _dependencyGraph;

public:
    StartingStage(Loader& loader,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>* dependencyGraph)
        : _loader(loader), _dependencyGraph(dependencyGraph) {}

    std::string_view GetName() const override { return "Starting"; }

    bool ContinueOnError() const override { return true; }

    bool ShouldProcess(const Extension& item) const override {
        return item.GetState() == ExtensionState::Loaded;
    }

    Result<void> ProcessItem(
        Extension& ext,
        size_t position,
        size_t total,
        const ExecutionContext<Extension>& ctx) override {

        ext.StartOperation(ExtensionState::Loading);

        switch (ext.GetType()) {
            case ExtensionType::Module: {
                break;
            }

            case ExtensionType::Plugin: {
                auto result = _loader.StartPlugin(ext);
                if (!result) {
                    ext.AddError(result.error());
                    ext.EndOperation(ExtensionState::Failed);
                    // TODO: propagate deps
                    return result;
                }
                break;
            }

            default:
                throw std::runtime_error("Unknown extension type");
        }

        ext.EndOperation(ExtensionState::Loaded);

        // Log progress if verbose
        //std::println("[{}/{}] Loaded {}", position + 1, total, ext.GetName());
        return {};
    }
};
#endif

class LoadingStage : public ISequentialStage<Extension> {
    Loader& _loader;
    FailureTracker& _failureTracker;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>* _dependencyGraph;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>* _reverseDependencyGraph;
    plg::flat_map<std::string, Extension*> _loadedModules;
    std::span<const Extension> _allExtensions;  // Reference to all extensions for lookup

public:
    LoadingStage(Loader& loader,
                FailureTracker& failureTracker,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>* dependencyGraph,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>* reverseDependencyGraph)
        : _loader(loader)
        , _failureTracker(failureTracker)
        , _dependencyGraph(dependencyGraph)
        , _reverseDependencyGraph(reverseDependencyGraph) {}

    std::string_view GetName() const override { return "Loading"; }

    bool ContinueOnError() const override { return true; }  // Continue to mark dependents as skipped

    bool ShouldProcess(const Extension& item) const override {
        return item.GetState() == ExtensionState::Resolved;
    }

    void Setup(std::span<Extension> items, const ExecutionContext<Extension>& ctx) override {
        // Store reference to all extensions for name lookup
        _allExtensions = items;
    }

    Result<void> ProcessItem(
        Extension& ext,
        size_t position,
        size_t total,
        const ExecutionContext<Extension>& ctx) override {

        // First check if any dependencies have failed
        if (_failureTracker.HasAnyDependencyFailed(ext, *_reverseDependencyGraph)) {
            std::string_view failedDep = _failureTracker.GetFailedDependencyName(
                ext, *_reverseDependencyGraph, _allExtensions);

            ext.SetState(ExtensionState::Skipped);
            ext.AddError(std::format("Skipped: dependency '{}' failed to load", failedDep));

            // Important: Mark this as failed too so its dependents are also skipped
            _failureTracker.MarkFailed(ext.GetId());

            return plg::unexpected(std::format("Dependency '{}' failed", failedDep));
        }

        // Proceed with normal loading
        ext.StartOperation(ExtensionState::Loading);

        Result<void> result;
        switch (ext.GetType()) {
            case ExtensionType::Module: {
                result = _loader.LoadModule(ext);
                if (result) {
                    _loadedModules[ext.GetLanguage()] = &ext;
                }
                break;
            }

            case ExtensionType::Plugin: {
                auto it = _loadedModules.find(ext.GetLanguage());
                if (it == _loadedModules.end()) {
                    result = plg::unexpected(std::format("Language module '{}' not found", ext.GetLanguage()));
                } else {
                    result = _loader.LoadPlugin(*it->second, ext);
                }
                break;
            }

            default:
                result = plg::unexpected("Unknown extension type");
        }

        if (!result) {
            ext.AddError(result.error());
            ext.EndOperation(ExtensionState::Failed);

            // Mark as failed and propagate to dependents
            _failureTracker.MarkFailed(ext.GetId());
            PropagateFailureToDirectDependents(ext);

            return result;
        }

        ext.EndOperation(ExtensionState::Loaded);
        return {};
    }

private:
    void PropagateFailureToDirectDependents(const Extension& failedExt) {
        // Mark all direct dependents as will-be-skipped
        // They'll be properly handled when their turn comes
        if (auto it = _dependencyGraph->find(failedExt.GetId());
            it != _dependencyGraph->end()) {
            for (const auto& dependentId : it->second) {
                _failureTracker.MarkFailed(dependentId);
            }
        }
    }
};

// ============================================================================
// Starting Stage with Failure Propagation
// ============================================================================

class StartingStage : public ISequentialStage<Extension> {
    Loader& _loader;
    FailureTracker& _failureTracker;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>* _dependencyGraph;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>* _reverseDependencyGraph;
    std::span<const Extension> _allExtensions;

public:
    StartingStage(Loader& loader,
                FailureTracker& failureTracker,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>* dependencyGraph,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>* reverseDependencyGraph)
        : _loader(loader)
        , _failureTracker(failureTracker)
        , _dependencyGraph(dependencyGraph)
        , _reverseDependencyGraph(reverseDependencyGraph) {}

    std::string_view GetName() const override { return "Starting"; }

    bool ContinueOnError() const override { return true; }

    bool ShouldProcess(const Extension& item) const override {
        return item.GetState() == ExtensionState::Loaded;
    }

    void Setup(std::span<Extension> items, const ExecutionContext<Extension>& ctx) override {
        _allExtensions = items;
    }

    Result<void> ProcessItem(
        Extension& ext,
        size_t position,
        size_t total,
        const ExecutionContext<Extension>& ctx) override {

        // Check if any dependencies have failed (during loading or starting)
        if (_failureTracker.HasAnyDependencyFailed(ext, *_reverseDependencyGraph)) {
            std::string_view failedDep = _failureTracker.GetFailedDependencyName(
                ext, *_reverseDependencyGraph, _allExtensions);

            ext.SetState(ExtensionState::Skipped);
            ext.AddError(std::format("Skipped: dependency '{}' failed to start", failedDep));

            _failureTracker.MarkFailed(ext.GetId());

            return plg::unexpected(std::format("Dependency '{}' failed", failedDep));
        }

        ext.StartOperation(ExtensionState::Starting);

        Result<void> result;
        switch (ext.GetType()) {
            case ExtensionType::Module: {
                // Modules typically don't need starting
                result = {};
                break;
            }

            case ExtensionType::Plugin: {
                result = _loader.StartPlugin(ext);
                break;
            }

            default:
                result = plg::unexpected("Unknown extension type");
        }

        if (!result) {
            ext.AddError(result.error());
            ext.EndOperation(ExtensionState::Failed);

            // Mark as failed and propagate
            _failureTracker.MarkFailed(ext.GetId());
            PropagateFailureToDirectDependents(ext);

            return result;
        }

        ext.EndOperation(ExtensionState::Running);
        return {};
    }

private:
    void PropagateFailureToDirectDependents(const Extension& failedExt) {
        if (auto it = _dependencyGraph->find(failedExt.GetId());
            it != _dependencyGraph->end()) {
            for (const auto& dependentId : it->second) {
                _failureTracker.MarkFailed(dependentId);
            }
        }
    }
};

struct Manager::Impl {
    Impl(std::shared_ptr<Context> context) {
        // Create services
        assemblyLoader = context->GetService<IAssemblyLoader>();
        fileSystem = context->GetService<IFileSystem>();
        manifestParser = context->GetService<IManifestParser>();
        //validator = context->GetService<ExtensionValidator>();
        resolver = context->GetService<IDependencyResolver>();  // Your libsolv wrapper
        //loader = context->GetService<DynamicExtensionLoader>();
        progressReporter = context->GetService<IProgressReporter>();
        //metricsCollector = context->Get<MetricsCollector>();

    }
    std::shared_ptr<Config> config;

    // Services
    std::shared_ptr<IAssemblyLoader> assemblyLoader;
    std::shared_ptr<IFileSystem> fileSystem;
    std::shared_ptr<IManifestParser> manifestParser;
    std::shared_ptr<IDependencyResolver> resolver;
    //std::shared_ptr<IExtensionValidator> validator;
    //std::shared_ptr<IExtensionRegistry> registry;
    std::shared_ptr<IProgressReporter> progressReporter;
    //std::shared_ptr<MetricsCollector> metricsCollector;
    std::make_shared<Provider>(std::move(context), std::move(manager))
    std::shared_ptr<Provider> provider;

    // Dependency graphs (filled by resolution stage)
    plg::flat_map<UniqueId, std::vector<UniqueId>> dependencyGraph;
    plg::flat_map<UniqueId, std::vector<UniqueId>> reverseDependencyGraph;

public:
    void Initialize() {
        Loader loader();

        // Step 1: Quick discovery
        std::vector<Extension> extensions = DiscoverExtensions();

        // Step 2: Create resolution stage with filters
        auto resolutionStage = std::make_unique<ResolutionStage>(
            resolver, &dependencyGraph, &reverseDependencyGraph
        );
        resolutionStage->SetFilters(config->whitelistedExtensions, config->blacklistedExtensions);

        // Step 3: Build pipeline
        auto pipeline = FlexiblePipeline<Extension>::Create()
            // Parse manifests in parallel (Transform stage)
            .AddStage(std::make_unique<ParsingStage>(manifestParser, fileSystem))

            // Validate extensions in batches (Batch stage)
            //.AddStage(std::make_unique<ValidationStage>(validator, 20), false)

            // Fetch metadata in controlled batches (Batch stage with rate limiting)
            //.AddStage(std::make_unique<NetworkFetchStage>(registry), false)

            // Resolve dependencies and reorder container (Barrier stage)
            .AddStage(std::move(resolutionStage))

            // Initialize extensions in batches (Batch stage for resource control)
            //.AddStage(std::make_unique<InitializationStage>(assemblyLoader), false)

            // Load extensions in dependency order (Sequential stage)
            .AddStage(std::make_unique<LoadingStage>(loader, &dependencyGraph))

            // Optional: Remove failed extensions to save processing
            //.AddStage(std::make_unique<FilterFailedStage>(), false)

            // Start extensions in order (Sequential stage)
            .AddStage(std::make_unique<StartingStage>())

            // Start extensions in order (Sequential stage)
            //.AddStage(std::make_unique<ExportStage>())

            .WithReporter(progressReporter)
            //.WithMetrics(metricsCollector)
            .WithConfig(config)
            .WithThreadPoolSize(std::thread::hardware_concurrency())
            .Build();

        // Step 4: Execute pipeline
        auto [finalExtensions, report] = pipeline->Execute(std::move(extensions));

        // Step 5: Store results
        extensions_ = std::move(finalExtensions);

        // Convert report to state
        //ConvertReportToState(report, state);

        // Print reports if configured
        if (config->printReport) {
            report.Print();
        }

        return state;
    }

    void Update(Duration deltaTime) {
        loader->Update(_extensions, deltaTime);
    }

    void Terminate() {
        for (const auto& ext : _extensions) {
            loader->Update(_extensions, deltaTime);
        }
    }

private:
    std::vector<Extension> _extensions;

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
    std::vector<Extension> DiscoverExtensions() {
        std::vector<Extension> extensions;

        // Collect all paths
        auto paths = CollectManifestPaths();

        // Create extension entries
        extensions.reserve(paths.size());
        for (auto&& path : paths) {
            extensions.emplace_back(id++, std::move(path));
        }

        return extensions;
    }
};

Manager::Manager(std::shared_ptr<Context> context)
    : _impl(std::make_unique<Impl>(std::move(context))) {}

Manager::~Manager() = default;

void Manager::Initialize() {
    return _impl->Initialize();
}

void Manager::Update(Duration deltaTime) {
    return _impl->Update(deltaTime);
}

void Manager::Terminate() {
    return _impl->Terminate();
}

// ============================================================================
// Dependency Injection
// ============================================================================

// Set progress reporter
/*void Manager::setProgressReporter(std::shared_ptr<IProgressReporter> reporter) {
    progressReporter = reporter;
}

void Manager::setExtensionDiscovery(std::unique_ptr<IExtensionDiscovery> discovery) {
    _impl->extensionDiscovery = std::move(discovery);
}

void Manager::setExtensionValidator(std::unique_ptr<IExtensionValidator> validator) {
    _impl->extensionValidator = std::move(validator);
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
