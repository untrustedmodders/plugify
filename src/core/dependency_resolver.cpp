#include "plugify/core/dependency_resolver.hpp"

#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"

using namespace plugify;

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
