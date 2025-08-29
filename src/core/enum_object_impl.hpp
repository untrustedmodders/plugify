#pragma once

#include "plugify/core/enum_object.hpp"
#include "plugify/core/enum_value.hpp"

namespace plugify {
	struct EnumObject::Impl {
		std::string name;
		std::vector<EnumValue> values;
	};
}