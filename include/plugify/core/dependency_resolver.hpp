#pragma once

#include <unordered_map>
#include <unordered_set>

#include "plugify/core/manifest.hpp"

namespace plugify {
    /**
     * @brief Represents a collection of packages with their manifests
     */
    using PackageCollection = std::unordered_map<UniqueId, ManifestPtr>;

    /**
     * @brief Represents the result of a dependency resolution process
     */
    struct DependencyResolution {
        /**
         * @brief Represents a specific issue found during dependency resolution
         */
        struct Issue {
            UniqueId affectedPackage{};
            std::string problem;
            std::string description;
            std::optional<UniqueId> involvedPackage;
            std::optional<std::vector<std::string>> suggestedFixes;
            bool isBlocking{true}; // True if this issue prevents loading the package

            // Generate detailed description with constraint info
            std::string GetDetailedDescription() const {
                std::stringstream ss;
                ss << problem << ": " << description;

                if (suggestedFixes && !suggestedFixes->empty()) {
                    ss << "\n  Suggestions:";
                    for (const auto& fix : *suggestedFixes) {
                        ss << "\n    - " << fix;
                    }
                }

                return ss.str();
            }
        };

        // Main report data
        std::unordered_map<UniqueId, std::vector<Issue>> issues;

        // Dependency graph
        std::unordered_map<UniqueId, std::vector<UniqueId>> dependencyGraph;  // For quick dep checks
        std::unordered_map<UniqueId, std::vector<UniqueId>> reverseDependencyGraph;  // For skipping dependents

        // Load order
        std::vector<UniqueId> loadOrder;
        bool isLoadOrderValid{false}; // False if circular deps prevent valid ordering
    };

    /**
     * @brief Interface for resolving dependencies in a Plugify environment
     *
     * This interface defines the contract for resolving dependencies, which includes
     * gathering package information, validating versions, and generating a dependency report.
     */
    class IDependencyResolver {
    public:
        virtual ~IDependencyResolver() = default;

        /**
         * @brief Resolve dependencies and generate a report
         *
         * This method performs the entire dependency resolution process, including:
         * - Gathering package information
         * - Validating versions and constraints
         * - Detecting conflicts
         * - Generating a load order
         *
         * @return DependencyResolution containing the results of the resolution process
         */
        virtual DependencyResolution Resolve(const PackageCollection& packages) = 0;
    };

    /**
     * @brief Libsolv-based implementation of the IDependencyResolver interface
     *
     * This class uses the libsolv library to perform dependency resolution,
     * leveraging its capabilities to handle complex dependency graphs, version
     * constraints, and conflict detection.
     */
    class LibsolvDependencyResolver : public IDependencyResolver {
    public:
        LibsolvDependencyResolver();
        ~LibsolvDependencyResolver() override;

        /**
         * Resolves dependencies for all packages, producing a report with resolutions,
         * conflicts, and load order.
         * @return DependencyReport containing the resolution results
         */
        DependencyResolution Resolve(const PackageCollection& packages) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> _impl;
    };
}  // namespace plugify