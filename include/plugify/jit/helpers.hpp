#pragma once

#include <asmjit/asmjit.h>
#include <plugify/method.hpp>

/**
 * @brief Namespace containing utility functions of jit related things.
 */
namespace plugify::JitUtils {
	template<typename T>
	constexpr asmjit::TypeId GetTypeIdx() noexcept {
		return static_cast<asmjit::TypeId>(asmjit::TypeUtils::TypeIdOfT<T>::kTypeId);
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

