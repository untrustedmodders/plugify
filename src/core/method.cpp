#include <wizard/method.h>

using namespace wizard;

bool Method::Read(const utils::json::Value& object) {
    paramTypes.clear();

    for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
        const char* blockName = itr->name.GetString();
        const auto& value = itr->value;

        if (!strcmp(blockName, "name")) {
            if (value.IsString()) {
                name = value.GetString();
            }
        }
        else if(!strcmp(blockName, "paramTypes")) {
            if (value.IsArray()) {
                for (const auto& val : value.GetArray()) {
                    if (val.IsString()) {
                        paramTypes.push_back(ValueTypeFromString(val.GetString()));
                    }
                }
            }
        }
        else if(!strcmp(blockName, "callConv")) {
            if (value.IsString()) {
                callConv = value.GetString();
            }
        }
        else if(!strcmp(blockName, "retType")) {
            if (value.IsString()) {
                retType = ValueTypeFromString(value.GetString());
            }
        }
        else if(!strcmp(blockName, "varIndex")) {
            if (value.IsUint()) {
                varIndex = value.GetUint();
            }
        }
    }

    if (name.empty()) {
        WIZARD_LOG("Method: Should contain valid name!", ErrorLevel::ERROR);
        return false;
    }

    if (retType == ValueType::Invalid) {
        WIZARD_LOG("Method: '" + name + "' Invalid return type!", ErrorLevel::ERROR);
        return false;
    }

    for (size_t i = 0; i < paramTypes.size(); ++i) {
        if (paramTypes[i] == ValueType::Invalid) {
            WIZARD_LOG("Method: '" + name + "' Invalid parameter type - at index: " + std::to_string(i), ErrorLevel::ERROR);
            return false;
        }
    }

    return true;
}

namespace wizard {

static std::string_view ValueTypeToString(ValueType valueType) {
    switch (valueType) {
        case ValueType::Void:      return "void";
        case ValueType::SChar:     return "signed char";
        case ValueType::UChar:     return "unsigned char";
        case ValueType::Short:     return "short";
        case ValueType::UShort:    return "unsigned short";
        case ValueType::Int:       return "int";
        case ValueType::UInt:      return "unsigned int";
        case ValueType::Long:      return "long";
        case ValueType::ULong:     return "unsigned long";
        case ValueType::LongLong:  return "long long";
        case ValueType::ULongLong: return "unsigned long long";
        case ValueType::Char:      return "char";
        case ValueType::Char16:    return "char16_t";
        case ValueType::Char32:    return "char32_t";
        case ValueType::WChar:     return "wchar_t";
        case ValueType::Uint8:     return "uint8_t";
        case ValueType::Int8:      return "int8_t";
        case ValueType::Uint16:    return "uint16_t";
        case ValueType::Int16:     return "int16_t";
        case ValueType::Int32:     return "int32_t";
        case ValueType::Uint32:    return "uint32_t";
        case ValueType::Uint64:    return "uint64_t";
        case ValueType::Int64:     return "int64_t";
        case ValueType::Float:     return "float";
        case ValueType::Double:    return "double";
        case ValueType::Bool:      return "bool";
        case ValueType::Uintptr:   return "uintptr_t";
        case ValueType::Intptr:    return "intptr_t";
        case ValueType::Ptr:       return "*";
        case ValueType::String:    return "string_t";
        default:                   return "Invalid";
    }
}

static ValueType ValueTypeFromString(std::string_view valueType) {
    static std::unordered_map<std::string_view , ValueType> valueMap {
        { "void",               ValueType::Void },
        { "signed char",        ValueType::SChar },
        { "unsigned char",      ValueType::UChar },
        { "short",              ValueType::Short },
        { "unsigned short",     ValueType::UShort },
        { "int",                ValueType::Int },
        { "unsigned int",       ValueType::UInt },
        { "long",               ValueType::Long },
        { "unsigned long",      ValueType::ULong },
        { "long long",          ValueType::LongLong },
        { "unsigned long long", ValueType::ULongLong },
        { "char",               ValueType::Char },
        { "char16_t",           ValueType::Char16 },
        { "char32_t",           ValueType::Char32 },
        { "wchar_t",            ValueType::WChar },
        { "uint8_t",            ValueType::Uint8 },
        { "int8_t",             ValueType::Int8 },
        { "uint16_t",           ValueType::Uint16 },
        { "int16_t",            ValueType::Int16 },
        { "int32_t",            ValueType::Int32 },
        { "uint32_t",           ValueType::Uint32 },
        { "uint64_t",           ValueType::Uint64 },
        { "int64_t",            ValueType::Int64 },
        { "float",              ValueType::Float },
        { "double",             ValueType::Double },
        { "bool",               ValueType::Bool },
        { "uintptr_t",          ValueType::Uintptr },
        { "intptr_t",           ValueType::Intptr },
        { "*",                  ValueType::Ptr },
        { "string_t",           ValueType::String }
    };
    auto it = valueMap.find(valueType);
    return it != valueMap.end() ? it->second : ValueType::Invalid;
}

}
