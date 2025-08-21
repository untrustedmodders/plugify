#pragma once

#include "plugify/core/enum_value.hpp"

namespace plugify {
	struct EnumValue::Impl {
		std::string name;
		int64_t value{0};
	};
}