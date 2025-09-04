#pragma once

#include "plugify/property.hpp"
#include "plugify/method.hpp"
#include "plugify/enum_object.hpp"

namespace plugify {
	struct Property::Impl {
		ValueType type{};
		std::optional<bool> ref;
		std::optional<Method> prototype;
		std::optional<EnumObject> enumerate;
	};
}