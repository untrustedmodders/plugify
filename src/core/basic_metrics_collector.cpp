#include "plugify/core/registrar.hpp"

#include "core/basic_metrics_collector.hpp"
#include "core/glaze_metadata.hpp"
#if 0
using namespace plugify;

// Recording methods
void BasicMetricsCollector::RecordExtensionTiming(UniqueId id, ExtensionState state, Duration time) {
    std::lock_guard lock(_mutex);
    _extensions[id].timings[state] = time;
    _extensions[id].lastUpdate = Clock::now();
}

void BasicMetricsCollector::RecordExtensionFailure(UniqueId id, ExtensionState failedState, std::string_view error) {
    std::lock_guard lock(_mutex);
    auto& ext = _extensions[id];
    ++ext.failures;
    ext.lastFailureState = failedState;
    ext.lastError = error;
    ++_totalFailures;
}

void BasicMetricsCollector::RecordExtensionRecovery(UniqueId id, ExtensionState fromState) {
    std::lock_guard lock(_mutex);
    ++_extensions[id].recoveries;
}

void BasicMetricsCollector::RecordUpdate(UniqueId id, Duration updateTime) {
    // Use atomic operations for hot path
    auto it = _extensions.find(id);
    if (it != _extensions.end()) {
        it->second.updateCount.fetch_add(1, std::memory_order_relaxed);
        it->second.totalUpdateTime.fetch_add(updateTime.count(), std::memory_order_relaxed);

        // Update max time
        auto current = it->second.maxUpdateTime.load(std::memory_order_relaxed);
        while (updateTime.count() > current) {
            if (it->second.maxUpdateTime.compare_exchange_weak(current, updateTime.count())) {
                break;
            }
        }
    }
}

void BasicMetricsCollector::RecordMemoryUsage(UniqueId id, size_t bytes) {
    std::lock_guard lock(_mutex);
    _extensions[id].memoryUsage = bytes;
}

void BasicMetricsCollector::RecordPipelineRun(Duration totalTime, size_t itemsProcessed) {
    std::lock_guard lock(_mutex);
    _lastPipelineTime = totalTime;
    _lastPipelineItems = itemsProcessed;
    ++_pipelineRuns;
}

void BasicMetricsCollector::RecordStageExecution(std::string_view stageName, Duration time, size_t succeeded, size_t failed) {
    std::lock_guard lock(_mutex);
    auto& stage = _stages[std::string(stageName)];
    ++stage.executions;
    stage.totalTime += time;
    stage.totalSucceeded += succeeded;
    stage.totalFailed += failed;
}

void BasicMetricsCollector::RecordTimeout(UniqueId id, ExtensionState state, Duration actualTime, Duration timeout) {
    std::lock_guard lock(_mutex);
    ++_extensions[id].timeouts;
    ++_totalTimeouts;

    _timeoutLog.push_back({
        .extensionId = id,
        .state = state,
        .actualTime = actualTime,
        .expectedTimeout = timeout,
        .when = Clock::now()
    });

    // Keep only last 100 timeouts
    if (_timeoutLog.size() > 100) {
        _timeoutLog.erase(_timeoutLog.begin());
    }
}

void BasicMetricsCollector::RecordDependencyCount(UniqueId id, size_t directDeps, size_t totalDeps) {
    std::lock_guard lock(_mutex);
    _extensions[id].directDependencies = directDeps;
    _extensions[id].totalDependencies = totalDeps;
}

void BasicMetricsCollector::RecordCascadingFailure(std::span<const UniqueId> failureChain) {
    std::lock_guard lock(_mutex);
    ++_cascadingFailures;
    _failureChains.emplace_back(failureChain.begin(), failureChain.end());

    // Keep only last 10 chains
    if (_failureChains.size() > 10) {
        _failureChains.pop_front();
    }
}

void BasicMetricsCollector::RecordCircularDependency(std::span<const UniqueId> cycle) {
    std::lock_guard lock(_mutex);
    _circularDependencies.emplace_back(cycle.begin(), cycle.end());
}

// Analysis methods
std::vector<PerformanceIssue> BasicMetricsCollector::AnalyzePerformance() const {
    std::lock_guard lock(_mutex);
    std::vector<PerformanceIssue> issues;

    using Severity = PerformanceIssue::Severity;
    using Category = PerformanceIssue::Category;

    for (const auto& [id, metrics] : _extensions) {
        // Check for slow initialization
        Duration totalInit{0};
        for (const auto& [state, time] : metrics.timings) {
            if (state == ExtensionState::Loading ||
                state == ExtensionState::Starting ||
                state == ExtensionState::Parsing) {
                totalInit += time;
            }
        }

        if (totalInit > std::chrono::seconds(5)) {
            issues.push_back({
                .category = Category::SlowInit,
                .severity = totalInit > std::chrono::seconds(10) ? Severity::Critical : Severity::High,
                .extensionId = id,
                .description = std::format("Initialization took {}ms", totalInit.count()),
                .measuredValue = totalInit
            });
        }

        // Check for slow updates
        if (metrics.updateCount > 0) {
            auto avgUpdate = Duration(metrics.totalUpdateTime.load()) / metrics.updateCount.load();
            if (avgUpdate > std::chrono::milliseconds(16)) {  // Over 60fps budget
                issues.push_back({
                    .category = Category::SlowUpdate,
                    .severity = avgUpdate > std::chrono::milliseconds(33) ? Severity::High : Severity::Medium,
                    .extensionId = id,
                    .description = std::format("Avg update {}ms (max {}ms)",
                                              avgUpdate.count(),
                                              metrics.maxUpdateTime.load()),
                    .measuredValue = avgUpdate
                });
            }
        }

        // Check for frequent failures
        if (metrics.failures > 5) {
            issues.push_back({
                .category = Category::FrequentFailures,
                .severity = metrics.failures > 20 ? Severity::Critical : metrics.failures > 10 ? Severity::High : Severity::Medium,
                .extensionId = id,
                .description = std::format("{} failures, {} recoveries",
                                          metrics.failures, metrics.recoveries),
                .measuredValue = Duration(metrics.failures)
            });
        }

        // Check for timeouts
        if (metrics.timeouts > 0) {
            issues.push_back({
                .category = Category::Timeout,
                .severity = metrics.timeouts > 3 ? Severity::High : Severity::Low,
                .extensionId = id,
                .description = std::format("{} timeout(s) detected", metrics.timeouts),
                .measuredValue = Duration(metrics.timeouts)
            });
        }
    }

    return issues;
}

std::vector<std::pair<UniqueId, Duration>> BasicMetricsCollector::GetBottlenecks(size_t topN) const {
    std::lock_guard lock(_mutex);
    std::vector<std::pair<UniqueId, Duration>> bottlenecks;

    for (const auto& [id, metrics] : _extensions) {
        Duration totalTime{0};
        for (const auto& [_, time] : metrics.timings) {
            totalTime += time;
        }
        bottlenecks.emplace_back(id, totalTime);
    }

    std::partial_sort(bottlenecks.begin(),
                     bottlenecks.begin() + std::min(topN, bottlenecks.size()),
                     bottlenecks.end(),
                     [](const auto& a, const auto& b) { return a.second > b.second; });

    bottlenecks.resize(std::min(topN, bottlenecks.size()));
    return bottlenecks;
}

double BasicMetricsCollector::GetSystemHealth() const {
    std::lock_guard lock(_mutex);

    // Simple health calculation (0.0 = dead, 1.0 = perfect)
    double health = 1.0;

    // Deduct for failures
    if (_totalFailures > 0) {
        health -= std::min(0.5, _totalFailures * 0.05);
    }

    // Deduct for timeouts
    if (_totalTimeouts > 0) {
        health -= std::min(0.3, _totalTimeouts * 0.1);
    }

    // Deduct for cascading failures
    if (_cascadingFailures > 0) {
        health -= std::min(0.4, _cascadingFailures * 0.2);
    }

    return std::max(0.0, health);
}

std::string BasicMetricsCollector::GenerateSummary() const {
    std::lock_guard lock(_mutex);
    std::string summary;
    summary.reserve(256);

    auto it = std::back_inserter(summary);

    std::format_to(it, "=== Metrics Summary ===\n");
    std::format_to(it, "System Health: {:.1f}%\n", GetSystemHealth() * 100));
    std::format_to(it, "Pipeline Runs: {}\n", _pipelineRuns));
    std::format_to(it, "Total Failures: {}\n", _totalFailures.load());
    std::format_to(it, "Total Timeouts: {}\n", _totalTimeouts.load());
    std::format_to(it, "Cascading Failures: {}\n\n", _cascadingFailures);

    auto bottlenecks = GetBottlenecks(3);
    if (!bottlenecks.empty()) {
        std::format_to(it, "Top Bottlenecks:\n");
        for (const auto& [id, time] : bottlenecks) {
            std::format_to(it, "  Extension {}: {}ms\n", id, time.count());
        }
    }

    return summary;
}

std::string BasicMetricsCollector::ExportPrometheus() const {
    std::lock_guard lock(_mutex);
    std::string output;

    output += "# HELP extension_health System health score (0-1)\n";
    output += std::format("extension_health {}\n", GetSystemHealth());

    output += "# HELP extension_failures_total Total number of failures\n";
    output += std::format("extension_failures_total {}\n", _totalFailures.load());

    output += "# HELP extension_timeouts_total Total number of timeouts\n";
    output += std::format("extension_timeouts_total {}\n", _totalTimeouts.load());

    // TODO

    return output;
}

std::string BasicMetricsCollector::ExportJson() const {
    std::lock_guard lock(_mutex);

    PerformanceReport::MetricsExport export_data;

    // Set metadata
    export_data.version = "1.0.0";
    export_data.timestamp = GetCurrentISO8601();
    auto export_start = Clock::now();

    // Fill system metrics
    export_data.system.health_score = GetSystemHealth();
    export_data.system.pipeline_runs = _pipelineRuns;
    export_data.system.last_pipeline_time_ms = _lastPipelineTime.count();
    export_data.system.last_pipeline_items = _lastPipelineItems;
    export_data.system.total_extensions = _extensions.size();
    export_data.system.active_extensions = CountActiveExtensions();
    export_data.system.failed_extensions = CountFailedExtensions();
    export_data.system.total_failures = _totalFailures.load();
    export_data.system.total_timeouts = _totalTimeouts.load();
    export_data.system.cascading_failures = _cascadingFailures;
    export_data.system.circular_dependencies = _circularDependencies.size();

    // Convert extension metrics
    export_data.extensions.reserve(_extensions.size());
    for (const auto& [id, metrics] : _extensions) {
        PerformanceReport::ExtensionMetrics ext_json;
        ext_json.id = id;

        // Convert timings
        for (const auto& [state, duration] : metrics.timings) {
            ext_json.timings_ms.emplace(plg::enum_to_string(state), duration.count());
        }

        // Calculate update statistics
        ext_json.update_count = metrics.updateCount.load();
        if (ext_json.update_count > 0) {
            ext_json.avg_update_ms = metrics.totalUpdateTime.load() / ext_json.update_count;
            ext_json.max_update_ms = metrics.maxUpdateTime.load();
        }

        ext_json.failures = metrics.failures;
        ext_json.recoveries = metrics.recoveries;
        ext_json.timeouts = metrics.timeouts;
        ext_json.memory_bytes = metrics.memoryUsage;
        ext_json.direct_dependencies = metrics.directDependencies;
        ext_json.total_dependencies = metrics.totalDependencies;
        ext_json.last_error = metrics.lastError;
        ext_json.last_update_iso8601 = TimePointToISO8601(metrics.lastUpdate);

        export_data.extensions.push_back(std::move(ext_json));
    }

    // Convert stage metrics
    export_data.stages.reserve(_stages.size());
    for (const auto& [name, metrics] : _stages) {
        PerformanceReport::StageMetrics stage_json;
        stage_json.name = name;
        stage_json.executions = metrics.executions;
        stage_json.total_time_ms = metrics.totalTime.count();
        stage_json.avg_time_ms = metrics.executions > 0 ?
            static_cast<double>(metrics.totalTime.count()) / metrics.executions : 0.0;
        stage_json.succeeded = metrics.totalSucceeded;
        stage_json.failed = metrics.totalFailed;
        stage_json.success_rate = (metrics.totalSucceeded + metrics.totalFailed) > 0 ?
            static_cast<double>(metrics.totalSucceeded) /
            (metrics.totalSucceeded + metrics.totalFailed) : 0.0;

        export_data.stages.push_back(std::move(stage_json));
    }

    // Get performance issues
    auto issues = AnalyzePerformance();
    export_data.performance_issues.reserve(issues.size());
    for (const auto& issue : issues) {
        PerformanceReport::PerformanceIssue issue_json;
        issue_json.category = plg::enum_to_string(issue.category);
        issue_json.severity = plg::enum_to_string(issue.severity);
        issue_json.extension = ToShortString(issue.extensionId);
        issue_json.description = issue.description;
        issue_json.measured_value_ms = issue.measuredValue.count();

        export_data.performance_issues.push_back(std::move(issue_json));
    }

    // Convert recent timeouts
    export_data.recent_timeouts.reserve(_timeoutLog.size());
    for (const auto& timeout : _timeoutLog) {
        PerformanceReport::TimeoutRecord timeout_json;
        timeout_json.extension = ToShortString(timeout.extensionId);
        timeout_json.state = plg::enum_to_string(timeout.state);
        timeout_json.actual_time_ms = timeout.actualTime.count();
        timeout_json.timeout_ms = timeout.expectedTimeout.count();
        timeout_json.overtime_ms = (timeout.actualTime - timeout.expectedTimeout).count();
        timeout_json.timestamp_iso8601 = TimePointToISO8601(timeout.when);

        export_data.recent_timeouts.push_back(std::move(timeout_json));
    }

    // Copy failure chains
    export_data.failure_chains.reserve(_failureChains.size());
    for (const auto& chain : _failureChains) {
        std::vector<std::string> failures;
        failures.reserve(chain.size());
        for (const auto& el : chain) {
            failures.emplace_back(ToShortString(el));
        }
        export_data.failure_chains.push_back(std::move(failures));
    }

    // Copy circular dependencies
    export_data.circular_dependency_cycles.reserve(_circularDependencies.size());
    for (const auto& cycle : _circularDependencies) {
        std::vector<std::string> failures;
        failures.reserve(cycle.size());
        for (const auto& failure : cycle) {
            failures.emplace_back(ToShortString(failure));
        }
        export_data.circular_dependency_cycles.push_back(std::move(failures));
    }

    // Get bottlenecks
    auto bottlenecks = GetBottlenecks(5);
    export_data.bottlenecks.reserve(bottlenecks.size());
    for (const auto& [id, time] : bottlenecks) {
        PerformanceReport::MetricsExport::Bottleneck bottleneck;
        bottleneck.extension_id = id;
        bottleneck.total_time_ms = time.count();

        // Find primary stage for this extension
        if (auto it = _extensions.find(id); it != _extensions.end()) {
            Duration maxTime{0};
            ExtensionState maxState{};
            for (const auto& [state, duration] : it->second.timings) {
                if (duration > maxTime) {
                    maxTime = duration;
                    maxState = state;
                }
            }
            bottleneck.primary_stage = plg::enum_to_string(maxState);
        }

        export_data.bottlenecks.push_back(std::move(bottleneck));
    }

    // Calculate statistics
    export_data.statistics = CalculateStatistics();

    // Set export time
    export_data.export_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - export_start).count();

    // Serialize to JSON using Glaze
    std::string json_output;
    auto result = glz::write_json(export_data, json_output);

    if (!result) {
        // Fallback to simple JSON if Glaze fails
        return R"({"error": "Failed to serialize metrics"})";
    }

    return json_output;
}

// Helper methods for JSON export
std::string BasicMetricsCollector::GetCurrentISO8601() {
    auto now = std::chrono::system_clock::now();
    // Format as UTC ISO8601: YYYY-MM-DDTHH:MM:SSZ
    return std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(now));
}

std::string BasicMetricsCollector::TimePointToISO8601(Clock::time_point tp) {
    // Convert steady_clock to system_clock for formatting
    auto sys_now = std::chrono::system_clock::now();
    auto steady_now = Clock::now();
    auto duration_since = tp - steady_now;
    auto sys_tp = sys_now + duration_since;

    // Format as UTC ISO8601
    return std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(sys_tp));
}

PerformanceReport::MetricsExport::Statistics BasicMetricsCollector::CalculateStatistics() const {
    PerformanceReport::MetricsExport::Statistics stats{};

    std::vector<int64_t> init_times;
    int64_t total_init = 0;
    size_t over_budget = 0;

    for (const auto& [id, metrics] : _extensions) {
        int64_t init_time = 0;
        for (const auto& [state, duration] : metrics.timings) {
            if (state == ExtensionState::Loading ||
                state == ExtensionState::Starting ||
                state == ExtensionState::Parsing) {
                init_time += duration.count();
            }
        }
        init_times.push_back(init_time);
        total_init += init_time;

        // Check if over 16ms update budget
        if (metrics.updateCount > 0) {
            auto avg = metrics.totalUpdateTime.load() / metrics.updateCount.load();
            if (avg > 16) {
                ++over_budget;
            }
        }
    }

    if (!init_times.empty()) {
        std::sort(init_times.begin(), init_times.end());

        stats.total_init_time_ms = total_init;
        stats.avg_init_time_ms = total_init / init_times.size();
        stats.p50_init_time_ms = init_times[init_times.size() / 2];
        stats.p95_init_time_ms = init_times[static_cast<size_t>(init_times.size() * 0.95)];
        stats.p99_init_time_ms = init_times[static_cast<size_t>(init_times.size() * 0.99)];
    }

    stats.extensions_over_budget = over_budget;

    // Calculate average FPS from update times
    int64_t total_updates = 0;
    int64_t total_update_time = 0;
    for (const auto& [id, metrics] : _extensions) {
        total_updates += metrics.updateCount.load();
        total_update_time += metrics.totalUpdateTime.load();
    }

    if (total_updates > 0 && total_update_time > 0) {
        double avg_ms = static_cast<double>(total_update_time) / total_updates;
        stats.avg_update_fps = avg_ms > 0 ? 1000.0 / avg_ms : 0.0;
    }

    return stats;
}

size_t BasicMetricsCollector::CountActiveExtensions() const {
    return std::count_if(_extensions.begin(), _extensions.end(),
        [](const auto& pair) { return pair.second.failures == 0; });
}

size_t BasicMetricsCollector::CountFailedExtensions() const {
    return std::count_if(_extensions.begin(), _extensions.end(),
        [](const auto& pair) { return pair.second.failures > 0; });
}

void BasicMetricsCollector::Reset() {
    std::lock_guard lock(_mutex);
    _extensions.clear();
    _stages.clear();
    _failureChains.clear();
    _circularDependencies.clear();
    _timeoutLog.clear();
    _totalFailures = 0;
    _totalTimeouts = 0;
    _cascadingFailures = 0;
    _pipelineRuns = 0;
}

void BasicMetricsCollector::Flush() {
    // In a real implementation, this might write to disk or send to a remote service
    // For now, it's a no-op
}
#endif