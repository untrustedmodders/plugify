#pragma once

#include <vector>
#include <string>
#include <optional>

#include "version.hpp"

namespace plugify {
	/**
	 * @struct VersionConstraint
	 * @brief Represents a version constraint for a package.
	 *
	 * The VersionConstraint structure holds the type of constraint (e.g., equal, greater than, etc.)
	 * and the specific version it applies to.
	 */
	struct VersionConstraint {
		enum class Type {
			Equal, NotEqual, Greater, GreaterEqual, Less, LessEqual, Compatible, Any
		} type{};
		plg::version version;
	};

	/**
	 * @struct PackageConstraint
	 * @brief Represents constraints for a specific package.
	 *
	 * The PackageConstraint structure holds the package name, a vector of version constraints,
	 * and an optional flag indicating if the package is optional.
	 */
	struct PackageConstraint {
		std::string name; ///< The package name
		std::vector<VersionConstraint> constraints; ///< Constraints ANDed for the same package
		std::optional<bool> optional; ///< Optional package, if true, it can be omitted
	};
} // namespace plugify