#include "plugify/core/report.hpp"

#include <glaze/glaze.hpp>

using namespace plugify;

std::string ValidationReport::GenerateTextReport() const {
	std::stringstream report;

	report << "=== Validation Report ===\n";
	report << std::format("Total packages validated: {}\n",
						  moduleResults.size() + pluginResults.size());
	report << std::format("Validation failures: {}\n\n", FailureCount());

	if (!moduleResults.empty()) {
		report << "Module Validations:\n";
		for (const auto& result : moduleResults) {
			if (result.passed) {
				report << std::format("  ✓ {}\n", result.id);
			} else {
				report << std::format("  ✗ {} - FAILED\n", result.id);
				if (result.error) {
					report << std::format("     Reason: {}\n", result.error->message);
				}
			}
			for (const auto& warning : result.warnings) {
				report << std::format("     ⚠ Warning: {}\n", warning);
			}
		}
	}

	if (!pluginResults.empty()) {
		report << "\nPlugin Validations:\n";
		for (const auto& result : pluginResults) {
			if (result.passed) {
				report << std::format("  ✓ {}\n", result.id);
			} else {
				report << std::format("  ✗ {} - FAILED\n", result.id);
				if (result.error) {
					report << std::format("     Reason: {}\n", result.error->message);
				}
			}
			for (const auto& warning : result.warnings) {
				report << std::format("     ⚠ Warning: {}\n", warning);
			}
		}
	}

	return report.str();
}

std::string ValidationReport::GenerateJsonReport() const {
	return "";//glz::write_json(*this).value_or("{\"error\": \"Failed to generate JSON report\"}");
}

std::string DependencyReport::GenerateTextReport() const {
	std::stringstream report;

	report << "=== Dependency Resolution Report ===\n\n";

	// Statistics
	report << "Statistics:\n";
	report << std::format("  Total packages: {}\n", stats.totalPackages);
	report << std::format("  Packages with issues: {}\n", stats.packagesWithIssues);
	report << std::format("  Missing dependencies: {}\n", stats.missingDependencyCount);
	report << std::format("  Optional dependencies: {}\n", stats.optionalDependencyCount);
	report << std::format("  Conflicts: {}\n", stats.conflictCount);
	report << std::format("  Version conflicts: {}\n", stats.versionConflictCount);
	report << std::format("  Circular dependencies: {}\n", stats.circularDependencyCount);
	report << "\n";

	// Version conflicts with detailed constraints
	if (!versionConflicts.empty()) {
		report << "Version Conflicts:\n";
		for (const auto& conflict : versionConflicts) {
			report << conflict.GetDetailedDescription() << "\n";
		}
	}

	// Circular dependencies
	if (!circularDependencies.empty()) {
		report << "Circular Dependencies:\n";
		for (const auto& cycle : circularDependencies) {
			report << "  " << cycle.GetCycleDescription() << "\n";
		}
		report << "\n";
	}

	// Package-specific issues with constraint details
	report << "Package Issues:\n";
	for (const auto& resolution : resolutions) {
		if (!resolution.issues.empty()) {
			report << std::format("  {}:\n", resolution.id);
			for (const auto& issue : resolution.issues) {
				report << std::format("    - {} {}\n",
									  issue.isBlocker ? "[BLOCKER]" : "[WARNING]",
									  issue.GetDetailedDescription());
			}
		}
	}

	// Load order
	if (isLoadOrderValid) {
		report << "\nRecommended Load Order:\n";
		for (size_t i = 0; i < loadOrder.size(); ++i) {
			report << std::format("  {}. {}\n", i + 1, loadOrder[i]);
		}
	} else {
		report << "\n[WARNING] No valid load order due to circular dependencies\n";
	}

	return report.str();
}

std::string DependencyReport::GenerateJsonReport() const {
	return "";//glz::write_json(*this).value_or("{\"error\": \"Failed to generate JSON report\"}");
}

std::string InitializationReport::GenerateTextReport() const {
	std::stringstream report;

	report << "=== Initialization Report ===\n";
	report << std::format("Total initialization time: {}ms\n\n", totalTime.count());

	// Module summary
	if (!moduleInits.empty()) {
		report << "Language Modules:\n";
		report << std::string(50, '-') << "\n";

		// Successful modules
		for (const auto& init : moduleInits) {
			if (init.finalState == PackageState::Ready) {
				report << std::format("  ✓ {} - {}ms", init.id, init.loadTime.count());
				if (init.retryAttempts > 0) {
					report << std::format(" (after {} retries)", init.retryAttempts);
				}
				report << "\n";
			}
		}

		// Skipped modules
		for (const auto& init : moduleInits) {
			if (init.finalState == PackageState::Skipped) {
				report << std::format("  ⚠ {} - SKIPPED\n", init.id);
				if (init.error) {
					report << std::format("     Reason: {}\n", init.error->message);
				}
			}
		}

		// Failed modules
		for (const auto& init : moduleInits) {
			if (init.finalState == PackageState::Error && init.error) {
				report << std::format("  ✗ {} - FAILED\n", init.id);
				report << std::format("     Reason: {} ({})\n",
									  init.error->message,
									  init.error->isRetryable ? "retryable" : "non-retryable");
				if (init.retryAttempts > 0) {
					report << std::format("     Attempted {} retries\n", init.retryAttempts);
				}
			}
		}
	}

	// Plugin summary
	if (!pluginInits.empty()) {
		report << "\nPlugins:\n";
		report << std::string(50, '-') << "\n";

		// Successful plugins
		for (const auto& init : pluginInits) {
			if (init.finalState == PackageState::Started) {
				report << std::format("  ✓ {} - {}ms", init.id, init.loadTime.count());
				if (init.retryAttempts > 0) {
					report << std::format(" (after {} retries)", init.retryAttempts);
				}
				report << "\n";
			}
		}

		// Skipped plugins
		for (const auto& init : pluginInits) {
			if (init.finalState == PackageState::Skipped) {
				report << std::format("  ⚠ {} - SKIPPED\n", init.id);
				if (init.error) {
					report << std::format("     Reason: {}\n", init.error->message);
				}
			}
		}

		// Failed plugins
		for (const auto& init : pluginInits) {
			if (init.finalState == PackageState::Error && init.error) {
				report << std::format("  ✗ {} - FAILED\n", init.id);
				report << std::format("     Reason: {} ({})\n",
									  init.error->message,
									  init.error->isRetryable ? "retryable" : "non-retryable");
				if (init.retryAttempts > 0) {
					report << std::format("     Attempted {} retries\n", init.retryAttempts);
				}
			}
		}
	}

	// Overall statistics
	report << "\n" << std::string(50, '-') << "\n";
	report << "Summary:\n";

	auto successfulModules = std::ranges::count_if(moduleInits,
												   [](const auto& i) { return i.finalState == PackageState::Ready; });
	auto skippedModules = std::ranges::count_if(moduleInits,
												[](const auto& i) { return i.finalState == PackageState::Skipped; });
	auto failedModules = std::ranges::count_if(moduleInits,
											   [](const auto& i) { return i.finalState == PackageState::Error; });

	auto successfulPlugins = std::ranges::count_if(pluginInits,
												   [](const auto& i) { return i.finalState == PackageState::Started; });
	auto skippedPlugins = std::ranges::count_if(pluginInits,
												[](const auto& i) { return i.finalState == PackageState::Skipped; });
	auto failedPlugins = std::ranges::count_if(pluginInits,
											   [](const auto& i) { return i.finalState == PackageState::Error; });

	report << std::format("  Modules: {} loaded, {} skipped, {} failed (total: {})\n",
						  successfulModules, skippedModules, failedModules, moduleInits.size());
	report << std::format("  Plugins: {} loaded, {} skipped, {} failed (total: {})\n",
						  successfulPlugins, skippedPlugins, failedPlugins, pluginInits.size());

	// Retry statistics
	auto totalRetries = 0;/*std::accumulate(moduleInits.begin(), moduleInits.end(), 0,
	            [](int sum, const auto& init) { return sum + static_cast<int>(init.retryAttempts); }) +
	            std::accumulate(pluginInits.begin(), pluginInits.end(), 0,
	            [](int sum, const auto& init) { return sum + static_cast<int>(init.retryAttempts); });*/

	if (totalRetries > 0) {
		report << std::format("  Total retry attempts: {}\n", totalRetries);
	}

	// Final status
	report << "\nStatus: ";
	if (FailureCount() == 0 && SkippedCount() == 0) {
		report << "✅ SUCCESS - All packages loaded\n";
	} else if (SuccessCount() > 0) {
		report << std::format("⚠️  PARTIAL SUCCESS\n");
		report << std::format("    {} packages loaded successfully\n", SuccessCount());
		if (SkippedCount() > 0) {
			report << std::format("    {} packages skipped (dependency cascade)\n", SkippedCount());
		}
		if (FailureCount() > 0) {
			report << std::format("    {} packages failed\n", FailureCount());
		}
	} else {
		report << "❌ FAILED - No packages loaded successfully\n";
	}

	return report.str();
}

std::string InitializationReport::GenerateJsonReport() const {
	return "";//glz::write_json(*this).value_or("{\"error\": \"Failed to generate JSON report\"}");
}

std::string InitializationState::GenerateTextReport() const {
	std::stringstream report;

	report << "=== COMPLETE INITIALIZATION REPORT ===\n\n";

	// Format timestamps (simplified - C++20 chrono formatting may need adaptation)
	//auto startTimeT = std::chrono::system_clock::to_time_t(startTime);
	//auto endTimeT = std::chrono::system_clock::to_time_t(endTime);

	//report << std::format("Start time: {}\n", std::ctime(&startTimeT));
	//report << std::format("End time: {}\n", std::ctime(&endTimeT));
	report << std::format("Total duration: {}ms\n\n", totalTime.count());

	report << validationReport.GenerateTextReport();
	report << "\n";
	report << dependencyReport.GenerateTextReport();
	report << "\n";
	report << initializationReport.GenerateTextReport();

	return report.str();
}

std::string InitializationState::GenerateJsonReport() const {
	return "";//glz::write_json(*this).value_or("{\"error\": \"Failed to generate JSON report\"}");
}