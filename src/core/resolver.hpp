#pragma once

#include <plugify/resolver.hpp>

namespace plugify {
	/**
	 * @brief Default dependency resolver implementation
	 */
	class DefaultDependencyResolver : public IDependencyResolver {
	public:
		DefaultDependencyResolver() = default;
		~DefaultDependencyResolver() override = default;

		Result<std::vector<PackageInfo>> ResolveDependencies(
			std::string_view packageName,
			const plg::version& version,
			std::span<const LocalPackage> availableLocal,
			std::span<const RemotePackage> availableRemote) const override;

		std::vector<ConflictInfo> DetectConflicts(
			std::span<const PackageInfo> packages) const override;

	private:
		// Implementation would use graph algorithms for dependency resolution
		// Handle version constraint satisfaction
	};

	/**
	 * @brief Default conflict resolver implementation
	 */
	class DefaultConflictResolver : public IConflictResolver {
	public:
		DefaultConflictResolver() = default;
		~DefaultConflictResolver() override = default;

		Result<std::vector<std::string>> ResolveConflicts(
			std::span<const ConflictInfo> conflicts,
			std::span<const PackageInfo> availablePackages) const override;

	private:
		// Implementation would analyze conflicts and suggest resolutions
		// Could use heuristics like preferring newer versions, local packages, etc.
	};
} // namespace plugify