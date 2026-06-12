#pragma once

#include <type_traits>

namespace plg {
	template <typename T>
	concept bitmask_enum =
#ifdef __cpp_lib_is_scoped_enum
    	std::is_scoped_enum_v<T>
#else
    	std::is_enum_v<T>
#endif
    && requires(T e) {
        enable_bitmask_operators(e);
    };

	template <plg::bitmask_enum T>
	class flag {
		T val_;
	public:
		constexpr flag(T val) noexcept : val_(val) {}
		constexpr operator T() const noexcept { return val_; }
		constexpr explicit operator bool() const noexcept {
			using U = std::underlying_type_t<T>;
			return static_cast<U>(val_) != 0;
		}
	};
}

template <plg::bitmask_enum T>
constexpr T operator|(T lhs, T rhs) noexcept {
	using U = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

template <plg::bitmask_enum T>
constexpr plg::flag<T> operator&(T lhs, T rhs) noexcept {
	using U = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

template <plg::bitmask_enum T>
constexpr T operator^(T lhs, T rhs) noexcept {
	using U = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
}

template <plg::bitmask_enum T>
constexpr T operator~(T rhs) noexcept {
	using U = std::underlying_type_t<T>;
	return static_cast<T>(~static_cast<U>(rhs));
}

template <plg::bitmask_enum T>
constexpr T& operator|=(T& lhs, T rhs) noexcept {
	return lhs = lhs | rhs;
}

template <plg::bitmask_enum T>
constexpr T& operator&=(T& lhs, T rhs) noexcept {
	return lhs = lhs & rhs;
}

template <plg::bitmask_enum T>
constexpr T& operator^=(T& lhs, T rhs) noexcept {
	return lhs = lhs ^ rhs;
}