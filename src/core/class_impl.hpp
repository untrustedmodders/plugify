#pragma once

#include "plugify/class.hpp"
#include "plugify/binding.hpp"

namespace plugify {
	struct Class::Impl {
		std::string name;
		std::optional<ValueType> type;
		std::optional<std::string> invalid;
		std::optional<std::vector<std::string>> constructors;
     	std::optional<std::string> destructor;
		std::vector<Binding> bindings;
	};
}
