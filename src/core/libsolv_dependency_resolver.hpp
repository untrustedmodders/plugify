#pragma once

#include "plugify/core/dependency_resolver.hpp"

namespace plugify {
    /**
     * @brief Libsolv-based implementation of the IDependencyResolver interface
     *
     * This class uses the libsolv library to perform dependency resolution,
     * leveraging its capabilities to handle complex dependency graphs, version
     * constraints, and conflict detection.
     */
    class LibsolvDependencyResolver : public IDependencyResolver {
        struct Impl;
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
        std::unique_ptr<Impl> _impl;
    };
} // namespace plugify