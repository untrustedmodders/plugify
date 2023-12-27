#include "value_type.h"

namespace wizard {
    std::string_view ValueTypeToString(ValueType valueType) {
        switch (valueType) {
            case ValueType::Void:   return "void";
            case ValueType::Bool:   return "bool";
            case ValueType::Char8:  return "char8";
            case ValueType::Char16: return "char16";
            case ValueType::Int8:   return "int8";
            case ValueType::Int16:  return "int16";
            case ValueType::Int32:  return "int32";
            case ValueType::Int64:  return "int64";
            case ValueType::Uint8:  return "uint8";
            case ValueType::Uint16: return "uint16";
            case ValueType::Uint32: return "uint32";
            case ValueType::Uint64: return "uint64";
            case ValueType::Ptr64:  return "ptr64";
            case ValueType::Float:  return "float";
            case ValueType::Double: return "double";
            case ValueType::String: return "string";
            default:                return "invalid";
        }
    }

    ValueType ValueTypeFromString(std::string_view valueType) {
        static std::unordered_map<std::string_view, ValueType> valueMap {
            { "void",   ValueType::Void },
            { "bool",   ValueType::Bool },
            { "char8",  ValueType::Char8 },
            { "char16", ValueType::Char16 },
            { "int8",   ValueType::Int8 },
            { "int16",  ValueType::Int16 },
            { "int32",  ValueType::Int32 },
            { "int64",  ValueType::Int64 },
            { "uint8",  ValueType::Uint8 },
            { "uint16", ValueType::Uint16 },
            { "uint32", ValueType::Uint32 },
            { "uint64", ValueType::Uint64 },
            { "ptr64",  ValueType::Ptr64 },
            { "float",  ValueType::Float },
            { "double", ValueType::Double },
            { "string", ValueType::String },
        };
        auto it = valueMap.find(valueType);
        return it != valueMap.end() ? it->second : ValueType::Invalid;
    }
}