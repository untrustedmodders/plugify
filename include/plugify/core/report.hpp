#pragma once

#include "error.hpp"
#include "package.hpp"
#include "types.hpp"
#include "date_time.hpp"

namespace plugify {
	// ============================================================================
	// Rich Result Types for Better Error Reporting
	// ============================================================================

	struct ValidationReport {
	    struct PackageValidation {
	        PackageId id;
	        bool passed;
	        std::optional<EnhancedError> error;
	        std::vector<std::string> warnings;
	    };

	    std::vector<PackageValidation> moduleResults;
	    std::vector<PackageValidation> pluginResults;

	    bool AllPassed() const {
	        return std::ranges::all_of(moduleResults, [](const auto& r) { return r.passed; }) &&
	               std::ranges::all_of(pluginResults, [](const auto& r) { return r.passed; });
	    }

	    std::size_t FailureCount() const {
	        return std::ranges::count_if(moduleResults, [](const auto& r) { return !r.passed; }) +
	               std::ranges::count_if(pluginResults, [](const auto& r) { return !r.passed; });
	    }

	    std::vector<PackageId> GetFailedPackages() const {
	        std::vector<PackageId> failed;
	        for (const auto& r : moduleResults) {
	            if (!r.passed) failed.push_back(r.id);
	        }
	        for (const auto& r : pluginResults) {
	            if (!r.passed) failed.push_back(r.id);
	        }
	        return failed;
	    }

	    std::string GenerateTextReport() const {
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
	};

	// Enhanced DependencyReport with detailed constraint information
	struct DependencyReport {
	    // Types of dependency issues
	    enum class IssueType {
	        None,
	        MissingDependency,      // Dependency doesn't exist
	        VersionConflict,        // Incompatible version requirements
	        CircularDependency,     // A -> B -> A cycle detected
	        OptionalMissing,        // Optional dependency not available
	        TransitiveMissing,      // Indirect dependency missing
	        ConflictingProviders    // Multiple packages provide same capability
	    };

	    struct DependencyIssue {
	        IssueType type;
	        PackageId affectedPackage;
	        std::string description;
	        std::vector<PackageId> involvedPackages;

	        // Enhanced constraint details
	        struct ConstraintDetail {
	            std::string constraintDescription;  // e.g., ">=2.0.0 and <3.0.0"
	            Version actualVersion;               // What version is actually available
	            bool isSatisfied;
	        };
	        std::vector<ConstraintDetail> failedConstraints;

	        std::optional<std::vector<std::string>> suggestedFixes;
	        bool isBlocker;  // Whether this prevents package loading

	        // Generate detailed description with constraint info
	        std::string GetDetailedDescription() const {
	            std::stringstream ss;
	            ss << description;

	            if (!failedConstraints.empty()) {
	                ss << "\n  Constraint details:";
	                for (const auto& constraint : failedConstraints) {
	                    ss << std::format("\n    Required: {} | Available: {} | {}",
	                                     constraint.constraintDescription,
	                                     constraint.actualVersion,
	                                     constraint.isSatisfied ? "✓" : "✗");
	                }
	            }

	            if (suggestedFixes && !suggestedFixes->empty()) {
	                ss << "\n  Suggestions:";
	                for (const auto& fix : *suggestedFixes) {
	                    ss << "\n    - " << fix;
	                }
	            }

	            return ss.str();
	        }
	    };

	    struct VersionConflict {
	        PackageId dependency;
	        struct Requirement {
	            PackageId requester;
	            //std::string requiredVersion;
	            //std::vector<Constraint> constraints;
	            std::string formattedConstraints;  // Human-readable constraint description
	        };
	        std::vector<Requirement> requirements;
	        std::optional<Version> availableVersion;
	        std::string conflictReason;

	        std::string GetDetailedDescription() const {
	            std::string desc = std::format("Version conflict for '{}':\n", dependency);
	            for (const auto& req : requirements) {
	                desc += std::format("  - '{}' requires: {}\n",
	                                   req.requester, req.formattedConstraints);
	            }
	            if (availableVersion) {
	                desc += std::format("  Available version: {}\n", *availableVersion);
	            }
	            return desc;
	        }
	    };

	    struct CircularDependency {
	        std::vector<PackageId> cycle;  // Ordered list forming the cycle
	        std::string GetCycleDescription() const {
	            if (cycle.empty()) return "";
	            std::string desc = cycle[0];
	            for (size_t i = 1; i < cycle.size(); ++i) {
	                desc += " → " + cycle[i];
	            }
	            desc += " → " + cycle[0];  // Complete the cycle
	            return desc;
	        }
	    };

	    struct PackageResolution {
	        PackageId id;

	        // Successfully resolved
	        std::vector<PackageId> resolvedDependencies;
	        std::vector<PackageId> optionalDependencies;  // Available optional deps

	        // Issues
	        std::vector<DependencyIssue> issues;

	        // Detailed breakdown
	        struct MissingDependency {
	            PackageId name;
	            //std::optional<Version> requiredVersion;
	            //std::vector<Constraint> requiredConstraints;
	            std::string formattedConstraints;
	            bool isOptional;
	            //std::vector<PackageId> searchPaths;  // Where we looked
	        };
	        std::vector<MissingDependency> missingDependencies;

	        // Transitive dependencies
	        std::unordered_map<PackageId, std::vector<PackageId>> transitiveDeps;

	        bool CanLoad() const {
	            return std::ranges::none_of(issues,
	                [](const auto& issue) { return issue.isBlocker; });
	        }

	        std::size_t BlockerCount() const {
	            return std::ranges::count_if(issues,
	                [](const auto& issue) { return issue.isBlocker; });
	        }
	    };

	    // Main report data
	    std::vector<PackageResolution> resolutions;
	    std::vector<VersionConflict> versionConflicts;
	    std::vector<CircularDependency> circularDependencies;

	    // Dependency graph
	    std::unordered_map<PackageId, std::vector<PackageId>> dependencyGraph;
	    std::unordered_map<PackageId, std::vector<PackageId>> reverseDependencyGraph;

	    // Load order (topologically sorted if possible)
	    std::vector<PackageId> loadOrder;
	    bool isLoadOrderValid;  // False if circular deps prevent valid ordering

	    // Statistics
	    struct Statistics {
	        std::size_t totalPackages;
	        std::size_t packagesWithIssues;
	        std::size_t missingDependencyCount;
	        std::size_t versionConflictCount;
	        std::size_t circularDependencyCount;
	        std::size_t maxDependencyDepth;
	        double averageDependencyCount;
	    } stats;

	    // Analysis methods
	    bool HasBlockingIssues() const {
	        return std::ranges::any_of(resolutions,
	            [](const auto& r) { return !r.CanLoad(); });
	    }


		std::size_t BlockerCount() const {
	    	return std::ranges::count_if(resolutions,
					[](const auto& r) { return r.BlockerCount() > 0; }));
		}


	    std::vector<PackageId> GetPackagesWithIssues() const {
	        std::vector<PackageId> issues;
	        for (const auto& r : resolutions) {
	            if (!r.issues.empty()) {
	                issues.push_back(r.id);
	            }
	        }
	        return issues;
	    }

	    std::vector<PackageId> GetLoadablePackages() const {
	        std::vector<PackageId> loadable;
	        for (const auto& r : resolutions) {
	            if (r.CanLoad()) {
	                loadable.push_back(r.id);
	            }
	        }
	        return loadable;
	    }

	    std::optional<CircularDependency> FindCycleInvolving(const PackageId& package) const {
	        for (const auto& cycle : circularDependencies) {
	            if (std::ranges::find(cycle.cycle, package) != cycle.cycle.end()) {
	                return cycle;
	            }
	        }
	        return std::nullopt;
	    }

	    std::vector<PackageId> GetDirectDependents(const PackageId& package) const {
	        auto it = reverseDependencyGraph.find(package);
	        if (it != reverseDependencyGraph.end()) {
	            return it->second;
	        }
	        return {};
	    }

	    // Generate human-readable report
	    std::string GenerateTextReport() const {
	        std::stringstream report;

	        report << "=== Dependency Resolution Report ===\n\n";

	        // Statistics
	        report << "Statistics:\n";
	        report << std::format("  Total packages: {}\n", stats.totalPackages);
	        report << std::format("  Packages with issues: {}\n", stats.packagesWithIssues);
	        report << std::format("  Missing dependencies: {}\n", stats.missingDependencyCount);
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
	};

	struct InitializationReport {
	    struct PackageInit {
	        PackageId id;
	        PackageState finalState;
	        std::size_t retryAttempts;
	        std::optional<EnhancedError> error;
	        std::chrono::milliseconds loadTime;
	    };

	    std::vector<PackageInit> moduleInits;
	    std::vector<PackageInit> pluginInits;
	    std::chrono::milliseconds totalTime;

	    std::size_t SuccessCount() const {
	        return std::ranges::count_if(moduleInits,
	            [](const auto& i) { return i.finalState == PackageState::Ready; }) +
	               std::ranges::count_if(pluginInits,
	            [](const auto& i) { return i.finalState == PackageState::Started; });
	    }

	    std::size_t FailureCount() const {
	        return std::ranges::count_if(moduleInits,
	            [](const auto& i) { return i.finalState == PackageState::Error; }) +
	               std::ranges::count_if(pluginInits,
	            [](const auto& i) { return i.finalState == PackageState::Error; });
	    }

	    std::size_t SkippedCount() const {
	        return std::ranges::count_if(moduleInits,
	            [](const auto& i) { return i.finalState == PackageState::Skipped; }) +
	               std::ranges::count_if(pluginInits,
	            [](const auto& i) { return i.finalState == PackageState::Skipped; });
	    }

	    std::size_t CascadeFailureCount() const {
	        // Skipped packages are cascade failures
	        return SkippedCount();
	    }

	    std::string GenerateTextReport() const {
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
	        auto totalRetries = std::accumulate(moduleInits.begin(), moduleInits.end(), 0,
	            [](int sum, const auto& init) { return sum + init.retryAttempts; }) +
	            std::accumulate(pluginInits.begin(), pluginInits.end(), 0,
	            [](int sum, const auto& init) { return sum + init.retryAttempts; });

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
	};

	// ============================================================================
	// Comprehensive Initialization State
	// ============================================================================

	struct InitializationState {
		ValidationReport validationReport;
		DependencyReport dependencyReport;
		InitializationReport initializationReport;

		std::chrono::system_clock::time_point startTime;
		std::chrono::system_clock::time_point endTime;
		std::chrono::milliseconds totalTime;

		bool Success() const {
			return initializationReport.FailureCount() == 0;
		}

		bool PartialSuccess() const {
			return initializationReport.FailureCount() > 0 &&
				   initializationReport.SuccessCount() > 0;
		}

		std::string GenerateFullReport() const {
			std::stringstream report;

			report << "=== COMPLETE INITIALIZATION REPORT ===\n\n";

			// Format timestamps (simplified - C++20 chrono formatting may need adaptation)
			auto startTimeT = std::chrono::system_clock::to_time_t(startTime);
			auto endTimeT = std::chrono::system_clock::to_time_t(endTime);

			report << std::format("Start time: {}\n", std::ctime(&startTimeT));
			report << std::format("End time: {}\n", std::ctime(&endTimeT));
			report << std::format("Total duration: {}ms\n\n", totalTime.count());

			report << validationReport.GenerateTextReport();
			report << "\n";
			report << dependencyReport.GenerateTextReport();
			report << "\n";
			report << initializationReport.GenerateTextReport();

			return report.str();
		}
	};

}