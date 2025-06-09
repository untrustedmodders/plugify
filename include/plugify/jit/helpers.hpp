#pragma once

#include <asmjit/core.h>
#include <plugify/method.hpp>

/**
 * @brief Namespace containing utility functions of jit related things.
 */
namespace plugify::JitUtils {
	template<typename T>
	constexpr asmjit::TypeId GetTypeIdx() noexcept {
		return static_cast<asmjit::TypeId>(asmjit::TypeUtils::TypeIdOfT<T>::kTypeId);
	}

	constexpr bool HasHiArgSlot(asmjit::TypeId typeId) noexcept {
		// 64bit width regs can fit wider args
		if constexpr (PLUGIFY_ARCH_BITS == 64) {
			return false;
		}

		switch (typeId) {
			case asmjit::TypeId::kInt64:
			case asmjit::TypeId::kUInt64:
				return true;
			default:
				return false;
		}
	}

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
} // namespace plugify::JitUtils

