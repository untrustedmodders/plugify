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
        DependencyReport ResolveDependencies(const PackageCollection& packages) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> _impl;
    };
}  // namespace plugify