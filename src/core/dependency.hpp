#pragma once

#include <plugify/api/version.hpp>

namespace plugify {
	struct Dependency {
		std::string name;
		std::optional<std::vector<Constraint>> constraints;  // ANDed together
		std::optional<bool> optional;

		bool IsSatisfiedBy(const Version& version) const {
			if (!constraints)
				return false;

			for (const auto& constraint : *constraints) {
				if (!constraint.IsSatisfiedBy(version)) {
					return false;
				}
			}

			return true;
		}

		bool operator==(const Dependency& dependency) const noexcept = default;
	};

	struct Conflict {
		std::string name;
		std::optional<std::vector<Constraint>> constraints;  // Versions that conflict
		std::optional<std::string> reason;

		bool ConflictsWith(const Version& version) const {
			if (!constraints)
				return false;

			for (const auto& constraint : *constraints) {
				if (!constraint.IsSatisfiedBy(version)) {
					return false;
				}
			}

			return true;
		}

		bool operator==(const Conflict& conflict) const noexcept = default;
	};

	struct BasePaths {
		fs::path base;
		fs::path configs;
		fs::path data;
		fs::path logs;
	};
}
