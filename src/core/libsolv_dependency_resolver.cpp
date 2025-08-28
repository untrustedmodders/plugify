#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"
#include "core/libsolv_dependency_resolver.hpp"

using namespace plugify;

// ============================================================================
// Main Resolution Function
// ============================================================================

DependencyResolution
LibsolvDependencyResolver::Resolve(const PackageCollection& packages) {
    // Step 1: Initialize libsolv pool
    InitializePool();

    // Step 2: Add all packages to the pool
    AddPackagesToPool(packages);

    // Step 3: Run the solver
    return RunSolver();
}

void LibsolvDependencyResolver::SetLogger(std::shared_ptr<ILogger> logger) {
    _logger = std::move(logger);
}

// ============================================================================
// Pool Setup
// ============================================================================

static void debug_callback([[maybe_unused]] Pool* pool, void* data, int type, const char* str) {
    auto* logger = reinterpret_cast<ILogger*>(data);
    if (logger ==  nullptr)
        return;

    std::string title = std::format("libsolv: {}", str);
    if (type & SOLV_FATAL)
        logger->Log(title, Severity::Fatal);
    else if (type & SOLV_ERROR)
        logger->Log(title, Severity::Error);
    else if (type & SOLV_WARN)
        logger->Log(title, Severity::Warning);
    else
        logger->Log(title, Severity::Info);
}

void
LibsolvDependencyResolver::InitializePool() {
    _pool.reset(pool_create());

    pool_setdebugcallback(_pool.get(), debug_callback, _logger.get());

    constexpr int level = PLUGIFY_IS_DEBUG ? 3 : 1;
    pool_setdebuglevel(_pool.get(), level);

    // Create a repository for our packages
    _repo = repo_create(_pool.get(), "installed");
}

void
LibsolvDependencyResolver::AddPackagesToPool(const PackageCollection& packages) {
    _packageToSolvableId.reserve(packages.size());
    _solvableIdToPackage.reserve(packages.size());

    for (const auto& package : packages) {
        const auto& id = package.GetId();
        const auto& manifest = package.GetManifest();

        Id solvableId = AddSolvable(manifest);
        _packageToSolvableId[id] = solvableId;
        _solvableIdToPackage[solvableId] = id;

        // Setup dependencies and conflicts
        SetupDependencies(solvableId, manifest);
        SetupConflicts(solvableId, manifest);
        SetupObsoletes(solvableId, manifest);
    }

    // Create provides for all packages (self-provides)
    pool_createwhatprovides(_pool.get());
}

Id
LibsolvDependencyResolver::AddSolvable(const PackageManifest& manifest) {
    Id solvableId = repo_add_solvable(_repo);
    Solvable* s = pool_id2solvable(_pool.get(), solvableId);

    // Set package name
    s->name = pool_str2id(_pool.get(), manifest.name.c_str(), 1);

    // Set version
    std::string versionStr = manifest.version.to_string();
    s->evr = pool_str2id(_pool.get(), versionStr.c_str(), 1);

    // Set vendor
    if (manifest.author) {
        s->vendor = pool_str2id(_pool.get(), manifest.author->c_str(), 1);
    }

    // Add self-provides
    Id provideDep = pool_rel2id(_pool.get(), s->name, s->evr, REL_EQ, 1);
    solvable_add_deparray(s, SOLVABLE_PROVIDES, provideDep, 0);

    return solvableId;
}

// ============================================================================
// Dependencies and Conflicts Setup
// ============================================================================

void
LibsolvDependencyResolver::SetupDependencies(Id solvableId, const PackageManifest& manifest) {
    if (!manifest.dependencies || manifest.dependencies->empty()) {
        return;
    }

    Solvable* s = pool_id2solvable(_pool.get(), solvableId);

    for (const auto& dependency : *manifest.dependencies) {
        const auto& [depName, constraints, optional] = *dependency._impl;

        Id depId = MakeDepConstraint(depName, constraints);

        if (optional.value_or(false)) {
            // Optional dependencies go to RECOMMENDS
            solvable_add_deparray(s, SOLVABLE_RECOMMENDS, depId, 0);
        } else {
            // Required dependencies go to REQUIRES
            solvable_add_deparray(s, SOLVABLE_REQUIRES, depId, 0);
        }
    }
}

void
LibsolvDependencyResolver::SetupConflicts(Id solvableId, const PackageManifest& manifest) {
    if (!manifest.conflicts || manifest.conflicts->empty()) {
        return;
    }

    Solvable* s = pool_id2solvable(_pool.get(), solvableId);

    for (const auto& conflict : *manifest.conflicts) {
        const auto& [conflictName, constraints, reason] = *conflict._impl;

        Id conflictId = MakeDepConstraint(conflictName, constraints);

        // Add as CONFLICTS
        solvable_add_deparray(s, SOLVABLE_CONFLICTS, conflictId, 0);
    }
}

void
LibsolvDependencyResolver::SetupObsoletes(Id solvableId, const PackageManifest& manifest) {
    if (!manifest.obsoletes || manifest.obsoletes->empty()) {
        return;
    }

    Solvable* s = pool_id2solvable(_pool.get(), solvableId);

    for (const auto& obsolete : *manifest.obsoletes) {
        const auto& [obsoleteName, constraints, reason] = *obsolete._impl;

        Id obsoleteId = MakeDepConstraint(obsoleteName, constraints);

        // Add as OBSOLETES
        solvable_add_deparray(s, SOLVABLE_OBSOLETES, obsoleteId, 0);
    }
}

Id
LibsolvDependencyResolver::MakeDepConstraint(
    const std::string& name,
    const std::optional<Constraint>& constraints
) {
    Id nameId = pool_str2id(_pool.get(), name.c_str(), 1);

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
            Id evr = pool_str2id(_pool.get(), versionStr.c_str(), 1);

            int flags = ConvertComparison(comparator.get_operator());
            Id dep = pool_rel2id(_pool.get(), nameId, evr, flags, 1);

            if (innerDep == 0) {
                innerDep = dep;
            } else {
                // Combine with AND for multiple constraints
                innerDep = pool_rel2id(_pool.get(), innerDep, dep, REL_AND, 1);
            }
        }

        if (resultDep == 0) {
            resultDep = innerDep;
        } else {
            // Combine with OR for multiple constraints
            resultDep = pool_rel2id(_pool.get(), resultDep, innerDep, REL_OR, 1);
        }
    }

    return resultDep;
}

int
LibsolvDependencyResolver::ConvertComparison(plg::detail::range_operator op) {
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

DependencyResolution
LibsolvDependencyResolver::RunSolver() {
    DependencyResolution resolution;

    // Create solver
    std::unique_ptr<Solver, SolverDeleter> solver(solver_create(_pool.get()));

    // Set solver flags
    solver_set_flag(solver.get(), SOLVER_FLAG_ALLOW_DOWNGRADE, 1);
    solver_set_flag(solver.get(), SOLVER_FLAG_SPLITPROVIDES, 1);

    // Create job queue - install all packages
    Queue queue;
    std::unique_ptr<Queue, QueueDeleter> jobs(&queue);
    queue_init(jobs.get());

    // Add all packages as install jobs
    for (const auto& [pkgId, solvId] : _packageToSolvableId) {
        queue_push2(jobs.get(), SOLVER_SOLVABLE | SOLVER_INSTALL, solvId);
    }

    // Run the solver
    int ret = solver_solve(solver.get(), jobs.get());

    if (ret != 0) {
        // Problems detected
        ProcessSolverProblems(solver.get(), resolution);
    }

    // Success - extract the solution
    std::unique_ptr<Transaction, TransactionDeleter> trans(solver_create_transaction(solver.get()));

    if (trans) {
        ComputeInstallationOrder(trans.get(), resolution);
    }

    return resolution;
}

// ============================================================================
// Problem Processing
// ============================================================================

void
LibsolvDependencyResolver::ProcessSolverProblems(Solver* solver, DependencyResolution& resolution) {
    Id problemCount = static_cast<Id>(solver_problem_count(solver));

    for (Id problemId = 1; problemId <= problemCount; problemId++) {
        Id probr = solver_findproblemrule(solver, problemId);

        Id source, target, dep;
        SolverRuleinfo type = solver_ruleinfo(solver, probr, &source, &target, &dep);

        // Create issue for this problem
        DependencyResolution::Issue issue;
        issue.problem = std::format("Problem {}/{}", problemId, problemCount);
        issue.isBlocking = true; // All problems from solver are blocking

        // Determine affected packages
        if (source) {
            auto it = _solvableIdToPackage.find(source);
            if (it != _solvableIdToPackage.end()) {
                issue.affectedPackage = it->second;
            }
        }

        if (target) {
            auto it = _solvableIdToPackage.find(target);
            if (it != _solvableIdToPackage.end()) {
                issue.involvedPackage = it->second;
            }
        }

        // Get all problem rules for more detailed analysis
        issue.description = solver_problemruleinfo2str(solver, type, source, target, dep);

        // Extract solutions for this problem
        issue.suggestedFixes = ExtractSolutions(solver, problemId);

        // Add issue to report
        if (issue.affectedPackage != -1) {
            auto it = resolution.issues.find(issue.affectedPackage);
            if (it != resolution.issues.end()) {
                it->second.push_back(std::move(issue));
            } else {
                resolution.issues.emplace(issue.affectedPackage, std::vector{std::move(issue)});
            }
        }
    }
}

std::vector<std::string>
LibsolvDependencyResolver::ExtractSolutions(Solver* solver, Id problemId) {
    // Get solution count for this problem
    Id solutionCount = static_cast<Id>(solver_solution_count(solver, problemId));

    std::vector<std::string> fixes;
    fixes.reserve(static_cast<size_t>(solutionCount));

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
LibsolvDependencyResolver::ComputeInstallationOrder(
    Transaction* trans,
    DependencyResolution& resolution
) {
    // Get installation order from transaction
    transaction_order(trans, 0);

    size_t count = static_cast<size_t>(trans->steps.count);
    resolution.loadOrder.reserve(count);
    resolution.dependencyGraph.reserve(count);
    resolution.reverseDependencyGraph.reserve(count);

    // Get installed packages from transaction
    for (size_t i = 0; i < count; i++) {
        Id p = trans->steps.elements[i];

        // Check if this is an installation step
        /*Id type = transaction_type(trans, p, SOLVER_TRANSACTION_SHOW_ALL);
        std::cout << "Transaction step: " << i << " - Solvable ID: " << p << " - Type: " << type << std::endl;
        if (type == SOLVER_TRANSACTION_INSTALL ||
            type == SOLVER_TRANSACTION_REINSTALL ||
            type == SOLVER_TRANSACTION_DOWNGRADE ||
            type == SOLVER_TRANSACTION_UPGRADE)*/
        {
            auto pkgIt = _solvableIdToPackage.find(p);
            if (pkgIt != _solvableIdToPackage.end()) {
                const UniqueId& pkgId = pkgIt->second;
                resolution.loadOrder.push_back(pkgId);

                // Build dependency graph
                Solvable* s = pool_id2solvable(_pool.get(), p);

                // Process requires
                if (s->dep_requires) {
                    Id req, *reqp;
                    reqp = s->repo->idarraydata + s->dep_requires;
                    while ((req = *reqp++) != 0) {
                        // Find what provides this requirement using pool_whatprovides
                        Id providers = pool_whatprovides(_pool.get(), req);
                        if (providers) {
                            Id* pp;
                            for (pp = _pool->whatprovidesdata + providers; *pp; pp++) {
                                Id providerId = *pp;
                                auto depIt = _solvableIdToPackage.find(providerId);
                                if (depIt != _solvableIdToPackage.end() && providerId != p) {
                                    const UniqueId& depId = depIt->second;
                                    resolution.dependencyGraph[pkgId].push_back(depId);
                                    resolution.reverseDependencyGraph[depId].push_back(pkgId);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    resolution.isLoadOrderValid = true; // Since libsolv provides a valid transaction order
}
