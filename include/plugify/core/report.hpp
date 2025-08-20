#pragma once

#include "error.hpp"
#include "package.hpp"
#include "types.hpp"
#include "dependency_resolver.hpp"

namespace plugify {
// TODO: Update ranges

	struct InitializationReport {
		struct PackageInit {
			PackageId id;
			PackageState finalState;
			std::size_t retryAttempts;
			std::optional<EnhancedError> error;
			DateTime loadTime;
		};

		std::vector<PackageInit> moduleInits;
		std::vector<PackageInit> pluginInits;
		DateTime totalTime;

		std::size_t successCount() const {
			return std::ranges::count_if(moduleInits,
				[](const auto& i) { return i.finalState == PackageState::Ready; }) +
				   std::ranges::count_if(pluginInits,
				[](const auto& i) { return i.finalState == PackageState::Started; });
		}

		std::size_t failureCount() const {
			return std::ranges::count_if(moduleInits,
				[](const auto& i) { return i.finalState == PackageState::Error; }) +
				   std::ranges::count_if(pluginInits,
				[](const auto& i) { return i.finalState == PackageState::Error; });
		}
	};

	struct ValidationReport {
		struct PackageValidation {
			PackageId id;
			bool passed;
			std::optional<EnhancedError> error;
			std::vector<std::string> warnings;
		};

		std::vector<PackageValidation> moduleResults;
		std::vector<PackageValidation> pluginResults;

		bool allPassed() const {
			return std::ranges::all_of(moduleResults, [](const auto& r) { return r.passed; }) &&
				   std::ranges::all_of(pluginResults, [](const auto& r) { return r.passed; });
		}

		std::size_t failureCount() const {
			return std::ranges::count_if(moduleResults, [](const auto& r) { return !r.passed; }) +
				   std::ranges::count_if(pluginResults, [](const auto& r) { return !r.passed; });
		}

		std::vector<PackageId> getFailedPackages() const {
			std::vector<PackageId> failed;
			for (const auto& r : moduleResults) {
				if (!r.passed) failed.push_back(r.id);
			}
			for (const auto& r : pluginResults) {
				if (!r.passed) failed.push_back(r.id);
			}
			return failed;
		}
	};

	// Enhanced DependencyReport moved to IDependencyResolver::Resolution
	using DependencyReport = IDependencyResolver::Resolution;

}