#pragma once

#include <plugify/api/method.hpp>
#include <plugify/api/value_type.hpp>

namespace plugify {
	struct Method;

	struct EnumValue {
		std::string name;
		int64_t value;
	};

	struct Enum {
		std::string name;
		std::vector<EnumValue> values;

		std::generator<std::string> Validate(size_t i) const {
			if (name.empty()) {
				co_yield std::format("Missing enum name at: {}", i);
			}

			if (!values.empty()) {
				for (const auto& value : values) {
					if (value.name.empty()) {
						co_yield std::format("Missing enum value name at: {}", name.empty() ? std::to_string(i) : name);
					}
				}
			}
		}
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

		std::shared_ptr<Method> FindPrototype(std::string_view _name) const noexcept {
			for (const auto& param : paramTypes) {
				if (param.prototype) {
					if (param.prototype->name == _name) {
						return param.prototype;
					}
					if (auto prototype = param.prototype->FindPrototype(_name)) {
						return prototype;
					}
				}
			}

			if (retType.prototype) {
				if (retType.prototype->name == _name) {
					return retType.prototype;
				}
				if (auto prototype = retType.prototype->FindPrototype(_name)) {
					return prototype;
				}
			}

			return {};
		}

		std::generator<std::string> Validate(size_t i, bool func = true) const {
			if (name.empty()) {
				co_yield std::format("Missing method name at: {}", i);
			}

			if (func && funcName.empty()) {
				co_yield std::format("Missing function name at: {}", name.empty() ? std::to_string(i) : name);
			}

			if (!callConv.empty()) {
#if PLUGIFY_ARCH_ARM
#if PLUGIFY_ARCH_BITS == 64

#elif PLUGIFY_ARCH_BITS == 32
				if (callConv != "soft" && callConv != "hard")
#endif // PLUGIFY_ARCH_BITS
#else
#if PLUGIFY_ARCH_BITS == 64 && PLUGIFY_PLATFORM_WINDOWS
				if (callConv != "vectorcall")
#elif PLUGIFY_ARCH_BITS == 32
				if (callConv != "cdecl" && callConv != "stdcall" && callConv != "fastcall" && callConv != "thiscall" && callConv != "vectorcall")
#endif // PLUGIFY_ARCH_BITS
#endif // PLUGIFY_ARCH_ARM
				{
					co_yield std::format("Invalid calling convention: '{}' at: {}", callConv, name.empty() ? std::to_string(i) : name);
				}
			}

			for (const auto& property : paramTypes) {
				if (property.type == ValueType::Void) {
					co_yield std::format("Parameter cannot be void type at: {}", name.empty() ? std::to_string(i) : name);
				} else if (property.type == ValueType::Function && property.ref.value_or(false)) {
					co_yield std::format("Parameter with function type cannot be reference at: {}", name.empty() ? std::to_string(i) : name);
				}

				if (property.prototype) {
					for (auto&& error : property.prototype->Validate(i)) {
						co_yield std::move(error);
					}
				}

				if (property.enumerate) {
					for (auto&& error : property.enumerate->Validate(i)) {
						co_yield std::move(error);
					}
				}
			}

			if (retType.ref.value_or(false)) {
				co_yield "Return cannot be reference";
			}

			if (retType.prototype) {
				for (auto&& error : retType.prototype->Validate(i, false)) {
					co_yield std::move(error);
				}
			}

			if (retType.enumerate) {
				for (auto&& error : retType.enumerate->Validate(i)) {
					co_yield std::move(error);
				}
			}

			if (varIndex != kNoVarArgs && varIndex >= paramTypes.size()) {
				co_yield "Invalid variable argument index";
			}
		}

		static inline const uint8_t kNoVarArgs = 0xFFU;

		bool operator==(const Method& rhs) const noexcept { return name == rhs.name; }
	};
}
