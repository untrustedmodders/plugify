#pragma once

#include <plugify/method.hpp>
#include <plugify/value_type.hpp>

namespace plugify {
	struct Method;

	struct EnumValue {
		std::string name;
		int64_t value;
	};

	struct Enum {
		std::string name;
		std::vector<EnumValue> values;

	private:
		mutable std::shared_ptr<std::vector<EnumValueHandle>> _values;
		friend class EnumHandle;
	};

	struct Property {
		ValueType type{};
		std::optional<bool> ref;
		std::shared_ptr<Method> prototype;
		std::shared_ptr<Enum> enumerate;
	};

	struct Method {
		std::vector<Property> paramTypes;
		Property retType;
		uint8_t varIndex{ kNoVarArgs };

		std::string name;
		std::string funcName;
		std::string callConv;

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

		mutable std::shared_ptr<std::vector<PropertyHandle>> _paramTypes;
		friend class MethodHandle;

	public:
		static inline const uint8_t kNoVarArgs = 0xFFU;

		bool operator==(const Method& rhs) const noexcept { return name == rhs.name; }
	};
}
