#pragma once

#include <plugify/api/version.hpp>

namespace plugify {
	struct Dependency {
		std::string name;
		std::optional<std::vector<Constraint>> constraints;  // ANDed together
		std::optional<bool> optional;

		std::vector<Constraint> GetFailedConstraints(const Version& version) const {
			if (!constraints) return {};
			std::vector<Constraint> failed;
			std::ranges::copy_if(*constraints, std::back_inserter(failed), [&](const Constraint& c) {
				return !c.IsSatisfiedBy(version);
			});
			return failed;
		}

		bool operator==(const Dependency& dependency) const noexcept = default;
	};
}
