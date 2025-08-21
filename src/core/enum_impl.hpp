#pragma once

#include "plugify/core/enum.hpp"
#include "plugify/core/enum_value.hpp"

namespace plugify {
	struct Enum::Impl {
		std::string name;
		std::vector<EnumValue> values;
	};
}