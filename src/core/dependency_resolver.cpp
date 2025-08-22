#include "plugify/core/dependency_resolver.hpp"

#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"

extern "C" {
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solver.h>
#include <solv/solverdebug.h>
#include <solv/selection.h>
#include <solv/evr.h>
#include <solv/policy.h>
#include <solv/poolarch.h>
#include <solv/repo_solv.h>
#include <solv/solvable.h>
#include <solv/transaction.h>
#include <solv/queue.h>
}

using namespace plugify;

// ============================================================================
// Main Resolution Function
// ============================================================================

DependencyReport
DependencyResolver::ResolveDependencies(const PackageCollection& packages) {
    DependencyReport report;
    report.stats.totalPackages = packages.size();

    // Step 1: Initialize libsolv pool
    InitializePool();

    // Step 2: Add all packages to the pool
    AddPackagesToPool(packages);

    // Step 3: Run the solver
    RunSolver(report);

    // Step 4: Collect statistics
    CollectStatistics(packages, report);

    return report;
}
// ============================================================================
// Pool Setup
// ============================================================================

void 
DependencyResolver::InitializePool() {
    pool.reset(pool_create());
    
    // Set architecture (optional, use "noarch" for architecture-independent)
    pool_setarch(pool.get(), "noarch");
    
    // Create a repository for our packages
    repo = repo_create(pool.get(), "packages");
    
    // Enable complex dependencies if needed
    pool_setdisttype(pool.get(), DISTTYPE_RPM);
}

void 
DependencyResolver::AddPackagesToPool(const PackageCollection& packages) {
    for (const auto& [id, manifest] : packages) {
        Id solvableId = AddSolvable(id, manifest);
        packageToSolvableId[id] = solvableId;
        solvableIdToPackage[solvableId] = id;
        
        // Setup dependencies and conflicts
        SetupDependencies(solvableId, manifest);
        SetupConflicts(solvableId, manifest);
    }
    
    // Create provides for all packages (self-provides)
    pool_createwhatprovides(pool.get());
}

Id 
DependencyResolver::AddSolvable(const PackageId& id, const Manifest& manifest) {
    Id solvableId = repo_add_solvable(repo);
    Solvable* s = pool_id2solvable(pool.get(), solvableId);
    
    // Set package name
    s->name = pool_str2id(pool.get(), id.c_str(), 1);
    
    // Set version
    std::string versionStr = manifest->version.to_string();
    s->evr = pool_str2id(pool.get(), versionStr.c_str(), 1);
    
    // Set architecture (using noarch for simplicity)
    s->arch = pool_str2id(pool.get(), "noarch", 1);
    
    // Add self-provides
    Id provideDep = pool_rel2id(pool.get(), s->name, s->evr, REL_EQ, 1);
    solvable_add_deparray(s, SOLVABLE_PROVIDES, provideDep, 0);
    
    return solvableId;
}

// ============================================================================
// Dependencies and Conflicts Setup
// ============================================================================

void 
DependencyResolver::SetupDependencies(Id solvableId, const Manifest& manifest) {
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
DependencyResolver::SetupConflicts(Id solvableId, const Manifest& manifest) {
    if (!manifest->conflicts || manifest->conflicts->empty()) {
        return;
    }
    
    Solvable* s = pool_id2solvable(pool.get(), solvableId);
    
    for (const auto& conflict : *manifest->conflicts) {
        const auto& [conflictName, constraints, reason] = *conflict._impl;
        
        Id conflictId = MakeDepConstraint(conflictName, constraints);
        
        // Add as CONFLICTS or OBSOLETES
        solvable_add_deparray(s, SOLVABLE_CONFLICTS, conflictId, 0);
    }
}

Id 
DependencyResolver::MakeDepConstraint(const std::string& name, 
                                            const std::optional<std::vector<Constraint>>& constraints) {
    Id nameId = pool_str2id(pool.get(), name.c_str(), 1);
    
    if (!constraints || constraints->empty()) {
        return nameId; // No version constraint
    }
    
    // Handle multiple constraints (AND logic)
    Id resultDep = 0;
    for (const auto& constraint : *constraints) {
        std::string versionStr = constraint.version.to_string();
        Id evr = pool_str2id(pool.get(), versionStr.c_str(), 1);
        
        int flags = ConvertComparison(constraint.comparison);
        Id dep = pool_rel2id(pool.get(), nameId, evr, flags, 1);
        
        if (resultDep == 0) {
            resultDep = dep;
        } else {
            // Combine with AND for multiple constraints
            resultDep = pool_rel2id(pool.get(), resultDep, dep, REL_AND, 1);
        }
    }
    
    return resultDep;
}

int 
DependencyResolver::ConvertComparison(Comparison comp) {
    switch (comp) {
        case Comparison::Less:
            return REL_LT;
        case Comparison::LessEqual:
            return REL_LT | REL_EQ;
        case Comparison::Equal:
            return REL_EQ;
        case Comparison::NotEqual:
            return REL_LT | REL_GT;
        case Comparison::GreaterEqual:
            return REL_GT | REL_EQ;
        case Comparison::Greater:
            return REL_GT;
        case Comparison::Compatible:
            // Compatible (~> in some package managers)
            // This needs special handling, using >= for simplicity
            return REL_GT | REL_EQ;
        case Comparison::Any:
        default:
            return 0; // No version requirement
    }
}

// ============================================================================
// Solver Execution
// ============================================================================

void 
DependencyResolver::RunSolver(DependencyReport& report) {
    // Create solver
    std::unique_ptr<Solver, SolverDeleter> solver(solver_create(pool.get()));
    
    // Set solver flags
    solver_set_flag(solver.get(), SOLVER_FLAG_ALLOW_DOWNGRADE, 1);
    solver_set_flag(solver.get(), SOLVER_FLAG_SPLITPROVIDES, 1);
    
    // Create job queue - install all packages
    Queue jobs;
    queue_init(&jobs);
    
    // Add all packages as install jobs
    for (const auto& [pkgId, solvId] : packageToSolvableId) {
        queue_push2(&jobs, SOLVER_SOLVABLE | SOLVER_INSTALL, solvId);
    }
    
    // Run the solver
    int ret = solver_solve(solver.get(), &jobs);
    
    if (ret != 0) {
        // Problems detected
        ProcessSolverProblems(solver.get(), report);
    } else {
        // Success - extract the solution
        std::unique_ptr<Transaction, TransactionDeleter> trans(
            solver_create_transaction(solver.get())
        );
        
        /*if (trans) {
            ComputeInstallationOrder(solver.get(), trans.get(), report);
        }*/
    }
    
    queue_free(&jobs);
}

// ============================================================================
// Problem Processing
// ============================================================================

void 
DependencyResolver::ProcessSolverProblems(Solver* solver, 
                                                DependencyReport& report) {
    int problemCount = solver_problem_count(solver);
    
    for (int problemId = 1; problemId <= problemCount; problemId++) {
        // Get problem description
        Id probr = solver_findproblemrule(solver, problemId);
        const char* problemStr = solver_problemruleinfo2str(solver, 
            solver_ruleinfo(solver, probr, 0, 0, 0), probr, 0, 0);
        
        // Create issue for this problem
        DependencyReport::DependencyIssue issue;
        issue.description = problemStr ? problemStr : "Unknown dependency problem";
        issue.isBlocker = true;
        
        // Analyze the problem rule
        Id source, target, dep;
        SolverRuleinfo type = solver_ruleinfo(solver, probr, &source, &target, &dep);
        issue.type = MapProblemType(type);
        
        // Determine affected packages
        if (source && solvableIdToPackage.contains(source)) {
            issue.affectedPackage = solvableIdToPackage[source];
        }
        
        if (target && solvableIdToPackage.contains(target)) {
            issue.involvedPackages.push_back(solvableIdToPackage[target]);
        }
        
        // Get all problem rules for more detailed analysis
        Queue problemRules;
        queue_init(&problemRules);
        solver_findallproblemrules(solver, problemId, &problemRules);
        
        for (int j = 0; j < problemRules.count; j++) {
            Id rule = problemRules.elements[j];
            AnalyzeProblemRule(solver, rule, issue);
        }
        
        queue_free(&problemRules);
        
        // Extract solutions for this problem
        ExtractSolutions(solver, problemId, report);
        
        // Add issue to report
        if (!issue.affectedPackage.empty()) {
            auto resIt = std::find_if(report.resolutions.begin(), 
                                      report.resolutions.end(),
                                      [&issue](const auto& r) {
                                          return r.id == issue.affectedPackage;
                                      });
            
            if (resIt == report.resolutions.end()) {
                DependencyReport::PackageResolution resolution;
                resolution.id = issue.affectedPackage;
                resolution.issues.push_back(issue);
                report.resolutions.push_back(resolution);
                report.stats.packagesWithIssues++;
            } else {
                resIt->issues.push_back(issue);
            }
        }
    }
}

void 
DependencyResolver::AnalyzeProblemRule(Solver* solver, Id rule,
                                             DependencyReport::DependencyIssue& issue) {
    Id source, target, dep;
    SolverRuleinfo type = solver_ruleinfo(solver, rule, &source, &target, &dep);
    
    switch (type) {
        case SOLVER_RULE_PKG_REQUIRES:
            issue.type = DependencyReport::IssueType::MissingDependency;
            if (dep) {
                issue.description = std::format("Missing dependency: {}",
                    pool_dep2str(pool.get(), dep));
            }
            break;
            
        case SOLVER_RULE_PKG_CONFLICTS:
        case SOLVER_RULE_PKG_OBSOLETES:
            issue.type = DependencyReport::IssueType::ConflictingProviders;
            if (source && target) {
                issue.description = std::format("Conflict between {} and {}",
                    GetSolvableName(source), GetSolvableName(target));
            }
            break;
            
        case SOLVER_RULE_PKG_SAME_NAME:
            issue.type = DependencyReport::IssueType::VersionConflict;
            issue.description = "Multiple versions of the same package";
            break;
            
        default:
            issue.type = DependencyReport::IssueType::Other;
            break;
    }
}

DependencyReport::IssueType 
DependencyResolver::MapProblemType(SolverRuleinfo type) {
    switch (type) {
        case SOLVER_RULE_PKG_REQUIRES:
            return DependencyReport::IssueType::MissingDependency;
        case SOLVER_RULE_PKG_CONFLICTS:
        case SOLVER_RULE_PKG_OBSOLETES:
            return DependencyReport::IssueType::ConflictingProviders;
        case SOLVER_RULE_PKG_SAME_NAME:
            return DependencyReport::IssueType::VersionConflict;
        default:
            return DependencyReport::IssueType::Other;
    }
}

void 
DependencyResolver::ExtractSolutions(Solver* solver, int problemId,
                                           DependencyReport& report) {
    std::vector<std::string> fixes;
    
    // Get solution count for this problem
    int solutionCount = solver_solution_count(solver, problemId);
    
    for (int solutionId = 1; solutionId <= solutionCount; solutionId++) {
        Queue solutionElements;
        queue_init(&solutionElements);
        
        // Get all elements of this solution
        solver_all_solutionelements(solver, problemId, solutionId, 0, &solutionElements);
        
        std::vector<std::string> solutionSteps;
        
        for (int j = 0; j < solutionElements.count; j += 2) {
            Id type = solutionElements.elements[j];
            Id what = solutionElements.elements[j + 1];
            
            std::string fix;
            
            // Handle based on solution type
            if (type == SOLVER_SOLUTION_ERASE) {
                fix = std::format("Do not install package '{}'", GetSolvableName(what));
            } else if (type == SOLVER_SOLUTION_REPLACE) {
                // For replace, 'what' contains packed info
                Id p = what & 0xffff;
                Id rp = what >> 16;
                if (rp) {
                    fix = std::format("Replace '{}' with '{}'", 
                        GetSolvableName(p), GetSolvableName(rp));
                } else {
                    fix = std::format("Replace package '{}'", GetSolvableName(p));
                }
            } /*else if (type == SOLVER_SOLUTION_JOB_REMOVE) {
                fix = "Remove job requirement";
            } else if (type == SOLVER_SOLUTION_JOB_ALLOW_INFERIOR) {
                fix = std::format("Allow inferior version of '{}'", GetSolvableName(what));
            } else if (type == SOLVER_SOLUTION_JOB_ALLOW_DOWNGRADE) {
                fix = std::format("Allow downgrade of '{}'", GetSolvableName(what));
            } else if (type == SOLVER_SOLUTION_JOB_ALLOW_ARCHCHANGE) {
                fix = std::format("Allow architecture change for '{}'", GetSolvableName(what));
            } else if (type == SOLVER_SOLUTION_JOB_ALLOW_VENDORCHANGE) {
                fix = std::format("Allow vendor change for '{}'", GetSolvableName(what));
            } else if (type == SOLVER_SOLUTION_JOB_ALLOW_NAMECHANGE) {
                fix = std::format("Allow name change for '{}'", GetSolvableName(what));
            } */else {
                // Generic solution
                //fix = std::format("Adjust constraints for '{}'", GetSolvableName(what));
            }
            
            if (!fix.empty()) {
                solutionSteps.push_back(fix);
            }
        }
        
        // Combine solution steps into a single suggestion
        if (!solutionSteps.empty()) {
            std::string combinedFix = std::format("Solution {}: ", solutionId);
            for (size_t i = 0; i < solutionSteps.size(); ++i) {
                if (i > 0) combinedFix += ", ";
                combinedFix += solutionSteps[i];
            }
            fixes.push_back(combinedFix);
        }
        
        queue_free(&solutionElements);
    }
    
    // If no solutions found, add generic suggestions
    if (fixes.empty()) {
        fixes.push_back("Check package availability");
        fixes.push_back("Review version constraints");
        fixes.push_back("Consider removing conflicting packages");
    }
    
    // Add fixes to the last issue in report
    if (!fixes.empty() && !report.resolutions.empty() && 
        !report.resolutions.back().issues.empty()) {
        report.resolutions.back().issues.back().suggestedFixes = fixes;
    }
}

// ============================================================================
// Installation Order (Topological Sort)
// ============================================================================

void 
DependencyResolver::ComputeInstallationOrder(Solver* ,
                                                   Transaction* ,
                                                   DependencyReport& ) {
    /*Queue order;
    queue_init(&order);
    
    // Get installation order from transaction
    transaction_order(trans, 0);
    transaction_installedresult(trans, &order);
    
    report.loadOrder.clear();
    report.loadOrder.reserve(order.count);
    
    for (int i = 0; i < order.count; i++) {
        Id solvableId = order.elements[i];
        
        if (solvableIdToPackage.contains(solvableId)) {
            report.loadOrder.push_back(solvableIdToPackage[solvableId]);
            
            // Build dependency graph
            Solvable* s = pool_id2solvable(pool.get(), solvableId);
            PackageId pkgId = solvableIdToPackage[solvableId];
            
            // Process requires
            if (s->requires) {
                Id* reqp = s->repo->idarraydata + s->requires;
                while (*reqp) {
                    // Find what provides this requirement
                    Queue providers;
                    queue_init(&providers);
                    pool_whatprovides(pool.get(), *reqp, &providers);
                    
                    for (int j = 0; j < providers.count; j++) {
                        Id providerId = providers.elements[j];
                        if (solvableIdToPackage.contains(providerId)) {
                            PackageId depId = solvableIdToPackage[providerId];
                            report.dependencyGraph[pkgId].push_back(depId);
                            report.reverseDependencyGraph[depId].push_back(pkgId);
                        }
                    }
                    
                    queue_free(&providers);
                    reqp++;
                }
            }
        }
    }
    
    queue_free(&order);
    report.isLoadOrderValid = !report.loadOrder.empty();*/
}

// ============================================================================
// Utilities
// ============================================================================

std::string 
DependencyResolver::GetSolvableName(Id solvableId) {
    Solvable* s = pool_id2solvable(pool.get(), solvableId);
    return s ? pool_id2str(pool.get(), s->name) : "unknown";
}

Version 
DependencyResolver::GetSolvableVersion(Id solvableId) {
    Solvable* s = pool_id2solvable(pool.get(), solvableId);
    if (s && s->evr) {
        std::string_view versionStr = pool_id2str(pool.get(), s->evr);
        // Parse version string back to Version object
        // This depends on your Version class implementation
        return Version(versionStr);
    }
    return Version{};
}

void 
DependencyResolver::CollectStatistics(const PackageCollection& packages,
                                            DependencyReport& report) {
    report.stats.totalPackages = packages.size();
    
    // Count various issue types
    for (const auto& resolution : report.resolutions) {
        for (const auto& issue : resolution.issues) {
            switch (issue.type) {
                case DependencyReport::IssueType::MissingDependency:
                    report.stats.missingDependencyCount++;
                    break;
                case DependencyReport::IssueType::VersionConflict:
                    report.stats.versionConflictCount++;
                    break;
                case DependencyReport::IssueType::ConflictingProviders:
                    report.stats.conflictCount++;
                    break;
                case DependencyReport::IssueType::CircularDependency:
                    report.stats.circularDependencyCount++;
                    break;
                default:
                    break;
            }
        }
        
        if (!resolution.missingDependencies.empty()) {
            report.stats.missingDependencyCount += resolution.missingDependencies.size();
        }
    }
    
    // Calculate average dependencies
    if (!packages.empty()) {
        double totalDeps = 0;
        for (const auto& [pkgId, deps] : report.dependencyGraph) {
            totalDeps += deps.size();
        }
        report.stats.averageDependencyCount = totalDeps / packages.size();
    }
}

#if 0
// ============================================================================
// Step 1: Package Collection
// ============================================================================

DependencyReport
DependencyResolver::ResolveDependencies(const PackageCollection& packages) {
    DependencyReport report;

    // Step 1: Collect all validated packages
    report.stats.totalPackages = packages.size();

    // Step 2: Process dependencies and conflicts for each package
    ProcessAllPackages(packages, report);

    // Step 3: Detect circular dependencies
    DetectAllCircularDependencies(packages, report);

    // Step 4: Calculate transitive dependencies
    CalculateTransitiveDependencies(report);

    // Step 5: Compute load order
    ComputeLoadOrder(packages, report);

    // Step 6: Calculate statistics
    CalculateStatistics(packages, report);

    // Step 7: Log results if needed
    LogResults(report);

    return report;
}

// ============================================================================
// Step 2: Package Processing
// ============================================================================

void
DependencyResolver::ProcessAllPackages(const PackageCollection& packages, DependencyReport& report) {
    for (const auto& [id, manifest] : packages) {
        DependencyReport::PackageResolution resolution{ .id = id };

        // Process dependencies
        ProcessPackageDependencies(id, manifest, packages, resolution, report);

        // Process conflicts
        ProcessPackageConflicts(id, manifest, packages, resolution, report);

        // Track packages with issues
        if (!resolution.issues.empty()) {
            report.stats.packagesWithIssues++;
        }

        report.resolutions.push_back(std::move(resolution));
    }
}

void
DependencyResolver::ProcessPackageDependencies(
    const PackageId& id,
    const Manifest& manifest,
    const PackageCollection& packages,
    DependencyReport::PackageResolution& resolution,
    DependencyReport& report
) {
    if (!manifest->dependencies || manifest->dependencies->empty()) {
        return;
    }

    for (const auto& dependency : *manifest->dependencies) {
        const auto& [depName, constraints, optional] = *dependency._impl;

        auto depIt = packages.find(depName);

        if (depIt == packages.end()) {
            HandleMissingDependency(id, dependency, resolution, report);
        } else {
            HandleExistingDependency(id, dependency, depIt->second, resolution, report);
        }
    }
}

void
DependencyResolver::ProcessPackageConflicts(
    const PackageId& id,
    const Manifest& manifest,
    const PackageCollection& packages,
    DependencyReport::PackageResolution& resolution,
    DependencyReport& report
) {
    if (!manifest->conflicts || manifest->conflicts->empty()) {
        return;
    }

    for (const auto& conflict : *manifest->conflicts) {
        const auto& [conflictName, constraints, reason] = *conflict._impl;

        auto conflictIt = packages.find(conflictName);
        if (conflictIt == packages.end()) {
            continue;  // No conflict if package doesn't exist
        }

        bool hasConflict = true;
        std::vector<DependencyReport::DependencyIssue::ConstraintDetail> conflictDetails;

        if (constraints) {
            // Check if conflict constraints are satisfied
            for (const auto& constraint : *constraints) {
                bool satisfied = constraint.IsSatisfiedBy(conflictIt->second->version);
                if (satisfied) {
                    conflictDetails.push_back(
                        {
                            .constraintDescription = FormatConstraints({ &constraint, 1 }),
                            .actualVersion = conflictIt->second->version,
                            .isSatisfied = satisfied,
                        }
                    );
                }
            }
            hasConflict = !conflictDetails.empty();
        }

        if (hasConflict) {
            std::string description = std::format("Package '{}' conflicts with '{}'", id, conflictName);
            if (reason) {
                description += std::format(" ({})", *reason);
            }

            if (!conflictDetails.empty()) {
                description += std::format(" - version {} matches conflict constraints",
                                                  conflictIt->second->version);
            }

            DependencyReport::DependencyIssue issue{
                .type = DependencyReport::IssueType::ConflictingProviders,
                .affectedPackage = id,
                .description = description,
                .involvedPackages = { conflictName },
                .failedConstraints = std::move(conflictDetails),
                .suggestedFixes = std::vector<
                    std::string>{ std::format("Remove either '{}' or '{}'", id, conflictName),
                                  std::format("Use alternative to '{}' or '{}'", id, conflictName) },
                .isBlocker = true
            };

            if (reason) {
                issue.suggestedFixes->push_back(std::format("Conflict reason: {}", *reason));
            }

            resolution.issues.push_back(std::move(issue));
            report.stats.conflictCount++;
        }
    }
}

// ============================================================================
// Dependency Handling Helpers
// ============================================================================

void
DependencyResolver::HandleMissingDependency(
    const PackageId& packageId,
    const Dependency& dep,
    DependencyReport::PackageResolution& resolution,
    DependencyReport& report
) {
    const auto& [depName, constraints, optional] = *dep._impl;
    bool isOptional = optional.value_or(false);

    // Create missing dependency record
    std::string formattedConstraints = constraints ? FormatConstraints(*constraints) : "any version";

    DependencyReport::PackageResolution::MissingDependency missing{
        .name = depName,
        .formattedConstraints = formattedConstraints,
        .isOptional = isOptional,
    };

    // Create issue
    DependencyReport::DependencyIssue issue{
        .type = isOptional ? DependencyReport::IssueType::OptionalMissing
                           : DependencyReport::IssueType::MissingDependency,
        .affectedPackage = packageId,
        .description = std::format(
            "Package '{}' requires '{}' ({}) which is not available",
            packageId,
            depName,
            formattedConstraints
        ),
        .involvedPackages = { depName },
        .failedConstraints = {},
        .suggestedFixes = CreateMissingDependencySuggestions(depName, formattedConstraints, isOptional),
        .isBlocker = !isOptional,
    };

    resolution.missingDependencies.push_back(std::move(missing));
    resolution.issues.push_back(std::move(issue));
    report.stats.missingDependencyCount++;
}

void
DependencyResolver::HandleExistingDependency(
    const PackageId& packageId,
    const Dependency& dep,
    const Manifest& depManifest,
    DependencyReport::PackageResolution& resolution,
    DependencyReport& report
) {
    const auto& [depName, constraints, optional] = *dep._impl;
    bool isOptional = optional.value_or(false);
    auto availableVersion = depManifest->version;

    if (!constraints) {
        resolution.resolvedDependencies.push_back(depName);
        report.dependencyGraph[packageId].push_back(depName);
        report.reverseDependencyGraph[depName].push_back(packageId);
        return;
    }

    auto failedConstraints = IsFailedConstraints(*constraints, availableVersion);
    if (failedConstraints) {
        // All constraints satisfied
        resolution.resolvedDependencies.push_back(depName);
        report.dependencyGraph[packageId].push_back(depName);
        report.reverseDependencyGraph[depName].push_back(packageId);
    } else {
        // Version conflict
        RecordVersionConflict(packageId, depName, availableVersion, *constraints, report);

        if (isOptional) {
            resolution.optionalDependencies.push_back(depName);
            report.stats.optionalDependencyCount++;
        } else {
            // Create detailed constraint failure information
            std::vector<DependencyReport::DependencyIssue::ConstraintDetail> details;
            for (const auto& constraint : *constraints) {
                bool satisfied = constraint.IsSatisfiedBy(availableVersion);
                if (!satisfied) {
                    details.push_back(
                        {
                            .constraintDescription = FormatConstraints({ &constraint, 1 }),
                            .actualVersion = availableVersion,
                            .isSatisfied = false,
                        }
                    );
                }
            }

            DependencyReport::DependencyIssue issue{
                .type = DependencyReport::IssueType::VersionConflict,
                .affectedPackage = packageId,
                .description = std::format(
                    "Version conflict: '{}' requires '{}' version {}, but {} is available",
                    packageId,
                    depName,
                    FormatConstraints(*constraints),
                    availableVersion
                ),
                .involvedPackages = { depName },
                .failedConstraints = std::move(details),
                .suggestedFixes = CreateVersionConflictSuggestions(packageId, depName, *constraints),
                .isBlocker = true
            };

            resolution.issues.push_back(std::move(issue));
            report.stats.versionConflictCount++;
        }
    }
}

void
DependencyResolver::RecordVersionConflict(
    const PackageId& packageId,
    const std::string& depName,
    const Version& availableVersion,
    const std::vector<Constraint>& constraints,
    DependencyReport& report
) {
    // Check if conflict already exists
    auto conflictIt = std::ranges::find_if(report.versionConflicts, [&depName](const auto& c) {
        return c.dependency == depName;
    });

    std::string formattedConstraints = FormatConstraints(constraints);

    if (conflictIt == report.versionConflicts.end()) {
        // Create new conflict
        DependencyReport::VersionConflict conflict{
            .dependency = depName,
            .availableVersion = availableVersion,
        };

        conflict.requirements.push_back(
            {
                .requester = packageId,
                .formattedConstraints = formattedConstraints,
            }
        );

        conflict.conflictReason = std::format(
            "Version {} does not satisfy constraints: {}",
            availableVersion,
            formattedConstraints
        );

        report.versionConflicts.push_back(std::move(conflict));
    } else {
        // Add to existing conflict
        conflictIt->requirements.push_back(
            {
                .requester = packageId,
                .formattedConstraints = formattedConstraints,
            }
        );
    }
}

// ============================================================================
// Step 3: Circular Dependency Detection
// ============================================================================

void
DependencyResolver::DetectAllCircularDependencies(
    const PackageCollection& packages,
    DependencyReport& report
) {
    std::unordered_set<PackageId> visited;
    std::unordered_set<PackageId> recursionStack;
    std::vector<PackageId> currentPath;

    for (const auto& [id, _] : packages) {
        if (!visited.contains(id)) {
            DetectCyclesFromNode(id, visited, recursionStack, currentPath, report);
        }
    }
}

bool
DependencyResolver::DetectCyclesFromNode(
    const PackageId& node,
    std::unordered_set<PackageId>& visited,
    std::unordered_set<PackageId>& recursionStack,
    std::vector<PackageId>& currentPath,
    DependencyReport& report
) {
    visited.insert(node);
    recursionStack.insert(node);
    currentPath.push_back(node);

    auto it = report.dependencyGraph.find(node);
    if (it != report.dependencyGraph.end()) {
        for (const auto& neighbor : it->second) {
            if (!visited.contains(neighbor)) {
                if (DetectCyclesFromNode(neighbor, visited, recursionStack, currentPath, report)) {
                    return true;
                }
            } else if (recursionStack.contains(neighbor)) {
                // Found a cycle
                auto cycleStart = std::ranges::find(currentPath, neighbor);
                if (cycleStart != currentPath.end()) {
                    std::vector<PackageId> cycle(cycleStart, currentPath.end());
                    RecordCircularDependency(cycle, report);
                }
                return true;
            }
        }
    }

    currentPath.pop_back();
    recursionStack.erase(node);
    return false;
}

void
DependencyResolver::RecordCircularDependency(const std::vector<PackageId>& cycle, DependencyReport& report) {
    // Check if this cycle already exists (in any rotation)
    for (const auto& existing : report.circularDependencies) {
        if (existing.cycle.size() == cycle.size()) {
            std::unordered_set<PackageId> existingSet(existing.cycle.begin(), existing.cycle.end());
            bool isSame = std::ranges::all_of(cycle, [&existingSet](const auto& pkg) {
                return existingSet.contains(pkg);
            });
            if (isSame) {
                return;  // Already recorded
            }
        }
    }

    // Record new cycle
    DependencyReport::CircularDependency circDep{ .cycle = cycle };

    // Add issues to affected packages
    for (const auto& pkgId : cycle) {
        auto resIt = std::ranges::find_if(report.resolutions, [&pkgId](const auto& r) {
            return r.id == pkgId;
        });

        if (resIt != report.resolutions.end()) {
            DependencyReport::DependencyIssue issue{
                .type = DependencyReport::IssueType::CircularDependency,
                .affectedPackage = pkgId,
                .description = std::format(
                    "Package '{}' is part of circular dependency: {}",
                    pkgId,
                    circDep.GetCycleDescription()
                ),
                .involvedPackages = cycle,
                .failedConstraints = {},
                .suggestedFixes = std::vector<
                    std::string>{ "Refactor to break the circular dependency",
                                  "Extract common functionality to a separate package",
                                  "Consider using dependency injection or interfaces" },
                .isBlocker = true
            };

            resIt->issues.push_back(std::move(issue));
        }
    }

    report.circularDependencies.push_back(std::move(circDep));
    report.stats.circularDependencyCount++;
}

// ============================================================================
// Step 4: Transitive Dependencies
// ============================================================================

void
DependencyResolver::CalculateTransitiveDependencies(DependencyReport& report) {
    for (auto& resolution : report.resolutions) {
        std::unordered_set<PackageId> transitive;
        std::queue<PackageId> toProcess;

        // Start with direct dependencies
        for (const auto& directDep : resolution.resolvedDependencies) {
            toProcess.push(directDep);
        }

        // BFS to find all transitive dependencies
        while (!toProcess.empty()) {
            auto current = toProcess.front();
            toProcess.pop();

            if (transitive.contains(current)) {
                continue;
            }
            transitive.insert(current);

            auto it = report.dependencyGraph.find(current);
            if (it != report.dependencyGraph.end()) {
                for (const auto& dep : it->second) {
                    if (!transitive.contains(dep)) {
                        toProcess.push(dep);
                        resolution.transitiveDeps[current].push_back(dep);
                    }
                }
            }
        }

        resolution.allTransitiveDependencies = { transitive.begin(), transitive.end() };
    }
}

// ============================================================================
// Step 5: Load Order Computation
// ============================================================================

void
DependencyResolver::ComputeLoadOrder(const PackageCollection& packages, DependencyReport& report) {
    report.loadOrder = ComputeTopologicalOrder(report);
    report.isLoadOrderValid = (report.loadOrder.size() == packages.size());
}

std::vector<PackageId>
DependencyResolver::ComputeTopologicalOrder(const DependencyReport& report) {
    std::vector<PackageId> loadOrder;
    std::unordered_map<PackageId, int> inDegree;

    // Initialize in-degrees
    for (const auto& resolution : report.resolutions) {
        auto it = report.dependencyGraph.find(resolution.id);
        inDegree[resolution.id] = (it == report.dependencyGraph.end()) ? 0
            : static_cast<int>(it->second.size());
    }

    // Kahn's algorithm
    std::queue<PackageId> queue;
    for (const auto& [id, degree] : inDegree) {
        if (degree == 0) {
            queue.push(id);
        }
    }

    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();
        loadOrder.push_back(current);

        auto it = report.reverseDependencyGraph.find(current);
        if (it != report.reverseDependencyGraph.end()) {
            for (const auto& dependent : it->second) {
                if (--inDegree[dependent] == 0) {
                    queue.push(dependent);
                }
            }
        }
    }

    /*
    for (const auto& cycle : report.circularDependencies) {
        inCycle.insert(cycle.cycle.begin(), cycle.cycle.end());
    }
    */

    return loadOrder;
}

// ============================================================================
// Step 6: Statistics Calculation
// ============================================================================

void
DependencyResolver::CalculateStatistics(const PackageCollection& packages, DependencyReport& report) {
    if (packages.empty()) {
        return;
    }

    double totalDeps = 0;
    int maxDepth = 0;

    std::unordered_map<PackageId, int> depths = CalculatePackageDepth(report);

    for (const auto& resolution : report.resolutions) {
        totalDeps += static_cast<double>(resolution.resolvedDependencies.size());

        if (depths.contains(resolution.id)) {
            maxDepth = std::max(maxDepth, depths[resolution.id]);
        }
    }

    report.stats.averageDependencyCount = totalDeps / static_cast<double>(packages.size());
    report.stats.maxDependencyDepth = maxDepth;
}

std::unordered_map<PackageId, int>
DependencyResolver::CalculatePackageDepth(
    const DependencyReport& report
) {
    // Topological sort with depth calculation
    std::unordered_map<PackageId, int> depths;
    std::unordered_map<PackageId, int> inDegree;

    // Initialize in-degrees and depths
    for (const auto& resolution : report.resolutions) {
        auto it = report.dependencyGraph.find(resolution.id);
        inDegree[resolution.id] = (it == report.dependencyGraph.end()) ? 0
            : static_cast<int>(it->second.size());
        depths[resolution.id] = 0;
    }

    // Process in topological order
    std::queue<PackageId> queue;
    for (const auto& [id, degree] : inDegree) {
        if (degree == 0) {
            queue.push(id);
        }
    }

    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        // Update depths of dependents
        auto it = report.dependencyGraph.find(current);
        if (it != report.dependencyGraph.end()) {
            for (const auto& dep : it->second) {
                depths[dep] = std::max(depths[dep], depths[current] + 1);

                if (--inDegree[dep] == 0) {
                    queue.push(dep);
                }
            }
        }
    }

    return depths;
}

// ============================================================================
// Utility Functions
// ============================================================================

std::string
DependencyResolver::FormatConstraints(std::span<const Constraint> constraints) {
    if (constraints.empty()) {
        return "any version";
    }

    std::string result;
    for (size_t i = 0; i < constraints.size(); ++i) {
        if (i > 0) {
            result += " AND ";
        }

        const auto& [comparison, version] = constraints[i];

        if (comparison == Comparison::Any) {
            result += "any version";
        } else if (comparison == Comparison::Compatible) {
            // Format compatible version range
            if (version.major > 0) {
                result += std::format("^{} (>={} and <{}.0.0)", version, version, version.major + 1);
            } else if (version.minor > 0) {
                result += std::format("^{} (>={} and <0.{}.0)", version, version, version.minor + 1);
            } else {
                result += std::format("^{} (>={} and <0.0.{})", version, version, version.patch + 1);
            }
        } else {
            result += std::format("{}{}", ComparisonUtils::ToString(comparison), version);
        }
    }

    return result;
}

bool
DependencyResolver::IsFailedConstraints(const std::vector<Constraint>& constraints, const Version& version) {
    for (const auto& constraint : constraints) {
        if (!constraint.IsSatisfiedBy(version)) {
           return true;
        }
    }

    return false;
}

std::vector<std::string>
DependencyResolver::CreateMissingDependencySuggestions(
    const std::string& depName,
    const std::string& constraints,
    bool isOptional
) {
    std::vector<std::string> suggestions;

    suggestions.push_back(std::format("Install package '{}' with version {}", depName, constraints));
    suggestions.push_back(std::format("Add '{}' to search paths", depName));

    if (isOptional) {
        suggestions.emplace_back("This is an optional dependency and can be skipped");
    }

    return suggestions;
}

std::vector<std::string>
DependencyResolver::CreateVersionConflictSuggestions(
    const std::string& packageId,
    const std::string& depName,
    const std::vector<Constraint>&
) {
    return {
        std::format("Update '{}' to a version that satisfies the constraints", depName),
        std::format("Relax version constraints in '{}' manifest", packageId),
        "Check if there's a compatible version available in other sources",
    };
}

// ============================================================================
// Step 7: Logging
// ============================================================================

void
DependencyResolver::LogResults(const DependencyReport& report) {
    if (report.HasBlockingIssues() || !report.circularDependencies.empty()) {
        std::cout << "\n" << report.GenerateTextReport() << "\n";
    } else {
        std::cout << std::format(
            "Dependencies resolved successfully. Load order determined for {} packages.\n",
            report.loadOrder.size()
        );
    }
}

#endif