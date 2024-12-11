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
} // namespace plugify::JitUtils

