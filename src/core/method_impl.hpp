#pragma once

#include "plugify/method.hpp"
#include "plugify/property.hpp"
#include "plugify/signarure.hpp"

namespace plugify {
	struct Method::Impl {
		std::inplace_vector<Property, 32> paramTypes;
		Property retType;
		std::string name;
		std::string funcName;
		std::optional<CallConv> callConv;
		std::optional<uint8_t> varIndex;
	};
}