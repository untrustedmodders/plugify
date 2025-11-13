#pragma once

#include "plugify/class.hpp"
#include "plugify/binding.hpp"

namespace plugify {
	struct Class::Impl {
		std::string name;
		std::optional<ValueType> handleType;
		//std::optional<PolicyType> nullPolicy;
		std::optional<std::string> invalidValue;
		std::optional<std::vector<std::string>> constructors;
     	std::optional<std::string> destructor;
		std::vector<Binding> bindings;
	};
}
