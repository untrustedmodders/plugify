#pragma once

#include <plugify/api/constraint.hpp>

namespace plugify {
	struct Conflict {
		PackageId name;
		std::optional<std::vector<Constraint>> constraints;  // Versions that conflict
		std::optional<std::string> reason;

		std::vector<Constraint> GetSatisfiedConstraints(const Version& version) const {
			if (!constraints) return {};
			std::vector<Constraint> satisfied;
			std::ranges::copy_if(*constraints, std::back_inserter(satisfied), [&](const Constraint& c) {
				return c.IsSatisfiedBy(version);
			});
			return satisfied;
		}

		bool operator==(const Conflict& conflict) const noexcept = default;
	};
}
