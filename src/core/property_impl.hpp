#pragma once

#include "plugify/core/property.hpp"

namespace plugify {
	struct Property::Impl {
		ValueType type{};
		std::optional<bool> ref;
		std::shared_ptr<Method> prototype;
		std::shared_ptr<EnumObject> enumerate;
	};
}