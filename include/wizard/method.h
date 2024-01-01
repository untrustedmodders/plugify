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
        String
    };

    struct Method {
        std::string name;
        std::string funcName;
        std::string callConv;
        std::vector<ValueType> paramTypes;
        ValueType retType{ };
        std::uint8_t varIndex{ kNoVarArgs };

        static inline const std::uint8_t kNoVarArgs = 0xFFu;
    };

    [[maybe_unused]] constexpr std::string_view ValueTypeToString(ValueType value) {
        switch (value) {
            case ValueType::Void:   return "Void";
            case ValueType::Bool:   return "Bool";
            case ValueType::Char8:  return "Char8";
            case ValueType::Char16: return "Char16";
            case ValueType::Int8:   return "Int8";
            case ValueType::Int16:  return "Int16";
            case ValueType::Int32:  return "Int32";
            case ValueType::Int64:  return "Int64";
            case ValueType::Uint8:  return "Uint8";
            case ValueType::Uint16: return "Uint16";
            case ValueType::Uint32: return "Uint32";
            case ValueType::Uint64: return "Uint64";
            case ValueType::Ptr64:  return "Ptr64";
            case ValueType::Float:  return "Float";
            case ValueType::Double: return "Double";
            case ValueType::String: return "String";
            default: return "Unknown";
        }
    }

    [[maybe_unused]] constexpr ValueType ValueTypeFromString(std::string_view value) {
        if (value == "void")   return ValueType::Void;
        if (value == "bool")   return ValueType::Bool;
        if (value == "char8")  return ValueType::Char8;
        if (value == "char16") return ValueType::Char16;
        if (value == "int8")   return ValueType::Int8;
        if (value == "int16")  return ValueType::Int16;
        if (value == "int32")  return ValueType::Int32;
        if (value == "int64")  return ValueType::Int64;
        if (value == "uint8")  return ValueType::Uint8;
        if (value == "uint16") return ValueType::Uint16;
        if (value == "uint32") return ValueType::Uint32;
        if (value == "uint64") return ValueType::Uint64;
        if (value == "ptr64")  return ValueType::Ptr64;
        if (value == "float")  return ValueType::Float;
        if (value == "double") return ValueType::Double;
        if (value == "string") return ValueType::String;
        return ValueType::Invalid;
    }
}