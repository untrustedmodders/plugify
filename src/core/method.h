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
		std::shared_ptr<Method> FindPrototype(std::string_view _name) const noexcept {
			for (const auto& param : paramTypes) {
				if (param.prototype) {
					if (param.prototype->name == _name) {
						return param.prototype;
					} else {
						auto prototype= param.prototype->FindPrototype(_name);
						if (prototype) {
							return prototype;
						}
					}
				}
			}

			if (retType.prototype) {
				if (retType.prototype->name == _name) {
					return retType.prototype;
				} else {
					auto prototype = retType.prototype->FindPrototype(_name);
					if (prototype) {
						return prototype;
					}
				}
			}

			return {};
		}

		mutable std::shared_ptr<std::vector<PropertyRef>> _paramTypes;
		friend class MethodRef;

	public:
		static inline const uint8_t kNoVarArgs = 0xFFU;

		[[nodiscard]] bool operator==(const Method& rhs) const noexcept { return name == rhs.name; }
	};
}
