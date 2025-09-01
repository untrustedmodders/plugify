#pragma once

#include "plugify/metric_collector.hpp"

namespace plugify {
#if 0
    // ============================================================================
    // Null Implementation (No-op for when metrics are disabled)
    // ============================================================================

    class NullMetricsCollector : public IMetricsCollector {
    public:
        void RecordExtensionTiming(UniqueId, ExtensionState, Duration) override {}
        void RecordExtensionFailure(UniqueId, ExtensionState, std::string_view) override {}
        void RecordExtensionRecovery(UniqueId, ExtensionState) override {}
        void RecordUpdate(UniqueId, Duration) override {}
        void RecordMemoryUsage(UniqueId, size_t) override {}
        void RecordPipelineRun(Duration, size_t) override {}
        void RecordStageExecution(std::string_view, Duration, size_t, size_t) override {}
        void RecordTimeout(UniqueId, ExtensionState, Duration, Duration) override {}
        void RecordDependencyCount(UniqueId, size_t, size_t) override {}
        void RecordCascadingFailure(std::span<const UniqueId>) override {}
        void RecordCircularDependency(std::span<const UniqueId>) override {}

        std::vector<PerformanceIssue> AnalyzePerformance() const override { return {}; }
        std::vector<std::pair<UniqueId, Duration>> GetBottlenecks(size_t) const override { return {}; }
        double GetSystemHealth() const override { return 1.0; }

        std::string GenerateSummary() const override { return "Metrics disabled"; }
        std::string ExportPrometheus() const override { return ""; }
        std::string ExportJson() const override { return "{}"; }

        void Reset() override {}
        void Flush() override {}
    };

    // ============================================================================
    // Basic Implementation
    // ============================================================================

    namespace PerformanceReport {
        // Extension state as string for JSON
        struct ExtensionState {
            std::string state;
            int64_t duration_ms;
        };

        // Individual extension metrics for JSON export
        struct ExtensionMetrics {
            size_t id;
            std::map<std::string, int64_t> timings_ms;  // state -> milliseconds
            size_t update_count;
            int64_t avg_update_ms;
            int64_t max_update_ms;
            size_t failures;
            size_t recoveries;
            size_t timeouts;
            size_t memory_bytes;
            size_t direct_dependencies;
            size_t total_dependencies;
            std::string last_error;
            std::string last_update_iso8601;
        };

        // Stage execution metrics
        struct StageMetrics {
            std::string name;
            size_t executions;
            int64_t total_time_ms;
            double avg_time_ms;
            size_t succeeded;
            size_t failed;
            double success_rate;
        };

        // Performance issue for JSON
        struct PerformanceIssue {
            std::string category;
            std::string severity;
            std::string extension;
            std::string description;
            int64_t measured_value_ms;
        };

        // Timeout record for JSON
        struct TimeoutRecord {
            std::string extension;
            std::string state;
            int64_t actual_time_ms;
            int64_t timeout_ms;
            int64_t overtime_ms;
            std::string timestamp_iso8601;
        };

        // Complete metrics export structure
        struct MetricsExport {
            // Metadata
            std::string version = "1.0.0";
            std::string timestamp;
            int64_t export_time_ms;

            // System metrics
            struct SystemMetrics {
                double health_score;
                size_t pipeline_runs;
                int64_t last_pipeline_time_ms;
                size_t last_pipeline_items;
                size_t total_extensions;
                size_t active_extensions;
                size_t failed_extensions;
                size_t total_failures;
                size_t total_timeouts;
                size_t cascading_failures;
                size_t circular_dependencies;
            } system;

            // Extension details
            std::vector<ExtensionMetrics> extensions;

            // Stage performance
            std::vector<StageMetrics> stages;

            // Issues and diagnostics
            std::vector<PerformanceIssue> performance_issues;
            std::vector<TimeoutRecord> recent_timeouts;
            std::vector<std::vector<std::string>> failure_chains;
            std::vector<std::vector<std::string>> circular_dependency_cycles;

            // Top bottlenecks
            struct Bottleneck {
                size_t extension_id;
                int64_t total_time_ms;
                std::string primary_stage;
            };
            std::vector<Bottleneck> bottlenecks;

            // Summary statistics
            struct Statistics {
                int64_t total_init_time_ms;
                int64_t avg_init_time_ms;
                int64_t p50_init_time_ms;
                int64_t p95_init_time_ms;
                int64_t p99_init_time_ms;
                double avg_update_fps;
                size_t extensions_over_budget;
            } statistics;
        };
    }

    class BasicMetricsCollector : public IMetricsCollector {
    public:
        BasicMetricsCollector() = default;

        // Recording methods
        void RecordExtensionTiming(UniqueId id, ExtensionState state, Duration time) override;

        void RecordExtensionFailure(UniqueId id, ExtensionState failedState, std::string_view error) override;

        void RecordExtensionRecovery(UniqueId id, ExtensionState fromState) override;

        void RecordUpdate(UniqueId id, Duration updateTime) override;

        void RecordMemoryUsage(UniqueId id, size_t bytes) override;

        void RecordPipelineRun(Duration totalTime, size_t itemsProcessed) override;

        void RecordStageExecution(std::string_view stageName, Duration time, size_t succeeded, size_t failed) override;

        void RecordTimeout(UniqueId id, ExtensionState state, Duration actualTime, Duration timeout) override;

        void RecordDependencyCount(UniqueId id, size_t directDeps, size_t totalDeps) override;

        void RecordCascadingFailure(std::span<const UniqueId> failureChain) override;

        void RecordCircularDependency(std::span<const UniqueId> cycle) override;

        // Analysis methods
        std::vector<PerformanceIssue> AnalyzePerformance() const override;

        std::vector<std::pair<UniqueId, Duration>> GetBottlenecks(size_t topN = 5) const override;

        double GetSystemHealth() const override;

        std::string GenerateSummary() const override;

        std::string ExportPrometheus() const override;

        std::string ExportJson() const override;

    private:
        static std::string GetCurrentISO8601();

        static std::string TimePointToISO8601(Clock::time_point tp);

        PerformanceReport::MetricsExport::Statistics CalculateStatistics() const;

        size_t CountActiveExtensions() const;

        size_t CountFailedExtensions() const;

    public:
        void Reset() override;

        void Flush() override;

    private:
        using Clock = std::chrono::steady_clock;

        struct ExtensionMetrics {
            plg::flat_map<ExtensionState, Duration> timings;
            std::atomic<size_t> updateCount{0};
            std::atomic<Duration::rep> totalUpdateTime{0};
            std::atomic<Duration::rep> maxUpdateTime{0};
            size_t failures{0};
            size_t recoveries{0};
            size_t timeouts{0};
            size_t memoryUsage{0};
            size_t directDependencies{0};
            size_t totalDependencies{0};
            ExtensionState lastFailureState{};
            std::string lastError;
            Clock::time_point lastUpdate;
        };

        struct StageMetrics {
            size_t executions{0};
            Duration totalTime{0};
            size_t totalSucceeded{0};
            size_t totalFailed{0};
        };

        struct TimeoutRecord {
            UniqueId extensionId;
            ExtensionState state;
            Duration actualTime;
            Duration expectedTimeout;
            Clock::time_point when;
        };

        mutable std::mutex _mutex;
        std::unordered_map<UniqueId, ExtensionMetrics> _extensions;
        std::unordered_map<std::string, StageMetrics> _stages;
        std::deque<std::vector<UniqueId>> _failureChains;
        std::vector<std::vector<UniqueId>> _circularDependencies;
        std::vector<TimeoutRecord> _timeoutLog;

        std::atomic<size_t> _totalFailures{0};
        std::atomic<size_t> _totalTimeouts{0};
        size_t _cascadingFailures{0};
        size_t _pipelineRuns{0};
        Duration _lastPipelineTime{0};
        size_t _lastPipelineItems{0};
    };
#endif
}