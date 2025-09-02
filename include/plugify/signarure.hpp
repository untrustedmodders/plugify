#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "plugify/value_type.hpp"

namespace plugify {
    /**
     * @struct Signature
     * @brief Replacement for asmjit::FuncSignature using ValueType
     */
    struct Signature {
        std::string callConv;     ///< Calling convention
        uint32_t varIndex;         ///< Variable index for variadic functions
        ValueType retType;         ///< Return type
        std::vector<ValueType> argTypes; ///< Argument types

        Signature() : varIndex(0), retType(ValueType::Void) {}

        Signature(std::string_view conv, uint32_t varIdx, ValueType ret)
            : callConv(conv), varIndex(varIdx), retType(ret) {}

        void addArg(ValueType type) { argTypes.push_back(type); }
        size_t argCount() const noexcept { return argTypes.size(); }
        bool hasRet() const noexcept { return retType != ValueType::Void; }
    };
}