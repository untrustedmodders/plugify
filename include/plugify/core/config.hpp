#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "plugify/core/logger.hpp"

namespace plugify {
    struct Config {
        // Paths configuration
        struct Paths {
            std::filesystem::path baseDir;
            std::filesystem::path pluginsDir = "plugins";
            std::filesystem::path configsDir = "configs";
            std::filesystem::path dataDir = "data";
            std::filesystem::path logsDir = "logs";
            std::filesystem::path cacheDir = "cache";
        } paths;

        // Loading configuration
        struct Loading {
            bool preferOwnSymbols = true;
            bool enableHotReload = false;
            bool lazyLoading = false;
            bool parallelLoading = true;
            size_t maxConcurrentLoads = 4;
            std::chrono::seconds loadTimeout{30};
        } loading;

        // Runtime configuration
        struct Runtime {
            bool enableSandboxing = false;
            bool enableProfiling = false;
            size_t maxMemoryPerPlugin = 0; // 0 = unlimited
            std::chrono::milliseconds updateInterval{16}; // ~60 FPS
        } runtime;

        // Security configuration
        struct Security {
            //bool verifySignatures = false;
            //bool allowUnsignedPlugins = true;
            //std::vector<std::string> trustedPublishers;
            std::vector<std::string> whitelistedPackages;
            std::vector<std::string> blacklistedPackages;
        } security;

        // Logging configuration
        struct Logging {
            bool printReport = false;
            bool printLoadOrder = false;
            bool printDependencyGraph = false;
            std::filesystem::path exportDigraphDot;
            Severity severity{Severity::Error};
        } logging;

        // Retry configuration
        /*struct RetryPolicy {
            std::size_t maxAttempts = 3;
            std::chrono::milliseconds baseDelay{100};
            std::chrono::milliseconds maxDelay{5000};
            bool exponentialBackoff = true;
            bool retryOnlyTransient = true;
        } retryPolicy;*/
    };


	/*
	struct Config {
		// Initialization behavior
		bool partialStartupMode = true;
		bool failOnMissingDependencies = false;
		//bool failOnModuleError = false;
		//bool continueOnValidationWarnings = true;
		bool respectDependencyOrder = true;  // Initialize in dependency order
		bool skipDependentsOnFailure = true; // Skip packages if their dependencies fail
		bool printSummary = true;  // Print initialization summary to console

		// Update behavior
		//bool failOnUpdateError = false;      // Whether to fail if any package update fails
		//bool verboseUpdates = false;         // Log update errors to console
		//bool trackUpdatePerformance = true;  // Track update timing statistics
		//std::chrono::microseconds slowUpdateThreshold{16667}; // ~60 FPS threshold

		// Timeouts
		//std::chrono::milliseconds initializationTimeout{30000};
		//std::chrono::milliseconds perPackageTimeout{5000};


		// Filtering
	};*/
} // namespace plugify
