#pragma once

#include "plugify/core/conflict.hpp"

namespace plugify {
	struct Conflict::Impl {
		PackageId name;
		std::optional<Constraint> constraints;
		std::optional<std::string> reason;
	};
}