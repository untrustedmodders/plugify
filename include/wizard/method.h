#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace wizard {
    enum class ValueType : uint8_t {
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
}