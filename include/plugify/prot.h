#pragma once

#include <cstdint>
#include <type_traits>

namespace plugify {
	/**
	 * @enum ProtFlag
	 * @brief Enum representing memory protection flags.
	 */
	enum ProtFlag : uint8_t {
		UNSET = 0, /**< Value means this gives no information about protection state (un-read) */
		X = 1 << 1, /**< Execute permission */
		R = 1 << 2, /**< Read permission */
		W = 1 << 3, /**< Write permission */
		S = 1 << 4, /**< Shared memory */
		P = 1 << 5, /**< Private memory */
		N = 1 << 6, /**< Value equaling the Linux flag PROT_UNSET (read the prot, and the prot is unset) */
		RWX = R | W | X /**< Read, Write, and Execute permissions */
	};

	/**
	 * @brief Overloads the binary OR operator for ProtFlag.
	 *
	 * Allows combining ProtFlag values using the | operator.
	 *
	 * @param lhs The left-hand side ProtFlag value.
	 * @param rhs The right-hand side ProtFlag value.
	 * @return The combined ProtFlag value.
	 */
	inline ProtFlag operator|(ProtFlag lhs, ProtFlag rhs) {
		using underlying = typename std::underlying_type<ProtFlag>::type;
		return static_cast<ProtFlag> (
				static_cast<underlying>(lhs) |
				static_cast<underlying>(rhs)
		);
	}

	/**
	 * @brief Overloads the binary AND operator for ProtFlag.
	 *
	 * Allows checking if a ProtFlag value contains a specific flag using the & operator.
	 *
	 * @param lhs The left-hand side ProtFlag value.
	 * @param rhs The right-hand side ProtFlag value.
	 * @return True if lhs contains rhs, false otherwise.
	 */
	inline bool operator&(ProtFlag lhs, ProtFlag rhs) {
		using underlying = typename std::underlying_type<ProtFlag>::type;
		return static_cast<underlying>(lhs) &
			   static_cast<underlying>(rhs);
	}
} // namespace plugify