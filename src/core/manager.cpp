#pragma once

#include <any>

#include "plugify/asm/assembly_loader.hpp"
#include "plugify/core/config.hpp"
#include "plugify/core/dependency_resolver.hpp"
#include "plugify/core/extension.hpp"
#include "plugify/core/manifest.hpp"
#include "plugify/core/manifest_parser.hpp"
#include "plugify/core/plugify.hpp"
#include "plugify/core/progress_reporter.hpp"

//#include "asm/defer.hpp"
#include "core/extension_loader.hpp"
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
    std::unordered_set<UniqueId> _failedExtensions;
    mutable std::shared_mutex _mutex;

public:
    FailureTracker(size_t capacity) {
        _failedExtensions.reserve(capacity);
    }

    void MarkFailed(UniqueId id) {
        std::unique_lock lock(_mutex);
        _failedExtensions.insert(id);
    }

    bool HasFailed(UniqueId id) const {
        std::shared_lock lock(_mutex);
        return _failedExtensions.contains(id);
    }

    bool HasAnyDependencyFailed(
        const Extension& ext,
        const plg::flat_map<UniqueId, std::vector<UniqueId>>& reverseDeps) const {

        std::shared_lock lock(_mutex);

        // Check if any of this extension's dependencies have failed
        if (auto it = reverseDeps.find(ext.GetId()); it != reverseDeps.end()) {
            for (const auto& depId : it->second) {
                if (_failedExtensions.contains(depId)) {
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

        std::shared_lock lock(_mutex);

        if (auto it = reverseDeps.find(ext.GetId()); it != reverseDeps.end()) {
            for (const auto& depId : it->second) {
                if (_failedExtensions.contains(depId)) {
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

        std::string ToString() const {
            std::string buffer;
            buffer.reserve(256);
            
            auto it = std::back_inserter(buffer);
            std::format_to(it, "=== Pipeline Report ===\n");
            std::format_to(it, "Items: {} -> {} ({} total)\n",
                       initialItems, finalItems, totalTime);

            for (size_t i = 0; i < stages.size(); ++i) {
                const auto& [n, s] = stages[i];
                std::format_to(it, "  Stage {} ({}): {} -> {} items, {} succeeded, {} failed ({})\n",
                           i, n, s.itemsIn, s.itemsOut, s.succeeded, s.failed, s.elapsed);

                if (!s.errors.empty()) {
                    for (const auto& err : s.errors) {
                        std::format_to(it, "    ERROR: {}\n", err);
                    }
                }
            }

            return buffer;
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
        size_t _threadPoolSize = std::thread::hardware_concurrency();
        const Config* _config;

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
            _threadPoolSize = size;
            return *this;
        }

        Builder& WithConfig(const Config& config) {
            _config = &config;
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
        , _config(std::move(builder.config_)) {

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
                ++stats.succeeded;
                ReportItemComplete(stage->GetName(), item, true);
            } else {
                ++stats.failed;
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
    const Config& _config;
};

// ============================================================================
// Concrete Stage Implementations
// ============================================================================

static std::string_view GetExtensionName(std::span<const Extension> allExtensions, UniqueId depId) {
    auto it = std::find_if(allExtensions.begin(), allExtensions.end(),
        [&](const Extension& e) { return e.GetId() == depId; });
    if (it != allExtensions.end()) {
        return it->GetName();
    }
    return it != allExtensions.end() ? it->GetName() : "<unknown>";
}

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
    std::vector<UniqueId>* _loadOrder;
    plg::flat_map<UniqueId, std::vector<UniqueId>>* _depGraph;
    plg::flat_map<UniqueId, std::vector<UniqueId>>* _reverseDepGraph;

public:
    ResolutionStage(std::shared_ptr<IDependencyResolver> resolver,
                    std::vector<UniqueId>* loadOrder,
                    plg::flat_map<UniqueId, std::vector<UniqueId>>* depGraph,
                    plg::flat_map<UniqueId, std::vector<UniqueId>>* reverseDepGraph)
        : _resolver(std::move(resolver))
        , _loadOrder(loadOrder)
        , _depGraph(depGraph)
        , _reverseDepGraph(reverseDepGraph) {}

    std::string_view GetName() const override { return "Resolution"; }

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
            if (include && !ctx.config.security.whitelistedExtensions.
                contains(ext.GetName())) {
                include = false;
            }

            // Check blacklist
            if (include && ctx.config.security.blacklistedExtensions.
                contains(ext.GetName())) {
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
                        ext.AddDependency(std::string(GetExtensionName(filtered, it->second)));
                    }
                }
            }
        }

        // Step 3: Resolve dependencies
        auto report = _resolver->Resolve(filtered);

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

        // Store dependency graphs
        *_depGraph = std::move(report.dependencyGraph);
        *_reverseDepGraph = std::move(report.reverseDependencyGraph);
        *_loadOrder = std::move(report.loadOrder);

        return result;
    }
};

// ============================================================================
// Loading Stage with Failure Propagation
// ============================================================================

class LoadingStage : public ISequentialStage<Extension> {
    ExtensionLoader& _loader;
    FailureTracker& _failureTracker;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>& _depGraph;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>& _reverseDepGraph;
    plg::flat_map<std::string, Extension*> _loadedModules;
    std::span<const Extension> _allExtensions;  // Reference to all extensions for lookup

public:
    LoadingStage(ExtensionLoader& loader,
                FailureTracker& failureTracker,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>& depGraph,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>& reverseDepGraph)
        : _loader(loader)
        , _failureTracker(failureTracker)
        , _depGraph(depGraph)
        , _reverseDepGraph(reverseDepGraph) {}

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
        if (_failureTracker.HasAnyDependencyFailed(ext, _reverseDepGraph)) {
            std::string_view failedDep = _failureTracker.GetFailedDependencyName(
                ext, _reverseDepGraph, _allExtensions);

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

        if (auto loadTime = ext.GetOperationTime(ExtensionState::Loading);
            loadTime > ctx.config.loading.loadTimeout) {
            ext.AddWarning(std::format("Took {} to start", loadTime));
        }

        return {};
    }

private:
    void PropagateFailureToDirectDependents(const Extension& failedExt) {
        // Mark all direct dependents as will-be-skipped
        // They'll be properly handled when their turn comes
        if (auto it = _depGraph.find(failedExt.GetId());
            it != _depGraph.end()) {
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
    ExtensionLoader& _loader;
    FailureTracker& _failureTracker;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>& _depGraph;
    const plg::flat_map<UniqueId, std::vector<UniqueId>>& _reverseDepGraph;
    std::span<const Extension> _allExtensions;

public:
    StartingStage(ExtensionLoader& loader,
                FailureTracker& failureTracker,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>& depGraph,
                const plg::flat_map<UniqueId, std::vector<UniqueId>>& reverseDepGraph)
        : _loader(loader)
        , _failureTracker(failureTracker)
        , _depGraph(depGraph)
        , _reverseDepGraph(reverseDepGraph) {}

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
        if (_failureTracker.HasAnyDependencyFailed(ext, _reverseDepGraph)) {
            std::string_view failedDep = _failureTracker.GetFailedDependencyName(
                ext, _reverseDepGraph, _allExtensions);

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

        ext.EndOperation(ExtensionState::Started);

        if (auto startTime = ext.GetOperationTime(ExtensionState::Loading);
            startTime > ctx.config.loading.startTimeout) {
            ext.AddWarning(std::format("Took {} to start", startTime));
        }

        return {};
    }

private:
    void PropagateFailureToDirectDependents(const Extension& failedExt) const {
        if (auto it = _depGraph.find(failedExt.GetId());
            it != _depGraph.end()) {
            for (const auto& dependentId : it->second) {
                _failureTracker.MarkFailed(dependentId);
            }
        }
    }
};

struct Manager::Impl {
    Impl(const ServiceLocator& services, const Config& cfg, const Manager& manager)
        : config{cfg}
        , provider(services, config, manager)
        , loader(services, config, provider)
    {
        // Create services
        assemblyLoader = services.Get<IAssemblyLoader>();
        fileSystem = services.Get<IFileSystem>();
        logger = services.Get<ILogger>();
        manifestParser = services.Get<IManifestParser>();
        resolver = services.Get<IDependencyResolver>();
        progressReporter = services.Get<IProgressReporter>();
    }
    ~Impl() {
        if (initialized) {
            Terminate();
        }
    }

    const Config& config;
    Provider provider;
    ExtensionLoader loader;
    std::vector<Extension> extensions;
    bool initialized{false};

    // Services
    std::shared_ptr<IAssemblyLoader> assemblyLoader;
    std::shared_ptr<IFileSystem> fileSystem;
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<IManifestParser> manifestParser;
    std::shared_ptr<IDependencyResolver> resolver;
    std::shared_ptr<IProgressReporter> progressReporter;

    // Dependency graphs (filled by resolution stage)
    std::vector<UniqueId> loadOrder;
    plg::flat_map<UniqueId, std::vector<UniqueId>> depGraph;
    plg::flat_map<UniqueId, std::vector<UniqueId>> reverseDepGraph;

public:
    void Initialize() {
        if (initialized) {
            logger->Log("Manager already initialized", Severity::Error);
            return;
        }

        std::vector<Extension> initialExtensions = DiscoverExtensions();

        FailureTracker failureTracker(initialExtensions.size());
        
        auto pipeline = FlexiblePipeline<Extension>::Create()
            .AddStage(std::make_unique<ParsingStage>(manifestParser, fileSystem))
            .AddStage(std::make_unique<ResolutionStage>(resolver, &loadOrder, &depGraph, &reverseDepGraph))
            .AddStage(std::make_unique<LoadingStage>(loader, failureTracker, depGraph, reverseDepGraph))
            //.AddStage(std::make_unique<ExportStage>())
            .AddStage(std::make_unique<StartingStage>(loader, failureTracker, depGraph, reverseDepGraph))
            .WithConfig(config)
            .WithReporter(progressReporter)
            .WithThreadPoolSize(config.loading.maxConcurrentLoads)
            .Build();

        auto [finalExtensions, report] = pipeline->Execute(std::move(initialExtensions));

        extensions = std::move(finalExtensions);

        // Print reports if configured
        if (config.logging.printReport || report.finalItems == 0) {
            logger->Log(report.ToString(), Severity::Info);
            logger->Log(loader.GetStatistics().ToString(), Severity::Info);
            logger->Log("=== Extensions Report ===", Severity::Info);
            for (auto& ext : extensions) {
                logger->Log(ext.GetPerformanceReport(), Severity::Info);
            }
        }
        if (config.logging.printLoadOrder) {
            logger->Log(DumpLoadOrder(), Severity::Info);
        }
        /*if (config.logging.printLoadStatistics) {
        }*/
        if (config.logging.printDependencyGraph) {
            logger->Log(DumpDependencyGraph(), Severity::Info);
        }
        const auto& exportPath = config.logging.exportDigraphDot;
        if (!exportPath.empty()) {
            auto graph = DumpDependencyGraphDOT();
            if (auto writeResult = fileSystem->WriteTextFile(exportPath, graph); !writeResult) {
                logger->Log(std::format("Export DOT: {}", writeResult.error()), Severity::Error);
            } else {
                logger->Log(std::format("Export DOT: {}", exportPath.string()), Severity::Info);
            }
        }

        initialized = true;
    }

    void Update(Duration deltaTime) {
        if (!initialized) {
            return;
        }

        for (const auto& ext : extensions) {
            if (ext.GetState() == ExtensionState::Started) {
                Result<void> result;
                switch (ext.GetType()) {
                    case ExtensionType::Module: {
                        result = loader.UpdateModule(ext, deltaTime);
                        break;
                    }

                    case ExtensionType::Plugin: {
                        result = loader.UpdatePlugin(ext, deltaTime);
                        break;
                    }

                    default:
                        result = plg::unexpected("Unknown extension type");
                }
                if (!result) {
                    logger->Log(result.error(), Severity::Error);
                }
            }
        }
    }

    void Terminate() {
        if (!initialized) {
            return;
        }

        for (auto it = extensions.rbegin(); it != extensions.rend(); ++it) {
            auto& ext = *it;
            if (ext.GetState() == ExtensionState::Started) {
                ext.StartOperation(ExtensionState::Ending);
                Result<void> result;
                switch (ext.GetType()) {
                    case ExtensionType::Module: {
                        // Modules typically don't need ending
                        result = {};
                        break;
                    }

                    case ExtensionType::Plugin: {
                        result = loader.EndPlugin(ext);
                        break;
                    }

                    default:
                        result = plg::unexpected("Unknown extension type");
                }
                if (!result) {
                    logger->Log(result.error(), Severity::Error);
                }
                ext.EndOperation(ExtensionState::Ended);
            }
        }

        for (auto it = extensions.rbegin(); it != extensions.rend(); ++it) {
            auto& ext = *it;
            if (ext.GetState() == ExtensionState::Ended) {
                ext.StartOperation(ExtensionState::Terminating);
                Result<void> result;
                switch (ext.GetType()) {
                    case ExtensionType::Module: {
                        result = loader.UnloadModule(ext);
                        break;
                    }

                    case ExtensionType::Plugin: {
                        result = loader.UnloadPlugin(ext);
                        break;
                    }

                    default:
                        result = plg::unexpected("Unknown extension type");
                }
                if (!result) {
                    logger->Log(result.error(), Severity::Error);
                }
                ext.EndOperation(ExtensionState::Terminated);
            }
        }

        initialized = false;
    }

private:
    static constexpr std::array<std::string_view, 2> MANIFEST_EXTENSIONS = {
        "*.pplugin"sv, "*.pmodule"sv
    };

    std::vector<Extension> DiscoverExtensions() const {
        auto paths = fileSystem->FindFiles(
            config.paths.extensionsDir,
            MANIFEST_EXTENSIONS,
            true
        );

        if (!paths) {
            logger->Log(paths.error(), Severity::Error);
            return {};
        }

        UniqueId id = 0;

        std::vector<Extension> exts;
        exts.reserve(paths->size());
        for (auto&& path : *paths) {
            exts.emplace_back(id++, std::move(path));
        }
        return exts;
    }

    std::string DumpLoadOrder() const {
        std::string buffer;
        buffer.reserve(1024); // preallocate some space to reduce reallocs

        auto it = std::back_inserter(buffer);
        
        std::format_to(it, "=== Load Order ===\n");

        if (loadOrder.empty()) {
            std::format_to(it, "(empty)\n\n");
            return buffer;
        }

        for (size_t i = 0; i < loadOrder.size(); ++i) {
            const auto& id = loadOrder[i];
            std::format_to(it, "{:3}: {} (id={})\n", i, GetExtensionName(extensions, id), id);
        }

        buffer.push_back('\n');
        return buffer;
    }

    std::string DumpDependencyGraph() const {
        std::string buffer;
        buffer.reserve(2048);

        auto it = std::back_inserter(buffer);
        
        std::format_to(it, "=== Dependency Graph (pkg -> [deps]) ===\n");

        if (depGraph.empty()) {
            std::format_to(it, "(empty)\n\n");
            return buffer;
        }

        for (const auto& [id, deps] : depGraph) {
            std::format_to(it, "{} (id={}) -> ", GetExtensionName(extensions, id), id);
            if (deps.empty()) {
                std::format_to(it, "[]\n");
                continue;
            }

            buffer.push_back('[');
            bool first = true;
            for (const auto& d : deps) {
                if (!first) {
                    std::format_to(it, ", ");
                }
                std::format_to(it, "{} (id={})", GetExtensionName(extensions, d), d);
                first = false;
            }
            std::format_to(it, "]\n");
        }

        buffer.push_back('\n');
        return buffer;
    }

    std::string DumpDependencyGraphDOT() const {
        std::string buffer;
        buffer.reserve(4096);

        auto it = std::back_inserter(buffer);
        
        std::format_to(it, "digraph packages {{\n");
        std::format_to(it, "  rankdir=LR;\n");

        // nodes
        for (size_t i = 0; i < extensions.size(); ++i) {
            const auto& ext = extensions[i];
            std::format_to(it,
                           "  node{} [label=\"{}\\n(id={})\"];\n",
                           i, ext.GetName(), ext.GetId());
        }

        // edges
        for (const auto& [id, deps] : depGraph) {
            for (auto dep : deps) {
                std::format_to(it, "  node{} -> node{};\n", id, dep);
            }
        }

        std::format_to(it, "}}\n");
        return buffer;
    }
};

Manager::Manager(const ServiceLocator& services, const Config& config)
    : _impl(std::make_unique<Impl>(services, config, *this)) {}

Manager::~Manager() = default;

void Manager::Initialize() const {
    return _impl->Initialize();
}

bool Manager::IsInitialized() const {
    return _impl->initialized;
}

void Manager::Update(Duration deltaTime) const {
    return _impl->Update(deltaTime);
}

void Manager::Terminate() const {
    return _impl->Terminate();
}

// Query operations
bool Manager::IsExtensionLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
    auto it = std::find_if(_impl->extensions.begin(), _impl->extensions.end(),
        [&](const Extension& e) { return e.GetName() == name; });
    if (it != _impl->extensions.end()) {
        return constraint ? constraint->contains(it->GetVersion()) : true;
    }
    return false;
}
const Extension* Manager::FindExtension(std::string_view name) const noexcept {
    auto it = std::find_if(_impl->extensions.begin(), _impl->extensions.end(),
        [&](const Extension& e) { return e.GetName() == name; });
    if (it != _impl->extensions.end()) {
        return &(*it);
    }
    return nullptr;
}

std::span<const Extension> Manager::GetExtensions() const {
   return _impl->extensions;
}

std::vector<const Extension*> Manager::GetExtensionsByState(ExtensionState state) const {
    std::vector<const Extension*> result;
    result.reserve(_impl->extensions.size() / 2);
    for (auto& ext : _impl->extensions) {
        if (ext.GetState() == state) {
            result.push_back(&ext);
        }
    }
    return result;
}

std::vector<const Extension*> Manager::GetExtensionsByType(ExtensionType type) const {
    std::vector<const Extension*> result;
    result.reserve(_impl->extensions.size() / 2);
    for (auto& ext : _impl->extensions) {
        if (ext.GetType() == type) {
            result.push_back(&ext);
        }
    }
    return result;
}

bool Manager::operator==(const Manager& other) const noexcept = default;
auto Manager::operator<=>(const Manager& other) const noexcept = default;