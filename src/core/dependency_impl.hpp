#pragma once

#include "plugify/core/dependency.hpp"
#include "plugify/core/constraint.hpp"

namespace plugify {
	struct Dependency::Impl {
		PackageId name;
		std::optional<std::vector<Constraint>> constraints;
		std::optional<bool> optional;
	};
}