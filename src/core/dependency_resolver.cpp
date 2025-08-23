#include "plugify/core/dependency_resolver.hpp"

#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"

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

#include <memory>
}

using namespace plugify;

struct LibsolvDependencyResolver::Impl {
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

    DependencyReport ResolveDependencies(const PackageCollection& packages);

    std::unique_ptr<Pool, PoolDeleter> pool;
    Repo* repo = nullptr;
    std::unordered_map<PackageId, Id> packageToSolvableId;
    std::unordered_map<Id, PackageId> solvableIdToPackage;

    // Setup functions
    void InitializePool();
    void AddPackagesToPool(const PackageCollection& packages);
    Id AddSolvable(const Manifest& manifest);
    void SetupDependencies(Id solvableId, const Manifest& manifest);
    void SetupConflicts(Id solvableId, const Manifest& manifest);
    void SetupObsoletes(Id solvableId, const Manifest& manifest);

    // Constraint conversion
    Id
    MakeDepConstraint(const std::string& name, const std::optional<Constraint>& constraints);
    int ConvertComparison(plg::detail::range_operator op);

    // Resolution
    DependencyReport RunSolver();
    void ProcessSolverProblems(Solver* solver, DependencyReport& report);
    std::vector<std::string> ExtractSolutions(Solver* solver, Id problemId);
    void ComputeInstallationOrder(Transaction* trans, DependencyReport& report);
};

using namespace plugify;

// ============================================================================
// Main Resolution Function
// ============================================================================

DependencyReport
LibsolvDependencyResolver::Impl::ResolveDependencies(const PackageCollection& packages) {
    // Step 1: Initialize libsolv pool
    InitializePool();

    // Step 2: Add all packages to the pool
    AddPackagesToPool(packages);

    // Step 3: Run the solver
    return RunSolver();
}

// ============================================================================
// Pool Setup
// ============================================================================

static void debug_callback([[maybe_unused]] Pool* pool, [[maybe_unused]] void* data, int type, const char* str) {
    if (type & (SOLV_FATAL | SOLV_ERROR))
        PL_LOG_ERROR("libsolv: {}", str);
    else if (type & SOLV_WARN)
        PL_LOG_WARNING("libsolv: {}", str);
    else
        PL_LOG_VERBOSE("libsolv: {}", str);
}

void
LibsolvDependencyResolver::Impl::InitializePool() {
    pool.reset(pool_create());

    pool_setdebugcallback(pool.get(), debug_callback, this);

    constexpr int level = PLUGIFY_IS_DEBUG ? 3 : 1;
    pool_setdebuglevel(pool.get(), level);

    // Create a repository for our packages
    repo = repo_create(pool.get(), "installed");
}

void
LibsolvDependencyResolver::Impl::AddPackagesToPool(const PackageCollection& packages) {
    for (const auto& [id, manifest] : packages) {
        Id solvableId = AddSolvable(manifest);
        packageToSolvableId[id] = solvableId;
        solvableIdToPackage[solvableId] = id;

        // Setup dependencies and conflicts
        SetupDependencies(solvableId, manifest);
        SetupConflicts(solvableId, manifest);
        SetupObsoletes(solvableId, manifest);
    }

    // Create provides for all packages (self-provides)
    pool_createwhatprovides(pool.get());
}

Id
LibsolvDependencyResolver::Impl::AddSolvable(const Manifest& manifest) {
    Id solvableId = repo_add_solvable(repo);
    Solvable* s = pool_id2solvable(pool.get(), solvableId);

    // Set package name
    s->name = pool_str2id(pool.get(), manifest->name.c_str(), 1);

    // Set version
    std::string versionStr = manifest->version.to_string();
    s->evr = pool_str2id(pool.get(), versionStr.c_str(), 1);

    // Set vendor
    if (manifest->author) {
        s->vendor = pool_str2id(pool.get(), manifest->author->c_str(), 1);
    }

    // Add self-provides
    Id provideDep = pool_rel2id(pool.get(), s->name, s->evr, REL_EQ, 1);
    solvable_add_deparray(s, SOLVABLE_PROVIDES, provideDep, 0);

    return solvableId;
}

// ============================================================================
// Dependencies and Conflicts Setup
// ============================================================================

void
LibsolvDependencyResolver::Impl::SetupDependencies(Id solvableId, const Manifest& manifest) {
    if (!manifest->dependencies || manifest->dependencies->empty()) {
        return;
    }

    Solvable* s = pool_id2solvable(pool.get(), solvableId);

    for (const auto& dependency : *manifest->dependencies) {
        const auto& [depName, constraints, optional] = *dependency._impl;

        Id depId = MakeDepConstraint(depName, constraints);

        if (optional && optional.value()) {
            // Optional dependencies go to RECOMMENDS
            solvable_add_deparray(s, SOLVABLE_RECOMMENDS, depId, 0);
        } else {
            // Required dependencies go to REQUIRES
            solvable_add_deparray(s, SOLVABLE_REQUIRES, depId, 0);
        }
    }
}

void
LibsolvDependencyResolver::Impl::SetupConflicts(Id solvableId, const Manifest& manifest) {
    if (!manifest->conflicts || manifest->conflicts->empty()) {
        return;
    }

    Solvable* s = pool_id2solvable(pool.get(), solvableId);

    for (const auto& conflict : *manifest->conflicts) {
        const auto& [conflictName, constraints, reason] = *conflict._impl;

        Id conflictId = MakeDepConstraint(conflictName, constraints);

        // Add as CONFLICTS
        solvable_add_deparray(s, SOLVABLE_CONFLICTS, conflictId, 0);
    }
}

void
LibsolvDependencyResolver::Impl::SetupObsoletes(Id solvableId, const Manifest& manifest) {
    if (!manifest->obsoletes || manifest->obsoletes->empty()) {
        return;
    }

    Solvable* s = pool_id2solvable(pool.get(), solvableId);

    for (const auto& obsolete : *manifest->obsoletes) {
        const auto& [obsoleteName, constraints, reason] = *obsolete._impl;

        Id obsoleteId = MakeDepConstraint(obsoleteName, constraints);

        // Add as OBSOLETES
        solvable_add_deparray(s, SOLVABLE_OBSOLETES, obsoleteId, 0);
    }
}

Id
LibsolvDependencyResolver::Impl::MakeDepConstraint(
    const std::string& name,
    const std::optional<Constraint>& constraints
) {
    Id nameId = pool_str2id(pool.get(), name.c_str(), 1);

    if (!constraints || constraints->empty()) {
        return nameId;  // No version constraint
    }

    // Handle multiple constraints (OR logic)
    Id resultDep = 0;

    for (const auto& constraint : *constraints) {
        // Handle multiple constraints (AND logic)
        Id innerDep = 0;

        for (const auto& comparator : constraint) {
            std::string versionStr = comparator.get_version().to_string();
            Id evr = pool_str2id(pool.get(), versionStr.c_str(), 1);

            int flags = ConvertComparison(comparator.get_operator());
            Id dep = pool_rel2id(pool.get(), nameId, evr, flags, 1);

            if (innerDep == 0) {
                innerDep = dep;
            } else {
                // Combine with AND for multiple constraints
                innerDep = pool_rel2id(pool.get(), innerDep, dep, REL_AND, 1);
            }
        }

        if (resultDep == 0) {
            resultDep = innerDep;
        } else {
            // Combine with OR for multiple constraints
            resultDep = pool_rel2id(pool.get(), resultDep, innerDep, REL_OR, 1);
        }
    }

    return resultDep;
}

int
LibsolvDependencyResolver::Impl::ConvertComparison(plg::detail::range_operator op) {
    switch (op) {
        case plg::detail::range_operator::less:
            return REL_LT;
        case plg::detail::range_operator::less_or_equal:
            return REL_LT | REL_EQ;
        case plg::detail::range_operator::equal:
            return REL_EQ;
        case plg::detail::range_operator::greater_or_equal:
            return REL_GT | REL_EQ;
        case plg::detail::range_operator::greater:
            return REL_GT;
        default:
            return 0;  // No version requirement
    }
}

// ============================================================================
// Solver Execution
// ============================================================================

DependencyReport
LibsolvDependencyResolver::Impl::RunSolver() {
    DependencyReport report;
    ReportTimer timer(report);

    // Create solver
    std::unique_ptr<Solver, SolverDeleter> solver(solver_create(pool.get()));

    // Set solver flags
    solver_set_flag(solver.get(), SOLVER_FLAG_ALLOW_DOWNGRADE, 1);
    solver_set_flag(solver.get(), SOLVER_FLAG_SPLITPROVIDES, 1);

    // Create job queue - install all packages
    Queue queue;
    std::unique_ptr<Queue, QueueDeleter> jobs(&queue);
    queue_init(jobs.get());

    // Add all packages as install jobs
    for (const auto& [pkgId, solvId] : packageToSolvableId) {
        queue_push2(jobs.get(), SOLVER_SOLVABLE | SOLVER_INSTALL, solvId);
    }

    // Run the solver
    int ret = solver_solve(solver.get(), jobs.get());

    if (ret != 0) {
        // Problems detected
        ProcessSolverProblems(solver.get(), report);
    }

    // Success - extract the solution
    std::unique_ptr<Transaction, TransactionDeleter> trans(solver_create_transaction(solver.get()));

    if (trans) {
        ComputeInstallationOrder(trans.get(), report);
    }

    return report;
}

// ============================================================================
// Problem Processing
// ============================================================================

void
LibsolvDependencyResolver::Impl::ProcessSolverProblems(Solver* solver, DependencyReport& report) {
    Id problemCount = static_cast<Id>(solver_problem_count(solver));

    for (Id problemId = 1; problemId <= problemCount; problemId++) {
        Id probr = solver_findproblemrule(solver, problemId);

        Id source, target, dep;
        SolverRuleinfo type = solver_ruleinfo(solver, probr, &source, &target, &dep);

        // Get problem description
        const char* problemStr = solver_problemruleinfo2str(solver, type, probr, 0, 0);

        // Create issue for this problem
        DependencyReport::DependencyIssue issue;
        issue.description = problemStr ? problemStr : "Unknown dependency problem";
        issue.isBlocker = true;

        // Determine affected packages
        if (source) {
            auto it = solvableIdToPackage.find(source);
            if (it != solvableIdToPackage.end()) {
                issue.affectedPackage = it->second;
            }
        }

        if (target) {
            auto it = solvableIdToPackage.find(target);
            if (it != solvableIdToPackage.end()) {
                issue.involvedPackage = it->second;
            }
        }

        // Get all problem rules for more detailed analysis
        issue.description = solver_problemruleinfo2str(solver, type, source, target, dep);

        // Extract solutions for this problem
        issue.suggestedFixes = ExtractSolutions(solver, problemId);

        // Add issue to report
        if (!issue.affectedPackage.empty()) {
            auto resIt = std::ranges::find_if(
                report.resolutions,
                [&issue](const auto& r) { return r.id == issue.affectedPackage; }
            );

            if (resIt == report.resolutions.end()) {
                DependencyReport::PackageResolution resolution;
                resolution.id = issue.affectedPackage;
                resolution.issues.push_back(std::move(issue));
                report.resolutions.push_back(std::move(resolution));
            } else {
                resIt->issues.push_back(std::move(issue));
            }
        }
    }
}

std::vector<std::string>
LibsolvDependencyResolver::Impl::ExtractSolutions(Solver* solver, Id problemId) {
    std::vector<std::string> fixes;

    // Get solution count for this problem
    Id solutionCount = static_cast<Id>(solver_solution_count(solver, problemId));

    for (Id solutionId = 1; solutionId <= solutionCount; solutionId++) {
        std::vector<std::string> solutionSteps;

        // Iterate through solution elements
        Id element = 0;
        Id p, rp;

        while ((element = solver_next_solutionelement(solver, problemId, solutionId, element, &p, &rp)) != 0) {
            solutionSteps.emplace_back(solver_solutionelement2str(solver, p, rp));
        }

        // Combine solution steps
        if (!solutionSteps.empty()) {
            fixes.push_back(std::format("Solution {}: {}", solutionId, plg::join(solutionSteps, ", ")));
        } else {
            fixes.push_back(std::format("Solution {}: Adjust installation requirements", solutionId));
        }
    }

    // If no solutions found, add generic suggestions
    if (fixes.empty()) {
        fixes.emplace_back("Check package availability");
        fixes.emplace_back("Review version constraints");
        fixes.emplace_back("Consider removing conflicting packages");
    }

    // Add fixes to the last issue in report
    return fixes;
}

// ============================================================================
// Installation Order (Topological Sort)
// ============================================================================

void
LibsolvDependencyResolver::Impl::ComputeInstallationOrder(
    Transaction* trans,
    DependencyReport& report
) {
    // Get installation order from transaction
    transaction_order(trans, 0);

    // Get installed packages from transaction
    for (int i = 0; i < trans->steps.count; i++) {
        Id p = trans->steps.elements[i];

        // Check if this is an installation step
        Id type = transaction_type(trans, p, SOLVER_TRANSACTION_SHOW_ALL);

        if (type == SOLVER_TRANSACTION_INSTALL ||
            type == SOLVER_TRANSACTION_REINSTALL ||
            type == SOLVER_TRANSACTION_DOWNGRADE ||
            type == SOLVER_TRANSACTION_UPGRADE)
        {
            auto pkgIt = solvableIdToPackage.find(p);
            if (pkgIt != solvableIdToPackage.end()) {
                const PackageId& pkgId = pkgIt->second;
                report.loadOrder.push_back(pkgId);

                // Build dependency graph
                Solvable* s = pool_id2solvable(pool.get(), p);

                // Process requires
                if (s->dep_requires) {
                    Id req, *reqp;
                    reqp = s->repo->idarraydata + s->dep_requires;
                    while ((req = *reqp++) != 0) {
                        // Find what provides this requirement using pool_whatprovides
                        Id providers = pool_whatprovides(pool.get(), req);
                        if (providers) {
                            Id* pp;
                            for (pp = pool->whatprovidesdata + providers; *pp; pp++) {
                                Id providerId = *pp;
                                auto depIt = solvableIdToPackage.find(providerId);
                                if (depIt != solvableIdToPackage.end() && providerId != p) {
                                    const PackageId& depId = depIt->second;
                                    report.dependencyGraph[pkgId].push_back(depId);
                                    report.reverseDependencyGraph[depId].push_back(pkgId);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    report.isLoadOrderValid = true; // Since libsolv provides a valid transaction order
}

LibsolvDependencyResolver::LibsolvDependencyResolver()
    : _impl(std::make_unique<Impl>()) {

}

LibsolvDependencyResolver::~LibsolvDependencyResolver() = default;

DependencyReport
LibsolvDependencyResolver::ResolveDependencies(const PackageCollection& packages) {
    return _impl->ResolveDependencies(packages);
}
