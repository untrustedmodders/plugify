#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "plugify/core/log_system.hpp"
#include "plugify/core/types.hpp"

namespace plugify {

	// Retry configuration
	struct RetryPolicy {
		std::size_t maxAttempts = 3;
		std::chrono::milliseconds baseDelay{100};
		std::chrono::milliseconds maxDelay{5000};
		bool exponentialBackoff = true;
		bool retryOnlyTransient = true;
	};

	struct Config {
		// Retry configuration
		RetryPolicy retryPolicy;

		std::vector<std::filesystem::path> searchPaths;
		bool autoResolveConflicts = false;
		bool strictVersionChecking = true;
		bool loadDisabledPackages = false;

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
		std::optional<std::unordered_set<std::string>> whitelistedPackages;
		std::optional<std::unordered_set<std::string>> blacklistedPackages;



		// Other
		std::filesystem::path configsDir;
		std::filesystem::path dataDir;
		std::filesystem::path logsDir;
		std::optional<Severity> logSeverity;
	};
} // namespace plugify
