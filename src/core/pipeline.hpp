#pragma once

#include "core/stages.hpp"

namespace plugify {
    // ============================================================================
    // Pipeline Implementation
    // ============================================================================

    // Stage statistics
    struct StageStatistics {
        size_t itemsIn = 0;
        size_t itemsOut = 0;
        size_t succeeded = 0;
        size_t failed = 0;
        std::chrono::milliseconds elapsed{0};
        std::vector<std::pair<std::string, std::string>> errors;
    };

    template<typename T>
    class Pipeline {
    public:
        struct Report {
            std::vector<std::pair<std::string, StageStatistics>> stages;
            std::chrono::milliseconds totalTime{0};
            size_t initialItems = 0;
            size_t finalItems = 0;

            std::string ToString() const {
                std::string buffer;
                buffer.reserve(256);
                
                auto it = std::back_inserter(buffer);
                std::format_to(it, "\n=== Pipeline Report ===\n");
                std::format_to(it, "Items: {} -> {} ({} total)\n",
                           initialItems, finalItems, totalTime);

                for (size_t i = 0; i < stages.size(); ++i) {
                    const auto& [n, s] = stages[i];
                    std::format_to(it, "  Stage {} ({}): {} -> {} items, {} succeeded, {} failed ({})\n",
                               i, n, s.itemsIn, s.itemsOut, s.succeeded, s.failed, s.elapsed);

                    size_t maxWidth = 0;
                    for (auto const& [key, _] : s.errors) {
                        maxWidth = std::max(maxWidth, key.size());
                    }

                    for (auto const& [key, value] : s.errors) {
                        std::format_to(it, "    {:<{}} : {}\n", key, maxWidth, value);
                    }
                }

                return buffer;
            }
        };

        // Builder pattern for pipeline construction
        class Builder {
            friend class Pipeline;

            struct StageEntry {
                std::unique_ptr<IStage<T>> stage;
                bool required = true;
            };

            std::vector<StageEntry> _stages;
            //std::shared_ptr<ILogger> _logger;
            //std::shared_ptr<IProgressReporter> _reporter;
            //std::shared_ptr<IMetricsCollector> _metrics;
            size_t _threadPoolSize = std::thread::hardware_concurrency();

        public:
            template<typename StageType>
            Builder& AddStage(std::unique_ptr<StageType> stage, bool required = true) {
                static_assert(std::is_base_of_v<IStage<T>, StageType>);
                _stages.push_back({std::move(stage), required});
                return *this;
            }

            /*Builder& WithLogger(std::shared_ptr<ILogger> logger) {
                _logger = std::move(logger);
                return *this;
            }

            Builder& WithReporter(std::shared_ptr<IProgressReporter> reporter) {
                _reporter = std::move(reporter);
                return *this;
            }

            Builder& WithMetrics(std::shared_ptr<IMetricsCollector> metrics) {
                _metrics = std::move(metrics);
                return *this;
            }*/

            Builder& WithThreadPoolSize(size_t size) {
                _threadPoolSize = size;
                return *this;
            }

            std::unique_ptr<Pipeline> Build() {
                return std::unique_ptr<Pipeline>(new Pipeline(std::move(*this)));
            }
        };

        static Builder Create() { return Builder{}; }

    private:
        Pipeline(Builder&& builder)
            : _pool(builder._threadPoolSize) {
            for (auto& [stage, required] : builder._stages) {
                _stages.push_back({std::move(stage), required});
            }
        }

    public:
        // Execute pipeline - container is modified in place
        Report Execute(std::vector<T>& items) {
            Report report;
            report.stages.reserve(_stages.size());
            report.initialItems = items.size();

            auto pipelineStart = std::chrono::steady_clock::now();

            // Create execution context
            ExecutionContext<T> ctx{
                .pool = _pool
            };

            // Execute each stage
            for (const auto& [stage, required] : _stages) {
                auto title = std::format("{} [{}]", stage->GetName(), plg::enum_to_string(stage->GetType()));
                auto stats = ExecuteStage(stage.get(), items, ctx);

                bool failed = stats.failed > 0 && required;
                report.stages.emplace_back(std::move(title), std::move(stats));

                // Check if we should continue
                if (failed)
                    break;
            }

            report.finalItems = items.size();
            report.totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - pipelineStart
            );

            return report;
        }

    private:
        StageStatistics ExecuteStage(
            IStage<T>* stage,
            std::vector<T>& items,
            const ExecutionContext<T>& ctx) {

            StageStatistics stats;
            stats.itemsIn = items.size();
            auto startTime = std::chrono::steady_clock::now();

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
    #if 0
                case StageType::Batch:
                    ExecuteBatch(static_cast<IBatchStage<T>*>(stage), items, stats, ctx);
                    break;
    #endif
            }

            // Teardown
            stage->Teardown(items, ctx);

            stats.itemsOut = items.size();
            stats.elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);

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

            for (auto& item : items) {
                _pool.emplace_back([&] {
                    if (!stage->ShouldProcess(item)) {
                        return;
                    }

                    if (auto result = stage->ProcessItem(item, ctx)) {
                        succeeded.fetch_add(1, std::memory_order_relaxed);
                    } else {
                        failed.fetch_add(1, std::memory_order_relaxed);
                        {
                            std::lock_guard lock(errorMutex);
                            stats.errors.emplace_back(GetItemName(item), std::move(result.error()));
                        }
                    }
                });
            }

            _pool.wait();

            stats.succeeded = succeeded.load();
            stats.failed = failed.load();
        }

        void ExecuteBarrier(
            IBarrierStage<T>* stage,
            std::vector<T>& items,
            StageStatistics& stats,
            const ExecutionContext<T>& ctx) {

            size_t originalSize = items.size();

            // Process all items together
            if (auto result = stage->ProcessAll(items, ctx)) {
                stats.succeeded = originalSize;
                stats.failed = 0;
            } else {
                stats.succeeded = 0;
                stats.failed = originalSize;
                stats.errors.emplace_back(stage->GetName(), std::move(result.error()));
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

                if (auto result = stage->ProcessItem(item, i, total, ctx)) {
                    ++stats.succeeded;
                } else {
                    ++stats.failed;
                    stats.errors.emplace_back(GetItemName(item), std::move(result.error()));
                }
            }
        }
    #if 0
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
                        _pool.submit_sequence(0, batch.size(), [&](size_t i) {
                            auto& item = batch[i];

                            if (!stage->ShouldProcess(item)) {
                                return;
                            }

                            auto result = stage->ProcessItem(item, ctx);

                            if (result.has_value()) {
                                succeeded.fetch_add(1, std::memory_order_relaxed);
                            } else {
                                failed.fetch_add(1, std::memory_order_relaxed);
                                {
                                    std::lock_guard lock(errorMutex);
                                    stats.errors.emplace_back(GetItemName(item), std::move(result.error()));
                                }
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
                            } else {
                                failed.fetch_add(1, std::memory_order_relaxed);
                                stats.errors.emplace_back(GetItemName(item), std::move(result.error()));
                            }
                        }
                    }
                } else {
                    // Batch processing failed
                    failed.fetch_add(batch.size());
                    {
                        std::lock_guard lock(errorMutex);
                        stats.errors.emplace_back(std::format("Batch {}", batchIdx), std::move(batchResult.error()));
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
    #endif

        static std::string GetItemName(const T& item) {
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
        glz::pool _pool;
        //std::shared_ptr<ILogger> _logger;
        //std::shared_ptr<IProgressReporter> _reporter;
        //std::shared_ptr<IMetricsCollector> _metrics;
    };
}