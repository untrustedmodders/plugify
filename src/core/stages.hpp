#pragma once

#include "plugify/core/metric_collector.hpp"

#include "plg/thread_pool.hpp"

namespace plugify {
    // ============================================================================
    // Stage Interfaces
    // ============================================================================
    enum class StageType {
        Transform,    // Parallel processing, no order changes
        Barrier,      // Can reorder/filter the container
        Sequential,   // Processes in container order
        //Batch        // Parallel processing in controlled batches
    };

    // Execution context
    template<typename T>
    struct ExecutionContext {
        plg::thread_pool<>& threadPool;
        //std::shared_ptr<ILogger> logger;
        //std::shared_ptr<IProgressReporter> reporter;
        //std::shared_ptr<IMetricsCollector> metrics;
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
            const ExecutionContext<T>& ctx
        ) = 0;

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
            const ExecutionContext<T>& ctx
        ) = 0;
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
            const ExecutionContext<T>& ctx
        ) = 0;

        // Should we continue after an error?
        virtual bool ContinueOnError() const { return true; }

        // Optional: filter
        virtual bool ShouldProcess([[maybe_unused]] const T& item) const { return true; }
    };

#if 0
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
#endif
}