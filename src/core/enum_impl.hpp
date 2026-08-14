#pragma once

#include "plugify/enum.hpp"
#include "plugify/value.hpp"

namespace plugify {
	struct Enum::Impl {
		std::string name;
		std::vector<Value> values;
	};
}
