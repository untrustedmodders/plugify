#pragma once

#include "plugify/core/dependency.hpp"

namespace plugify {
	struct Dependency::Impl {
		std::string name;
		std::optional<Constraint> constraints;
		std::optional<bool> optional;
	};
}