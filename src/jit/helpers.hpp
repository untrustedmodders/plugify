#pragma once

#include <asmjit/core.h>

#include "plugify/signarure.hpp"
#include "plugify/value_type.hpp"

/**
 * @brief Namespace containing utility functions of jit related things.
 */
namespace plugify {
	template <typename T>
	constexpr asmjit::TypeId GetTypeIdx() noexcept {
		return static_cast<asmjit::TypeId>(asmjit::TypeUtils::TypeIdOfT<T>::kTypeId);
	}

	bool HasHiArgSlot(asmjit::TypeId typeId) noexcept;

	asmjit::TypeId GetValueTypeId(ValueType valueType) noexcept;

	asmjit::TypeId GetRetTypeId(ValueType valueType) noexcept;

	asmjit::CallConvId GetCallConvId(CallConv callConv) noexcept;

	class SimpleErrorHandler : public asmjit::ErrorHandler {
	public:
		void handle_error(
		    asmjit::Error err,
		    const char* message,
		    [[maybe_unused]] asmjit::BaseEmitter* origin
		) override {
			error = err;
			code = message;
		}

		asmjit::Error error{ asmjit::kErrorOk };
		const char* code{};
	};

	inline asmjit::FuncSignature ConvertSignature(const Signature& sig) {
		asmjit::FuncSignature asmSig(
		    GetCallConvId(sig.callConv),
		    sig.varIndex,
		    GetRetTypeId(sig.retType)
		);

		for (const auto& arg : sig.argTypes) {
			asmSig.add_arg(GetValueTypeId(arg));
		}

		return asmSig;
	}
}  // namespace plugify::JitUtils
