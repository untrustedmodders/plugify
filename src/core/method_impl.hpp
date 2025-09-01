#pragma once

#include "plugify/method.hpp"
#include "plugify/property.hpp"

namespace plugify {
	struct Method::Impl {
		std::vector<Property> paramTypes;
		Property retType;
		uint8_t varIndex{kNoVarArgs};
		std::string name;
		std::string funcName;
		std::string callConv;
	};
}