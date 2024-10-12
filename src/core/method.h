#pragma once

#include <plugify/method.h>
#include <plugify/value_type.h>

namespace plugify {
	struct Method;

	struct Property final {
		ValueType type{};
		bool ref{ false };
		std::shared_ptr<Method> prototype;
	};

	struct Method final {
		std::string name;
		std::string funcName;
		std::string callConv;
		std::vector<Property> paramTypes;
		Property retType;
		uint8_t varIndex{ kNoVarArgs };

	private:
		mutable std::shared_ptr<std::vector<PropertyRef>> _paramTypes;
		friend class MethodRef;

	public:
		static inline const uint8_t kNoVarArgs = 0xFFU;

		[[nodiscard]] bool operator==(const Method& rhs) const noexcept{ return name == rhs.name; }
	};
}
