#pragma once

#include <string>
#include <vector>
#include <optional>

#include "plugify/core/types.hpp"

namespace plugify {
	/**
	 * TODO
	 */
	enum class Comparison {
		Equal,        // Exactly this version
		NotEqual,     // Any version except this
		Greater,      // Strictly greater than
		GreaterEqual, // Greater than or equal to
		Less,         // Strictly less than
		LessEqual,    // Less than or equal to
		Compatible,   // Compatible with (e.g., ^2.1.0 = >=2.1.0 <3.0.0)
		Any           // Any version acceptable
	};

	/**
	 * @struct Constraint
	 * @brief Represents a version constraint for a plugin.
	 *
	 * The VersionConstraint structure holds the type of constraint (e.g., equal, greater than, etc.)
	 * and the specific version it applies to.
	 */
	struct Constraint {
		Comparison comparison{Comparison::Any};
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
			switch (comparison) {
				case Comparison::Any:
					return true;

				case Comparison::Equal:
					return other == version;

				case Comparison::NotEqual:
					return other != version;

				case Comparison::Greater:
					return other > version;

				case Comparison::GreaterEqual:
					return other >= version;

				case Comparison::Less:
					return other < version;

				case Comparison::LessEqual:
					return other <= version;

				case Comparison::Compatible: {
					// Semver caret (^) compatibility rules:
					// ^1.2.3 := >=1.2.3 <2.0.0 (compatible with 1.x.x)
					// ^0.2.3 := >=0.2.3 <0.3.0 (0.x.x is special - minor is breaking)
					// ^0.0.3 := >=0.0.3 <0.0.4 (0.0.x is special - patch is breaking)

					// Must be at least the specified version
					if (other < version) {
						return false;
					}

					// TODO: fix

					// Determine upper bound based on version numbers
					if (version.major > 0) {
						// Normal semver: compatible within same major version
						// ^2.1.0 means >=2.1.0 && <3.0.0
						return other.major == version.major;
					} else if (version.minor > 0) {
						// 0.x.y: minor version is breaking
						// ^0.2.3 means >=0.2.3 && <0.3.0
						return other.major == 0 && other.minor == version.minor;
					} else {
						// 0.0.x: patch version is breaking
						// ^0.0.3 means >=0.0.3 && <0.0.4
						return other.major == 0 && other.minor == 0 && other.patch == version.patch;
					}
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
	
	/**
	 * @brief Namespace containing utility functions of Comparison enum.
	 */
	struct ComparisonUtils {
		/**
		 * @brief Convert a Comparison enum value to its string representation.
		 * @param comparison The Comparison value to convert.
		 * @return The string representation of the Comparison.
		 */
		static constexpr std::string_view ToString(Comparison comparison) noexcept {
			switch (comparison) {
				case Comparison::Equal:        return "==";
				case Comparison::NotEqual:     return "!=";
				case Comparison::Greater:      return ">";
				case Comparison::GreaterEqual: return ">=";
				case Comparison::Less:         return "<";
				case Comparison::LessEqual:    return "<=";
				case Comparison::Compatible:   return "~>";
				case Comparison::Any:          return "";
			}
			return "Unknown";
		}

		/**
		 * @brief Convert a string representation to a Comparison enum value.
		 * @param comparison The string representation of Comparison.
		 * @return The corresponding Comparison enum value.
		 */
		static constexpr Comparison FromString(std::string_view comparison) noexcept {
			if (comparison == "==") {
				return Comparison::Equal;
			} else if (comparison == "!=") {
				return Comparison::NotEqual;
			} else if (comparison == ">") {
				return Comparison::Greater;
			} else if (comparison == ">=") {
				return Comparison::GreaterEqual;
			} else if (comparison == "<") {
				return Comparison::Less;
			} else if (comparison == "<=") {
				return Comparison::LessEqual;
			} else if (comparison == "~>") {
				return Comparison::Compatible;
			} else if (comparison.empty()) {
				return Comparison::Any;
			}
			return Comparison::Equal; // default fallback
		}
	};
} // namespace plugify
