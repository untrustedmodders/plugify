#pragma once

#include "plugify/enum_object.hpp"
#include "plugify/method.hpp"
#include "plugify/property.hpp"

namespace plugify {
	struct Property::Impl {
		ValueType type{};
		std::optional<bool> ref;
		std::shared_ptr<Method> prototype;
		std::shared_ptr<EnumObject> enumerate;
	};
}