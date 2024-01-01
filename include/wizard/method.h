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

        bool operator==(const Method& rhs) const { return name == rhs.name; }
    };

    [[maybe_unused]] constexpr std::string_view ValueTypeToString(ValueType value) {
        using enum ValueType;
        switch (value) {
            case Void:   return "void";
            case Bool:   return "bool";
            case Char8:  return "char8";
            case Char16: return "char16";
            case Int8:   return "int8";
            case Int16:  return "int16";
            case Int32:  return "int32";
            case Int64:  return "int64";
            case Uint8:  return "uint8";
            case Uint16: return "uint16";
            case Uint32: return "uint32";
            case Uint64: return "uint64";
            case Ptr64:  return "ptr64";
            case Float:  return "float";
            case Double: return "double";
            case String: return "string";
            default:     return "unknown";
        }
    }

    [[maybe_unused]] constexpr ValueType ValueTypeFromString(std::string_view value) {
        using enum ValueType;
        if (value == "void") {
            return Void;
        } else if (value == "bool") {
            return Bool;
        } else if (value == "char8") {
            return Char8;
        } else if (value == "char16") {
            return Char16;
        } else if (value == "int8") {
            return Int8;
        } else if (value == "int16") {
            return Int16;
        } else if (value == "int32") {
            return Int32;
        } else if (value == "int64") {
            return Int64;
        } else if (value == "uint8") {
            return Uint8;
        } else if (value == "uint16") {
            return Uint16;
        } else if (value == "uint32") {
            return Uint32;
        } else if (value == "uint64") {
            return Uint64;
        } else if (value == "ptr64") {
            return Ptr64;
        } else if (value == "float") {
            return Float;
        } else if (value == "double") {
            return Double;
        } else if (value == "string") {
            return String;
        }
        return Invalid;
    }
}