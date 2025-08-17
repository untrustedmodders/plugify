#pragma once

#include <plugify/api/version.hpp>

namespace plugify {
	struct Conflict {
		std::string name;
		std::optional<std::vector<Constraint>> constraints;  // Versions that conflict
		std::string reason;

		std::optional<Constraint> ConflictsWith(const Version& version) const {
			if (!constraints)
				return {};

			for (const auto& constraint : *constraints) {
				if (!constraint.IsSatisfiedBy(version)) {
					return constraint;
				}
			}

			return {};
		}

		bool operator==(const Conflict& conflict) const noexcept = default;
	};
}
