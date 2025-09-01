#pragma once

#include "plugify/enum_object.hpp"
#include "plugify/enum_value.hpp"

namespace plugify {
	struct EnumObject::Impl {
		std::string name;
		std::vector<EnumValue> values;
	};
}