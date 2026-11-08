#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <span>

#include "plugify/extension.hpp"
#include "plg/format.hpp"

namespace plugify {
	/**
	 * @struct DependencyIssue
	 * @brief Represents a specific issue found during dependency resolution
	 */
	struct DependencyIssue {
		UniqueId affectedExtension;  ///< The extension the issue was found for.
		UniqueId involvedExtension;  ///< The other extension the issue is about, if there is one.
		std::string problem;  ///< Short classification of the issue.
		std::string description;  ///< What exactly is wrong.
		std::optional<std::vector<std::string>> suggestedFixes;  ///< What the user could do about it.
		bool isBlocking{ true }; ///< True if this issue prevents loading the extension

		/**
		 * @brief Render this issue as human-readable text
		 * @return The problem and its description, followed by the suggested fixes if there are any
		 */
		std::string GetDetailedDescription() const {
			std::string buffer;
			auto it = std::back_inserter(buffer);

			std::format_to(it, "{}: {}", problem, description);

			if (suggestedFixes && !suggestedFixes->empty()) {
				std::format_to(it, "\n  Suggestions:");
				for (const auto& fix : *suggestedFixes) {
					std::format_to(it, "\n    - {}", fix);
				}
			}

			return buffer;
		}
	};

	/**
	 * @struct ResolutionReport
	 * @brief Represents the result of a dependency resolution process
	 */
	struct ResolutionReport {
		// Main report data

		std::unordered_map<UniqueId, std::vector<DependencyIssue>> issues;  ///< Every issue found, per extension.

		// Dependency graph

		std::unordered_map<UniqueId, std::vector<UniqueId>> dependencyGraph;  ///< What each extension depends on. For quick dep checks.
		std::unordered_map<UniqueId, std::vector<UniqueId>> reverseDependencyGraph;  ///< What depends on each extension. For skipping dependents.

		// Load order

		std::vector<UniqueId> loadOrder;  ///< The order to load the extensions in.
		bool isLoadOrderValid{ false };  ///< False if circular deps prevent valid ordering.
	};

	/**
	 * @brief Interface for resolving dependencies in a Plugify environment
	 *
	 * This interface defines the contract for resolving dependencies, which includes
	 * gathering extension information, validating versions, and generating a dependency report.
	 */
	class IDependencyResolver {
	public:
		/**
		 * @brief Virtual destructor
		 */
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
		 * @param extensions Every extension that was discovered and parsed
		 * @return ResolutionReport containing the results of the resolution process
		 */
		virtual ResolutionReport Resolve(std::span<const Extension> extensions) = 0;
	};
}  // namespace plugify
