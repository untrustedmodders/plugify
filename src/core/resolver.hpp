#pragma once

#include <plugify/resolver.hpp>

namespace plugify {
	/**
	 * @brief Default dependency resolver implementation
	 */
	class DependencyResolver : public IDependencyResolver {
	public:
		DependencyResolutionResult ResolveDependencies(
			const Package& package,
			std::span<const Package> availablePackages) override;

		std::unordered_map<PackageId, std::vector<PackageId>> BuildDependencyGraph(
			std::span<const Package> packages) override;

		bool HasCircularDependencies(std::span<const Package> packages) override;
	};

	/**
	 * @brief Default conflict resolver implementation
	 */
	class ConflictResolver : public IConflictResolver {
	public:
		std::vector<ConflictInfo> DetectConflicts(std::span<const Package> packages) override;

		Result<std::vector<Package>> ResolveConflicts(
			std::span<const ConflictInfo> conflicts,
			ConflictResolutionStrategy strategy) override;
	};

} // namespace plugify