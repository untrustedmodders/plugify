#pragma once

#include "plugify/value.hpp"

namespace plugify {
	struct Value::Impl {
		std::string name;
		int64_t value{ 0 };
	};
}
