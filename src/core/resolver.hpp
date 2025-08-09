#pragma once

#include <plugify/resolver.hpp>

namespace plugify {
	// Concrete dependency resolver
	class DependencyResolver : public IDependencyResolver {
	public:
	    Result<std::vector<PackageConstraint>> ResolveDependencies(
	        const Package& package,
	        std::span<const LocalPackage> installed,
	        std::span<const RemotePackage> available
	    ) override;

	    bool AreDependenciesSatisfied(
	        const std::vector<PackageConstraint>& dependencies,
	        std::span<const LocalPackage> installed
	    ) override;

	    Result<std::vector<std::string>> CalculateInstallOrder(
	        std::span<const std::string> packages,
	        std::span<const LocalPackage> installed,
	        std::span<const RemotePackage> available
	    ) override;

	private:
	    // Topological sort for dependency ordering
	    Result<std::vector<std::string>> TopologicalSort(
	        const std::unordered_map<std::string, std::vector<std::string>>& graph
	    );

	    bool SatisfiesConstraint(const plg::version& version, const VersionConstraint& constraint);
	};

	// Concrete conflict resolver
	class ConflictResolver : public IConflictResolver {
	public:
	    std::vector<ConflictInfo> DetectConflicts(
	        std::span<const LocalPackage> packages
	    ) override;

	    Result<std::vector<LocalPackage>> ResolveConflicts(
	        std::span<const ConflictInfo> conflicts,
	        ConflictResolutionStrategy strategy
	    ) override;

	    bool WouldConflict(
	        const Package& package,
	        const PackageVersion& version,
	        std::span<const LocalPackage> installed
	    ) override;

	private:
	    bool CheckVersionConflict(const LocalPackage& pkg1, const LocalPackage& pkg2);
	    bool CheckExplicitConflict(const LocalPackage& pkg, std::span<const LocalPackage> others);
	};
} // namespace plugify