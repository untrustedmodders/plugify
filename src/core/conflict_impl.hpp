#pragma once

#include "plugify/core/conflict.hpp"
#include "plugify/core/constraint.hpp"

namespace plugify {
	struct Conflict::Impl {
		PackageId name;
		std::optional<std::vector<Constraint>> constraints;
		std::optional<std::string> reason;
	};
}