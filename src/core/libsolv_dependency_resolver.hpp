#pragma once

#include "plugify/core/logger.hpp"
#include "plugify/core/dependency_resolver.hpp"

extern "C" {
#include <solv/evr.h>
#include <solv/policy.h>
#include <solv/pool.h>
#include <solv/poolarch.h>
#include <solv/queue.h>
#include <solv/repo.h>
#include <solv/repo_solv.h>
#include <solv/selection.h>
#include <solv/solvable.h>
#include <solv/solver.h>
#include <solv/solverdebug.h>
#include <solv/transaction.h>
}

namespace plugify {
    /**
     * @brief Libsolv-based implementation of the IDependencyResolver interface
     *
     * This class uses the libsolv library to perform dependency resolution,
     * leveraging its capabilities to handle complex dependency graphs, version
     * constraints, and conflict detection.
     */
    class LibsolvDependencyResolver : public IDependencyResolver {
        struct PoolDeleter {
            void operator()(Pool* p) const {
                if (p) {
                    pool_free(p);
                }
            }
        };

        struct SolverDeleter {
            void operator()(Solver* s) const {
                if (s) {
                    solver_free(s);
                }
            }
        };

        struct TransactionDeleter {
            void operator()(Transaction* t) const {
                if (t) {
                    transaction_free(t);
                }
            }
        };

        struct QueueDeleter {
            void operator()(Queue* q) const {
                if (q) {
                    queue_free(q);
                }
            }
        };

    public:
        LibsolvDependencyResolver(std::shared_ptr<ILogger> logger);
        ~LibsolvDependencyResolver() override = default;

        /**
         * Resolves dependencies for all extensions, producing a report with resolutions,
         * conflicts, and load order.
         * @return DependencyReport containing the resolution results
         */
        DependencyResolution Resolve(const ExtensionCollection& extensions) override;

    private:
        // Setup functions
        void InitializePool();
        void AddExtensionsToPool(const ExtensionCollection& extensions);
        Id AddSolvable(const Manifest& manifest);
        void SetupDependencies(Id solvableId, const Manifest& manifest);
        void SetupConflicts(Id solvableId, const Manifest& manifest);
        void SetupObsoletes(Id solvableId, const Manifest& manifest);

        // Constraint conversion
        Id
        MakeDepConstraint(const std::string& name, const std::optional<Constraint>& constraints);
        int ConvertComparison(plg::detail::range_operator op);

        // Resolution
        DependencyResolution RunSolver();
        void ProcessSolverProblems(Solver* solver, DependencyResolution& resolution);
        std::vector<std::string> ExtractSolutions(Solver* solver, Id problemId);
        void ComputeInstallationOrder(Transaction* trans, DependencyResolution& resolution);

    private:
        std::shared_ptr<ILogger> _logger;
        std::unique_ptr<Pool, PoolDeleter> _pool;
        Repo* _repo = nullptr;
        plg::flat_map<UniqueId, Id> _extensionToSolvableId;
        plg::flat_map<Id, UniqueId> _solvableIdToExtension;
    };
} // namespace plugify