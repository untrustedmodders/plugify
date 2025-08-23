#pragma once

#include "plugify/core/error.hpp"
#include "plugify/core/package.hpp"
#include "plugify/core/types.hpp"
#include "plugify/core/date_time.hpp"

namespace plugify {
#if 0
	// ============================================================================
	// Rich Result Types for Better Error Reporting
	// ============================================================================

	struct ValidationReport {
	    struct PackageValidation {
	        PackageId id;
	        bool passed;
	        std::optional<Error> error;
	        std::vector<std::string> warnings;
	    };

	    std::vector<PackageValidation> results;

	    std::chrono::system_clock::time_point startTime;
	    std::chrono::system_clock::time_point endTime;
	    std::chrono::milliseconds totalTime;

	    bool AllPassed() const {
	        return std::ranges::all_of(results, [](const auto& r) { return r.passed; });
	    }

	    std::size_t FailureCount() const {
	        return std::ranges::count_if(results, [](const auto& r) { return !r.passed; });
	    }

	    std::size_t PassedCount() const {
	        return std::ranges::count_if(results, [](const auto& r) { return r.passed; });
	    }

	    std::vector<PackageId> GetFailedPackages() const {
	        std::vector<PackageId> failed;
	        for (const auto& r : results) {
	            if (!r.passed) failed.push_back(r.id);
	        }
	        return failed;
	    }

		PLUGIFY_API std::string GenerateTextReport() const;
		PLUGIFY_API std::string GenerateJsonReport() const;
	};

    struct DependencyReport {
        struct DependencyIssue {
            PackageId affectedPackage;
            std::string description;
            std::optional<PackageId> involvedPackage;
            std::optional<std::vector<std::string>> suggestedFixes;
            bool isBlocker;  // Whether this prevents package loading

            // Generate detailed description with constraint info
            std::string GetDetailedDescription() const {
                std::stringstream ss;
                ss << description;

                if (suggestedFixes && !suggestedFixes->empty()) {
                    ss << "\n  Suggestions:";
                    for (const auto& fix : *suggestedFixes) {
                        ss << "\n    - " << fix;
                    }
                }

                return ss.str();
            }
        };

        struct PackageResolution {
            PackageId id;
            std::vector<DependencyIssue> issues;

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

        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
	    std::chrono::milliseconds totalTime;

        // Dependency graph
        std::unordered_map<PackageId, std::vector<PackageId>> dependencyGraph;
        std::unordered_map<PackageId, std::vector<PackageId>> reverseDependencyGraph;

        // Load order
        std::vector<PackageId> loadOrder;
        bool isLoadOrderValid{false}; // False if circular deps prevent valid ordering

        // Analysis methods
        bool HasBlockingIssues() const {
            return std::ranges::any_of(resolutions, [](const auto& r) { return !r.CanLoad(); });
        }

        std::size_t BlockerCount() const {
            return std::ranges::count_if(resolutions, [](const auto& r) { return r.BlockerCount() > 0; });
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

        std::vector<PackageId> GetDirectDependents(const PackageId& package) const {
            auto it = reverseDependencyGraph.find(package);
            if (it != reverseDependencyGraph.end()) {
                return it->second;
            }
            return {};
        }

        PLUGIFY_API std::string GenerateTextReport() const;
        PLUGIFY_API std::string GenerateJsonReport() const;
    };

	struct InitializationReport {
	    struct PackageInit {
	        PackageId id;
	        PackageState finalState;
	        std::size_t retryAttempts;
	        std::optional<Error> error;
	        std::chrono::milliseconds loadTime;
	    };
	    std::vector<PackageInit> inits;

	    std::chrono::system_clock::time_point startTime;
	    std::chrono::system_clock::time_point endTime;
	    std::chrono::milliseconds totalTime;

	    std::size_t SuccessCount() const {
	        return std::ranges::count_if(inits,
	            [](const auto& i) { return i.finalState == PackageState::Ready; });
	    }

	    std::size_t FailureCount() const {
	        return std::ranges::count_if(inits,
	            [](const auto& i) { return i.finalState == PackageState::Error; });
	    }

	    std::size_t SkippedCount() const {
	        return std::ranges::count_if(inits,
	            [](const auto& i) { return i.finalState == PackageState::Skipped; });
	    }

		PLUGIFY_API std::string GenerateTextReport() const;
		PLUGIFY_API std::string GenerateJsonReport() const;
	};

	// ============================================================================
	// Comprehensive Initialization State
	// ============================================================================

	struct InitializationState {
		InitializationReport initializationReport;
		ValidationReport validationReport;
		DependencyReport dependencyReport;

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

		PLUGIFY_API std::string GenerateTextReport() const;
		PLUGIFY_API std::string GenerateJsonReport() const;
	};

    template<typename T>
    struct ReportTimer {
        T& report;

        ReportTimer(T& t) : report{t} {
            report.startTime = std::chrono::steady_clock::now();
        }

        ~ReportTimer() {
            report.stopTime = std::chrono::steady_clock::now();
            report.totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(report.startTime - report.stopTime);
        }
    };
#endif
}