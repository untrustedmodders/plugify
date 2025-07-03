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

	public:
		std::vector<std::string> Validate(size_t i) const {
			std::vector<std::string> errors;

			if (name.empty()) {
				errors.emplace_back(std::format("Missing enum name at: {}", i));
			}

			if (!values.empty()) {
				for (const auto& value : values) {
					if (value.name.empty()) {
						errors.emplace_back(std::format("Missing enum value name at: {}", name.empty() ? std::to_string(i) : name));
					}
				}
			}

			return errors;
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

	private:
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

		mutable std::shared_ptr<std::vector<PropertyHandle>> _paramTypes;
		friend class MethodHandle;

	public:
		std::vector<std::string> Validate(size_t i, bool func = true) const {
			std::vector<std::string> errors;

			if (name.empty()) {
				errors.emplace_back(std::format("Missing method name at: {}", i));
			}

			if (func && funcName.empty()) {
				errors.emplace_back(std::format("Missing function name at: {}", name.empty() ? std::to_string(i) : name));
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
					errors.emplace_back(std::format("Invalid calling convention: '{}' at: {}", callConv, name.empty() ? std::to_string(i) : name));
				}
			}

			for (const auto& property : paramTypes) {
				if (property.type == ValueType::Void) {
					errors.emplace_back(std::format("Parameter cannot be void type at: {}", name.empty() ? std::to_string(i) : name));
				} else if (property.type == ValueType::Function && property.ref.value_or(false)) {
					errors.emplace_back(std::format("Parameter with function type cannot be reference at: {}", name.empty() ? std::to_string(i) : name));
				}

				if (property.prototype) {
					auto paramErrors = property.prototype->Validate(i, false);
					errors.insert(errors.end(), paramErrors.begin(), paramErrors.end());
				}

				if (property.enumerate) {
					auto enumErrors = property.enumerate->Validate(i);
					errors.insert(errors.end(), enumErrors.begin(), enumErrors.end());
				}
			}

			if (retType.ref.value_or(false)) {
				errors.emplace_back("Return cannot be reference");
			}

			if (retType.prototype) {
				auto paramErrors = retType.prototype->Validate(i, false);
				errors.insert(errors.end(), paramErrors.begin(), paramErrors.end());
			}

			if (retType.enumerate) {
				auto enumErrors = retType.enumerate->Validate(i);
				errors.insert(errors.end(), enumErrors.begin(), enumErrors.end());
			}

			if (varIndex != kNoVarArgs && varIndex >= paramTypes.size()) {
				errors.emplace_back("Invalid variable argument index");
			}

			return errors;
		}

		static inline const uint8_t kNoVarArgs = 0xFFU;

		bool operator==(const Method& rhs) const noexcept { return name == rhs.name; }
	};
}
