#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace wizard {
    enum class ValueType : uint8_t {
        Invalid,
        Void,
        SChar,
        UChar,
        Short,
        UShort,
        Int,
        UInt,
        Long,
        ULong,
        LongLong,
        ULongLong,
        Char,
        Char16,
        Char32,
        WChar,
        Uint8,
        Int8,
        Uint16,
        Int16,
        Int32,
        Uint32,
        Uint64,
        Int64,
        Float,
        Double,
        Bool,
        Uintptr,
        Intptr,
        Ptr,
        String,
    };

    struct Method {
        std::string name;
        std::string callConv;
        std::vector<ValueType> paramTypes;
        ValueType retType{ ValueType::Void };
        std::uint8_t varIndex{ 0xFFu };

#if WIZARD_BUILD_MAIN_LIB
        Method() = default;

        bool Read(const utils::json::Value& object);
#endif
    };

#if WIZARD_BUILD_MAIN_LIB
    // How export to language modules ?
    [[maybe_unused]] static std::string_view ValueTypeToString(ValueType valueType);
    [[maybe_unused]] static ValueType ValueTypeFromString(std::string_view valueType);
#endif
}