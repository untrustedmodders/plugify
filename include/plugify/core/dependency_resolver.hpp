#pragma once

#include "constraint.hpp"
#include "plugify/core/types.hpp"

namespace plugify {
	class IDependencyResolver {
	public:
	    virtual ~IDependencyResolver() = default;

	    // Enhanced resolution structure with all dependency analysis
	    struct Resolution {
	        // Issue types
	        enum class IssueType {
	            None,
	            MissingDependency,
	            VersionConflict,
	            CircularDependency,
	            OptionalMissing,
	            TransitiveMissing,
	            ConflictingProviders
	        };

	        struct DependencyIssue {
	            IssueType type;
	            PackageId affectedPackage;
	            std::string description;
	            std::vector<PackageId> involvedPackages;
	            std::optional<std::vector<std::string>> suggestedFixes;
	            bool isBlocker;
	        };

	        struct VersionConflict {
	            PackageId dependency;
	            struct Requirement {
	                PackageId requester;
	                std::string requiredVersion;
	                std::vector<Constraint> constraints;
	            };
	            std::vector<Requirement> requirements;
	            std::optional<Version> availableVersion;
	            std::string conflictReason;
	        };

	        struct CircularDependency {
	            std::vector<PackageId> cycle;
	            std::string getCycleDescription() const;
	        };

	        struct PackageResolution {
	            PackageId id;
	            std::vector<PackageId> resolvedDependencies;
	            std::vector<PackageId> optionalDependencies;
	            std::vector<DependencyIssue> issues;
	            std::unordered_map<PackageId, std::vector<PackageId>> transitiveDeps;
	            bool canLoad() const;
	            std::size_t blockerCount() const;
	        };

	        // Main resolution data
	        std::vector<PackageResolution> resolutions;
	        std::vector<VersionConflict> versionConflicts;
	        std::vector<CircularDependency> circularDependencies;

	        // Dependency graphs
	        std::unordered_map<PackageId, std::vector<PackageId>> dependencyGraph;
	        std::unordered_map<PackageId, std::vector<PackageId>> reverseDependencyGraph;

	        // Topologically sorted load order
	        std::vector<PackageId> loadOrder;
	        bool isLoadOrderValid;

	        // Helper methods
	        bool hasBlockingIssues() const;
	        std::vector<PackageId> getPackagesWithIssues() const;
	        std::vector<PackageId> getLoadablePackages() const;
	        std::string generateTextReport() const;
	    };

	    virtual Result<Resolution> resolve(
	        std::span<const PluginInfo> plugins,
	        std::span<const ModuleInfo> modules) = 0;
	};
}