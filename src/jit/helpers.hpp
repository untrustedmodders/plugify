#pragma once

#include <asmjit/core.h>

#include "plugify/signarure.hpp"
#include "plugify/value_type.hpp"

/**
 * @brief Namespace containing utility functions of jit related things.
 */
namespace plugify {
	template<typename T>
	constexpr asmjit::TypeId GetTypeIdx() noexcept {
		return static_cast<asmjit::TypeId>(asmjit::TypeUtils::TypeIdOfT<T>::kTypeId);
	}

	bool HasHiArgSlot(asmjit::TypeId typeId) noexcept;

	asmjit::TypeId GetValueTypeId(ValueType valueType) noexcept;

	asmjit::TypeId GetRetTypeId(ValueType valueType) noexcept;

	asmjit::CallConvId GetCallConv([[maybe_unused]] std::string_view conv) noexcept;

	struct SimpleErrorHandler : asmjit::ErrorHandler {
		asmjit::Error error{asmjit::kErrorOk};
		const char* code{};

		void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* ) override {
			error = err;
			code = message;
		}
	};

    inline asmjit::FuncSignature ConvertSignature(const Signature& sig) {
        asmjit::FuncSignature asmSig(
            GetCallConv(sig.callConv),
            sig.varIndex,
            GetRetTypeId(sig.retType)
        );

        for (const auto& arg : sig.argTypes) {
            asmSig.addArg(GetValueTypeId(arg));
        }

        return asmSig;
    }
} // namespace plugify::JitUtils

