#pragma once

#include "plugify/extension.hpp"

namespace plugify {
#if 0
    enum class MetricType {
        Counter,    // Monotonically increasing value
        Gauge,      // Value that can go up and down
        Histogram,  // Distribution of values
        Timer       // Duration measurements
    };

    // Performance issue for analysis
    struct PerformanceIssue {
        enum class Severity { Low, Medium, High, Critical };
        enum class Category { SlowInit, SlowUpdate, FrequentFailures, Timeout, Bottleneck };

        Category category;
        Severity severity;
        UniqueId extensionId;
        std::string description;
        Duration measuredValue{0};
    };

    // ============================================================================
    // IMetricsCollector Interface
    // ============================================================================

    class IMetricsCollector {
    public:
        virtual ~IMetricsCollector() = default;

        // ========================================
        // Recording Methods
        // ========================================

        // Extension lifecycle events
        virtual void RecordExtensionTiming(UniqueId id, ExtensionState state, Duration time) = 0;
        virtual void RecordExtensionFailure(UniqueId id, ExtensionState failedState, std::string_view error) = 0;
        virtual void RecordExtensionRecovery(UniqueId id, ExtensionState fromState) = 0;

        // Runtime performance
        virtual void RecordUpdate(UniqueId id, Duration updateTime) = 0;
        virtual void RecordMemoryUsage(UniqueId id, size_t bytes) = 0;

        // System-wide events
        virtual void RecordPipelineRun(Duration totalTime, size_t itemsProcessed) = 0;
        virtual void RecordStageExecution(std::string_view stageName, Duration time, size_t succeeded, size_t failed) = 0;
        virtual void RecordTimeout(UniqueId id, ExtensionState state, Duration actualTime, Duration timeout) = 0;

        // Dependency tracking
        virtual void RecordDependencyCount(UniqueId id, size_t directDeps, size_t totalDeps) = 0;
        virtual void RecordCascadingFailure(std::span<const UniqueId> failureChain) = 0;
        virtual void RecordCircularDependency(std::span<const UniqueId> cycle) = 0;

        // ========================================
        // Query Methods
        // ========================================

        // Analysis
        virtual std::vector<PerformanceIssue> AnalyzePerformance() const = 0;
        virtual std::vector<std::pair<UniqueId, Duration>> GetBottlenecks(size_t topN = 5) const = 0;
        virtual double GetSystemHealth() const = 0;  // 0.0 = critical, 1.0 = healthy

        // Export formats
        virtual std::string GenerateSummary() const = 0;
        virtual std::string ExportPrometheus() const = 0;
        virtual std::string ExportJson() const = 0;

        // Management
        virtual void Reset() = 0;
        virtual void Flush() = 0;  // Force write any buffered metrics
    };
#endif
}