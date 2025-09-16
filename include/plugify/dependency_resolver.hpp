#pragma once

#include "plugify/extension.hpp"

namespace plugify {
	/**
	 * @brief Represents a specific issue found during dependency resolution
	 */
	struct DependencyIssue {
		UniqueId affectedExtension{ -1 };
		UniqueId involvedExtension{ -1 };
		std::string problem;
		std::string description;
		std::optional<std::vector<std::string>> suggestedFixes;
		bool isBlocking{ true };  // True if this issue prevents loading the extension

		// Generate detailed description with constraint info
		std::string GetDetailedDescription() const {
			std::string buffer;
			auto it = std::back_inserter(buffer);

			std::format_to(it, "{}: {}", problem, description);

			if (suggestedFixes && !suggestedFixes->empty()) {
				std::format_to(it, "\n  Suggestions:");
				for (const auto& fix : *suggestedFixes) {
					std::format_to(it, "\n    - ", fix);
				}
			}

			return buffer;
		}
	};

	/**
	 * @brief Represents the result of a dependency resolution process
	 */
	struct ResolutionReport {
		// Main report data
		std::flat_map<UniqueId, std::vector<DependencyIssue>> issues;

		// Dependency graph
		std::flat_map<UniqueId, std::vector<UniqueId>> dependencyGraph;  // For quick dep checks
		std::flat_map<UniqueId, std::vector<UniqueId>> reverseDependencyGraph;  // For skipping
		                                                                        // dependents

		// Load order
		std::vector<UniqueId> loadOrder;
		bool isLoadOrderValid{ false };  // False if circular deps prevent valid ordering
	};

	/**
	 * @brief Interface for resolving dependencies in a Plugify environment
	 *
	 * This interface defines the contract for resolving dependencies, which includes
	 * gathering extension information, validating versions, and generating a dependency report.
	 */
	class IDependencyResolver {
	public:
		virtual ~IDependencyResolver() = default;

		/**
		 * @brief Resolve dependencies and generate a report
		 *
		 * This method performs the entire dependency resolution process, including:
		 * - Gathering extension information
		 * - Validating versions and constraints
		 * - Detecting conflicts
		 * - Generating a load order
		 *
		 * @return ResolutionReport containing the results of the resolution process
		 */
		virtual ResolutionReport Resolve(std::span<const Extension> extensions) = 0;
	};
}  // namespace plugify