#pragma once

#include "plugify/enum_value.hpp"

namespace plugify {
	struct EnumValue::Impl {
		std::string name;
		int64_t value{ 0 };
	};
}