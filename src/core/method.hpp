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

		void Validate(StackLogger& logger, const std::string& title) const {
			ScopeLogger{logger};

			if (name.empty()) {
				logger.Log(std::format("Missing enum name at: {}", title));
			}

			if (!values.empty()) {
				ScopeLogger{logger};
				for (const auto& value : values) {
					if (value.name.empty()) {
						logger.Log(std::format("Missing enum value name at: {}", name.empty() ? title : name));
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

		void Validate(StackLogger& logger, const std::string& title, bool func = true) const {
			ScopeLogger{logger};

			if (name.empty()) {
				logger.Log(std::format("Missing method name at: {}", title));
			}

			if (func && funcName.empty()) {
				logger.Log(std::format("Missing function name at: {}", name.empty() ? title : name));
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
					logger.Log(std::format("Invalid calling convention: '{}' at: {}", callConv, name.empty() ? title : name));
				}
			}

			{
				ScopeLogger{logger};
				for (const auto& property : paramTypes) {
					if (property.type == ValueType::Void) {
						logger.Log(std::format("Parameter cannot be void type at: {}", name.empty() ? title : name));
					} else if (property.type == ValueType::Function && property.ref.value_or(false)) {
						logger.Log(std::format("Parameter with function type cannot be reference at: {}", name.empty() ? title : name));
					}

					if (property.prototype) {
						property.prototype->Validate(logger, name.empty() ? title : name);
					}

					if (property.enumerate) {
						property.enumerate->Validate(logger, name.empty() ? title : name);
					}
				}
			}

			if (retType.ref.value_or(false)) {
				logger.Log("Return cannot be reference");
			}

			if (retType.prototype) {
				retType.prototype->Validate(logger, name.empty() ? title : name, false);
			}

			if (retType.enumerate) {
				retType.enumerate->Validate(logger, name.empty() ? title : name);
			}

			if (varIndex != kNoVarArgs && varIndex >= paramTypes.size()) {
				logger.Log("Invalid variable argument index");
			}
		}

		static inline const uint8_t kNoVarArgs = 0xFFU;

		bool operator==(const Method& rhs) const noexcept { return name == rhs.name; }
	};
}
