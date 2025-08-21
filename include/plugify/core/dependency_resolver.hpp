#pragma once

#include <unordered_map>
#include <unordered_set>

#include "plugify/core/report.hpp"

namespace plugify {
    /**
     * @brief Represents a collection of packages with their manifests
     */
    using PackageCollection = std::unordered_map<PackageId, Manifest>;

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
         * @return DependencyReport containing the results of the resolution process
         */
        virtual DependencyReport ResolveDependencies(const PackageCollection& packages) = 0;
    };

    /**
     * @brief Simple resolver interface for reoslv files and iterating directories
     */
    class DependencyResolver final : public IDependencyResolver {
    public:

        /**
         * Resolves dependencies for all packages, producing a report with resolutions,
         * conflicts, and load order.
         * @return DependencyReport containing the resolution results
         */
        DependencyReport ResolveDependencies(const PackageCollection& packages) override;

    private:

        // Main workflow steps
        void ProcessAllPackages(const PackageCollection& packages, DependencyReport& report);
        void
        DetectAllCircularDependencies(const PackageCollection& packages, DependencyReport& report);
        void CalculateTransitiveDependencies(DependencyReport& report);
        void ComputeLoadOrder(const PackageCollection& packages, DependencyReport& report);
        void CalculateStatistics(const PackageCollection& packages, DependencyReport& report);

        // Package processing helpers
        void ProcessPackageDependencies(
            const PackageId& id,
            const Manifest& manifest,
            const PackageCollection& packages,
            DependencyReport::PackageResolution& resolution,
            DependencyReport& report
        );
        void ProcessPackageConflicts(
            const PackageId& id,
            const Manifest& manifest,
            const PackageCollection& packages,
            DependencyReport::PackageResolution& resolution,
            DependencyReport& report
        );

        // Dependency handling
        void HandleMissingDependency(
            const PackageId& packageId,
            const Dependency& dep,
            DependencyReport::PackageResolution& resolution,
            DependencyReport& report
        );
        void HandleExistingDependency(
            const PackageId& packageId,
            const Dependency& dep,
            const Manifest& depManifest,
            DependencyReport::PackageResolution& resolution,
            DependencyReport& report
        );

        // Version conflict handling
        void RecordVersionConflict(
            const PackageId& packageId,
            const std::string& depName,
            const Version& availableVersion,
            const std::vector<Constraint>& constraints,
            DependencyReport& report
        );

        // Circular dependency detection
        bool DetectCyclesFromNode(
            const PackageId& node,
            std::unordered_set<PackageId>& visited,
            std::unordered_set<PackageId>& recursionStack,
            std::vector<PackageId>& currentPath,
            DependencyReport& report
        );
        void RecordCircularDependency(const std::vector<PackageId>& cycle, DependencyReport& report);

        // Topological sorting
        std::vector<PackageId>
        ComputeTopologicalOrder(const DependencyReport& report);

        // Utility functions
        static std::string FormatConstraints(std::span<const Constraint> constraints);
        static bool IsFailedConstraints(const std::vector<Constraint>& constraints, const Version& version);
        static std::unordered_map<PackageId, int>
        CalculatePackageDepth(const DependencyReport& report);
        static std::vector<std::string> CreateMissingDependencySuggestions(
            const std::string& depName,
            const std::string& constraints,
            bool isOptional
        );
        static std::vector<std::string> CreateVersionConflictSuggestions(
            const std::string& packageId,
            const std::string& depName,
            const std::vector<Constraint>& constraints
        );

        // Logging
        static void LogResults(const DependencyReport& report);
    };
}  // namespace plugify