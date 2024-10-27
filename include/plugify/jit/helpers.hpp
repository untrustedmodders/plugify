#pragma once

#include <asmjit/asmjit.h>
#include <plugify/method.hpp>

namespace plugify {
	/**
	 * @brief Namespace containing utility functions of jit related things.
	 */
	namespace JitUtils {
		template<typename T>
		[[nodiscard]] constexpr asmjit::TypeId GetTypeIdx() noexcept {
			return static_cast<asmjit::TypeId>(asmjit::TypeUtils::TypeIdOfT<T>::kTypeId);
		}

		[[nodiscard]] asmjit::TypeId GetValueTypeId(ValueType valueType) noexcept;

		[[nodiscard]] asmjit::TypeId GetRetTypeId(ValueType valueType) noexcept;

		[[nodiscard]] asmjit::CallConvId GetCallConv([[maybe_unused]] std::string_view conv) noexcept;
	} // namespace JitUtils
} // namespace plugify
