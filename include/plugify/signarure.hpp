#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "plugify/value_type.hpp"
#include "plugify/method.hpp"

namespace plugify {
    /**
     * @struct Signature
     * @brief Replacement for asmjit::FuncSignature using ValueType
     */
    struct Signature {
        std::string callConv;     ///< Calling convention
        ValueType retType;         ///< Return type
        uint8_t varIndex;         ///< Variable index for variadic functions
        std::vector<ValueType> argTypes; ///< Argument types

        Signature() : varIndex(Method::kNoVarArgs), retType(ValueType::Void) {}

        Signature(std::string_view conv, ValueType ret, uint8_t varIdx = Method::kNoVarArgs)
            : callConv(conv), retType(ret), varIndex(varIdx) {}

        void AddArg(ValueType type) { argTypes.push_back(type); }
        size_t ArgCount() const noexcept { return argTypes.size(); }
        bool HasRet() const noexcept { return retType != ValueType::Void; }

        void SetRet(ValueType type) noexcept { retType = type; }
    };
}