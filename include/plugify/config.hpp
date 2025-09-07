#pragma once

#include <any>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_set>

#include "plugify/logger.hpp"
#include "plugify/types.hpp"
#include "plugify/platform_ops.hpp"

namespace plugify {
    // Update mode for explicit control
    enum class UpdateMode {
        Manual,           // User calls Update() manually
        BackgroundThread, // Automatic updates in background thread
        Callback         // User provides update callback
    };

    // Configuration source priority
    enum class ConfigSource {
        Default,    // Default values
        File,       // Loaded from file
        Builder,    // Set via builder methods
        Override    // Explicit overrides (highest priority)
    };

    struct Config {
        // Track source priority for each field group
        struct SourceTracking {
            ConfigSource paths = ConfigSource::Default;
            ConfigSource loading = ConfigSource::Default;
            ConfigSource runtime = ConfigSource::Default;
            ConfigSource security = ConfigSource::Default;
            ConfigSource logging = ConfigSource::Default;
        } _sources;

        // Paths configuration
        struct Paths {
            std::filesystem::path baseDir;
            std::filesystem::path extensionsDir = "extensions";
            std::filesystem::path configsDir = "configs";
            std::filesystem::path dataDir = "data";
            std::filesystem::path logsDir = "logs";
            std::filesystem::path cacheDir = "cache";
            
            // Check if paths have non-default values
            bool HasCustomExtensionsDir() const { return extensionsDir != "extensions"; }
            bool HasCustomConfigsDir() const { return configsDir != "configs"; }
            bool HasCustomDataDir() const { return dataDir != "data"; }
            bool HasCustomLogsDir() const { return logsDir != "logs"; }
            bool HasCustomCacheDir() const { return cacheDir != "cache"; }
            
            void ResolveRelativePaths() {
                auto makeAbsolute = [this](std::filesystem::path& path) {
                    if (!path.empty() && path.is_relative() && !baseDir.empty()) {
                        path = (baseDir / path).lexically_normal();
                    }
                };
                
                makeAbsolute(extensionsDir);
                makeAbsolute(configsDir);
                makeAbsolute(dataDir);
                makeAbsolute(logsDir);
                makeAbsolute(cacheDir);
            }
        } paths;

        // Loading configuration
        struct Loading {
            bool preferOwnSymbols = true;
            size_t maxConcurrentLoads = 4;
            std::chrono::milliseconds loadTimeout{500};
            std::chrono::milliseconds exportTimeout{100};
            std::chrono::milliseconds startTimeout{250};
            
            // Check if values are non-default
            bool HasCustomPreferOwnSymbols() const { return preferOwnSymbols != true; }
            bool HasCustomMaxConcurrentLoads() const { return maxConcurrentLoads != 4; }
            bool HasCustomLoadTimeout() const { return loadTimeout != std::chrono::milliseconds{500}; }
            bool HasCustomExportTimeout() const { return exportTimeout != std::chrono::milliseconds{100}; }
            bool HasCustomStartTimeout() const { return startTimeout != std::chrono::milliseconds{250}; }
        } loading;

        // Runtime configuration
        struct Runtime {
            bool pinToMainThread = false;
            UpdateMode updateMode = UpdateMode::Manual;
            std::chrono::milliseconds updateInterval{16};
            std::function<void(std::chrono::milliseconds)> updateCallback;
            std::optional<size_t> threadPriority;
            
            // Check if values are non-default
            bool HasCustomPinToMainThread() const { return pinToMainThread != false; }
            bool HasCustomUpdateMode() const { return updateMode != UpdateMode::Manual; }
            bool HasCustomUpdateInterval() const { return updateInterval != std::chrono::milliseconds{16}; }
            bool HasThreadPriority() const { return threadPriority.has_value(); }
        } runtime;

        // Security configuration
        struct Security {
            std::unordered_set<std::string> whitelistedExtensions;
            std::unordered_set<std::string> blacklistedExtensions;
            
            bool HasWhitelist() const { return !whitelistedExtensions.empty(); }
            bool HasBlacklist() const { return !blacklistedExtensions.empty(); }
        } security;

        // Logging configuration
        struct Logging {
            constexpr static auto kVerbosity = PLUGIFY_IS_DEBUG ? Severity::Debug : Severity::Error;

            Severity severity{kVerbosity};
            bool printReport = true;
            bool printLoadOrder = true;
            bool printDependencyGraph = true;
            bool printDigraphDot = true;
            std::filesystem::path exportDigraphDot;
            
            bool HasCustomSeverity() const { return severity != kVerbosity; }
            bool HasCustomPrintReport() const { return printReport != true; }
            bool HasCustomPrintLoadOrder() const { return printLoadOrder != true; }
            bool HasCustomPrintDependencyGraph() const { return printDependencyGraph != true; }
            bool HasCustomPrintDigraphDot() const { return printDigraphDot != true; }
            bool HasExportPath() const { return !exportDigraphDot.empty(); }
        } logging;

        // Comprehensive merge implementation
        void MergeFrom(const Config& other, ConfigSource source = ConfigSource::Override) {
            // Only merge if the new source has equal or higher priority
            
            // Merge Paths
            if (source >= _sources.paths) {
                bool pathsChanged = false;
                
                // Always take baseDir if provided
                if (!other.paths.baseDir.empty()) {
                    paths.baseDir = other.paths.baseDir;
                    pathsChanged = true;
                }
                
                // Take other paths if they're customized
                if (other.paths.HasCustomExtensionsDir()) {
                    paths.extensionsDir = other.paths.extensionsDir;
                    pathsChanged = true;
                }
                if (other.paths.HasCustomConfigsDir()) {
                    paths.configsDir = other.paths.configsDir;
                    pathsChanged = true;
                }
                if (other.paths.HasCustomDataDir()) {
                    paths.dataDir = other.paths.dataDir;
                    pathsChanged = true;
                }
                if (other.paths.HasCustomLogsDir()) {
                    paths.logsDir = other.paths.logsDir;
                    pathsChanged = true;
                }
                if (other.paths.HasCustomCacheDir()) {
                    paths.cacheDir = other.paths.cacheDir;
                    pathsChanged = true;
                }
                
                if (pathsChanged) {
                    _sources.paths = source;
                }
            }
            
            // Merge Loading
            if (source >= _sources.loading) {
                bool loadingChanged = false;
                
                if (other.loading.HasCustomPreferOwnSymbols()) {
                    loading.preferOwnSymbols = other.loading.preferOwnSymbols;
                    loadingChanged = true;
                }
                if (other.loading.HasCustomMaxConcurrentLoads()) {
                    loading.maxConcurrentLoads = other.loading.maxConcurrentLoads;
                    loadingChanged = true;
                }
                if (other.loading.HasCustomLoadTimeout()) {
                    loading.loadTimeout = other.loading.loadTimeout;
                    loadingChanged = true;
                }
                if (other.loading.HasCustomExportTimeout()) {
                    loading.exportTimeout = other.loading.exportTimeout;
                    loadingChanged = true;
                }
                if (other.loading.HasCustomStartTimeout()) {
                    loading.startTimeout = other.loading.startTimeout;
                    loadingChanged = true;
                }
                
                if (loadingChanged) {
                    _sources.loading = source;
                }
            }
            
            // Merge Runtime
            if (source >= _sources.runtime) {
                bool runtimeChanged = false;
                
                if (other.runtime.HasCustomUpdateMode()) {
                    runtime.updateMode = other.runtime.updateMode;
                    runtimeChanged = true;
                }
                if (other.runtime.HasCustomUpdateInterval()) {
                    runtime.updateInterval = other.runtime.updateInterval;
                    runtimeChanged = true;
                }
                if (other.runtime.updateCallback) {
                    runtime.updateCallback = other.runtime.updateCallback;
                    runtimeChanged = true;
                }
                if (other.runtime.HasCustomPinToMainThread()) {
                    runtime.pinToMainThread = other.runtime.pinToMainThread;
                    runtimeChanged = true;
                }
                if (other.runtime.HasThreadPriority()) {
                    runtime.threadPriority = other.runtime.threadPriority;
                    runtimeChanged = true;
                }
                
                if (runtimeChanged) {
                    _sources.runtime = source;
                }
            }
            
            // Merge Security
            if (source >= _sources.security) {
                bool securityChanged = false;
                
                // For sets, we have different merge strategies
                if (source == ConfigSource::Override) {
                    // Override replaces entirely if non-empty
                    if (other.security.HasWhitelist()) {
                        security.whitelistedExtensions = other.security.whitelistedExtensions;
                        securityChanged = true;
                    }
                    if (other.security.HasBlacklist()) {
                        security.blacklistedExtensions = other.security.blacklistedExtensions;
                        securityChanged = true;
                    }
                } else {
                    // Other sources merge/append
                    if (other.security.HasWhitelist()) {
                        security.whitelistedExtensions.insert(
                            other.security.whitelistedExtensions.begin(),
                            other.security.whitelistedExtensions.end()
                        );
                        securityChanged = true;
                    }
                    if (other.security.HasBlacklist()) {
                        security.blacklistedExtensions.insert(
                            other.security.blacklistedExtensions.begin(),
                            other.security.blacklistedExtensions.end()
                        );
                        securityChanged = true;
                    }
                }
                
                if (securityChanged) {
                    _sources.security = source;
                }
            }
            
            // Merge Logging
            if (source >= _sources.logging) {
                bool loggingChanged = false;
                
                if (other.logging.HasCustomSeverity()) {
                    logging.severity = other.logging.severity;
                    loggingChanged = true;
                }
                if (other.logging.HasCustomPrintReport()) {
                    logging.printReport = other.logging.printReport;
                    loggingChanged = true;
                }
                if (other.logging.HasCustomPrintLoadOrder()) {
                    logging.printLoadOrder = other.logging.printLoadOrder;
                    loggingChanged = true;
                }
                if (other.logging.HasCustomPrintDependencyGraph()) {
                    logging.printDependencyGraph = other.logging.printDependencyGraph;
                    loggingChanged = true;
                }
                if (other.logging.HasCustomPrintDigraphDot()) {
                    logging.printDigraphDot = other.logging.printDigraphDot;
                    loggingChanged = true;
                }
                if (other.logging.HasExportPath()) {
                    logging.exportDigraphDot = other.logging.exportDigraphDot;
                    loggingChanged = true;
                }
                
                if (loggingChanged) {
                    _sources.logging = source;
                }
            }
            
            // Always resolve paths after merge
            paths.ResolveRelativePaths();
        }

        // Alternative merge for selective field updates
        /*void MergeField(std::string_view fieldPath, const Config& other, ConfigSource source = ConfigSource::Override) {
            // This allows merging specific fields only
            if (fieldPath == "paths") {
                if (source >= _sources.paths) {
                    paths = other.paths;
                    _sources.paths = source;
                    paths.ResolveRelativePaths();
                }
            } else if (fieldPath == "loading") {
                if (source >= _sources.loading) {
                    loading = other.loading;
                    _sources.loading = source;
                }
            } else if (fieldPath == "runtime") {
                if (source >= _sources.runtime) {
                    runtime = other.runtime;
                    _sources.runtime = source;
                }
            } else if (fieldPath == "security") {
                if (source >= _sources.security) {
                    security = other.security;
                    _sources.security = source;
                }
            } else if (fieldPath == "logging") {
                if (source >= _sources.logging) {
                    logging = other.logging;
                    _sources.logging = source;
                }
            }
        }

        // Get effective source for debugging
        std::string GetSourceInfo() const {
            return std::format(
                "Config sources - Paths: {}, Loading: {}, Runtime: {}, Security: {}, Logging: {}",
                plg::enum_to_string(_sources.paths),
                plg::enum_to_string(_sources.loading),
                plg::enum_to_string(_sources.runtime),
                plg::enum_to_string(_sources.security),
                plg::enum_to_string(_sources.logging)
            );
        }*/

        // Reset to defaults
        void Reset() {
            *this = Config{};
        }

        // Validation remains the same
        Result<void> Validate() const {
            if (paths.baseDir.empty()) {
                return MakeError("Base directory not set");
            }
            
            // Check for path collisions
            using pair = std::pair<std::string_view, const std::filesystem::path*>;
            std::array pathList = {
                pair{"extensions", &paths.extensionsDir},
                pair{"configs", &paths.configsDir},
                pair{"data", &paths.dataDir},
                pair{"logs", &paths.logsDir},
                pair{"cache", &paths.cacheDir}
            };
            
            for (size_t i = 0; i < pathList.size(); ++i) {
                for (size_t j = i + 1; j < pathList.size(); ++j) {
                    if (*pathList[i].second == *pathList[j].second) {
                        return MakeError("{} and {} directories are the same: {}",
                                      pathList[i].first, 
                                      pathList[j].first,
                                      plg::as_string(*pathList[i].second)
                        );
                    }
                }
            }
            
            // Validate runtime config
            if (runtime.updateMode == UpdateMode::BackgroundThread && 
                runtime.updateInterval <= std::chrono::milliseconds{0}) {
                return MakeError("Invalid update interval for background thread mode");
            }
            
            if (runtime.updateMode == UpdateMode::Callback && !runtime.updateCallback) {
                return MakeError("Update callback not set for callback mode");
            }
            
            return {};
        }
    };
} // namespace plugify
