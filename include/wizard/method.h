#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace wizard {
	enum class ValueType : std::uint8_t {
		Invalid,
		Void,
		Bool,
		Char8,
		Char16,
		Int8,
		Int16,
		Int32,
		Int64,
		Uint8,
		Uint16,
		Uint32,
		Uint64,
		Ptr64,
		Float,
		Double,
		String,
		Function,
	};

	struct Property {
		ValueType type;
	};

	struct Method {
		std::string name;
		std::string funcName;
		std::string callConv;
		std::vector<Property> paramTypes;
		Property retType{ };
		std::uint8_t varIndex{ kNoVarArgs };

		static inline const std::uint8_t kNoVarArgs = 0xFFu;

		bool operator==(const Method& rhs) const { return name == rhs.name; }
	};

	[[maybe_unused]] constexpr std::string_view ValueTypeToString(ValueType value) {
		switch (value) {
			case ValueType::Void:     return "void";
			case ValueType::Bool:     return "bool";
			case ValueType::Char8:    return "char8";
			case ValueType::Char16:   return "char16";
			case ValueType::Int8:     return "int8";
			case ValueType::Int16:    return "int16";
			case ValueType::Int32:    return "int32";
			case ValueType::Int64:    return "int64";
			case ValueType::Uint8:    return "uint8";
			case ValueType::Uint16:   return "uint16";
			case ValueType::Uint32:   return "uint32";
			case ValueType::Uint64:   return "uint64";
			case ValueType::Ptr64:    return "ptr64";
			case ValueType::Float:    return "float";
			case ValueType::Double:   return "double";
			case ValueType::String:   return "string";
			case ValueType::Function: return "function";
			default:                  return "unknown";
		}
	}

	[[maybe_unused]] constexpr ValueType ValueTypeFromString(std::string_view value) {
		if (value == "void") {
			return ValueType::Void;
		} else if (value == "bool") {
			return ValueType::Bool;
		} else if (value == "char8") {
			return ValueType::Char8;
		} else if (value == "char16") {
			return ValueType::Char16;
		} else if (value == "int8") {
			return ValueType::Int8;
		} else if (value == "int16") {
			return ValueType::Int16;
		} else if (value == "int32") {
			return ValueType::Int32;
		} else if (value == "int64") {
			return ValueType::Int64;
		} else if (value == "uint8") {
			return ValueType::Uint8;
		} else if (value == "uint16") {
			return ValueType::Uint16;
		} else if (value == "uint32") {
			return ValueType::Uint32;
		} else if (value == "uint64") {
			return ValueType::Uint64;
		} else if (value == "ptr64") {
			return ValueType::Ptr64;
		} else if (value == "float") {
			return ValueType::Float;
		} else if (value == "double") {
			return ValueType::Double;
		} else if (value == "string") {
			return ValueType::String;
		} else if (value == "function") {
			return ValueType::Function;
		}
		return ValueType::Invalid;
	}
}