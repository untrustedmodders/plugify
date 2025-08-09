#pragma once

#include <vector>
#include <string>
#include <optional>

#include <plg/version.hpp>

namespace plugify {
	/**
	 * @typedef Version
	 * @brief Represents a version of a package.
	 *
	 * This type is an alias for plg::version, which encapsulates the versioning
	 * information for packages in the Plugify ecosystem.
	 */
	using Version = plg::version;

	/**
	 * @struct Constraint
	 * @brief Represents a version constraint for a plugin.
	 *
	 * The VersionConstraint structure holds the type of constraint (e.g., equal, greater than, etc.)
	 * and the specific version it applies to.
	 */
	struct Constraint {
		enum class Type {
			Equal,        // Exactly this version
			NotEqual,     // Any version except this
			Greater,      // Strictly greater than
			GreaterEqual, // Greater than or equal to
			Less,         // Strictly less than
			LessEqual,    // Less than or equal to
			Compatible,   // Compatible with (e.g., ^2.1.0 = >=2.1.0 <3.0.0)
			Any           // Any version acceptable
		};

		Type type{Type::Any};
		Version version;

		/**
		 * @brief Check if a version satisfies this constraint
		 *
		 * @note For Compatible type:
		 *       - ^1.2.3 := >=1.2.3 <2.0.0 (compatible with 1.x.x)
		 *       - ^0.2.3 := >=0.2.3 <0.3.0 (0.x.x is special - minor is breaking)
		 *       - ^0.0.3 := >=0.0.3 <0.0.4 (0.0.x is special - patch is breaking)
		 */
		bool IsSatisfiedBy(const Version& other) const {
			switch (type) {
				case Type::Any:
					return true;

				case Type::Equal:
					return other == version;

				case Type::NotEqual:
					return other != version;

				case Type::Greater:
					return other > version;

				case Type::GreaterEqual:
					return other >= version;

				case Type::Less:
					return other < version;

				case Type::LessEqual:
					return other <= version;

				case Type::Compatible: {
					return version.major == other.major // different major-versions are always incompatible
					   && version.minor >= other.minor; // data minor-version is incompatible if greater than code minor-version
				}
			}
			return false;
		}

		/**
		 * @brief Equality operator.
		 * @param constraint The other Constraint object to compare with.
		 * @return True if both Constraint objects are equal, false otherwise.
		 */
		bool operator==(const Constraint& constraint) const noexcept = default;
	};
}