#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace plugify {
	/**
	 * @brief A wrapper class for memory addresses, providing utility functions for pointer
	 * manipulation.
	 *
	 * Holds a raw address and converts to and from pointers and integers freely,
	 * so a resolved symbol can be carried around, offset, dereferenced and cast
	 * without a cast at every step. Nothing here checks what it points at, which
	 * makes every accessor as unsafe as the pointer arithmetic it replaces.
	 */
	class Address {
	public:
		/**
		 * @brief Construct an uninitialised address.
		 */
		Address() noexcept = default;

		/**
		 * @brief Construct a null address.
		 */
		Address(std::nullptr_t) noexcept : m_value(0) {}

		/**
		 * @brief Construct from a pointer.
		 * @tparam T Any pointer type.
		 * @param ptr The pointer to wrap.
		 */
		template <typename T> requires std::is_pointer_v<T>
		Address(T ptr) noexcept : m_value(reinterpret_cast<uintptr_t>(ptr)) {}

		/**
		 * @brief Construct from an integer.
		 * @tparam T Any integral type.
		 * @param val The address as a number.
		 */
		template <typename T> requires std::is_integral_v<T>
		Address(T val) noexcept : m_value(static_cast<uintptr_t>(val)) {}

		/**
		 * @brief Convert to the raw address.
		 * @return The address as an integer.
		 */
		operator uintptr_t() const noexcept {
			return m_value;
		}

		/**
		 * @brief Test whether the address is non-null.
		 * @return True if the address is not zero.
		 */
		explicit operator bool() const noexcept {
			return m_value != 0;
		}

		/**
		 * @brief Convert to a byte pointer.
		 * @return The address as a pointer to bytes.
		 */
		operator uint8_t*() const noexcept {
			return reinterpret_cast<uint8_t*>(m_value);
		}

		/**
		 * @brief Convert to an untyped pointer.
		 * @return The address as a void pointer.
		 */
		operator void*() const noexcept {
			return reinterpret_cast<void*>(m_value);
		}

		/**
		 * @brief Reinterpret the address as another type.
		 * @tparam T The type to cast to, usually a pointer or a function pointer.
		 * @return The address as a @p T.
		 */
		template <typename T>
		[[nodiscard]] T As() noexcept {
			return reinterpret_cast<T>(m_value);
		}

		/**
		 * @brief Reinterpret the address as another type.
		 * @tparam T The type to cast to, usually a pointer or a function pointer.
		 * @return The address as a @p T.
		 */
		template <typename T>
		[[nodiscard]] T As() const noexcept {
			return reinterpret_cast<T>(m_value);
		}

		/**
		 * @brief Read the value stored at the address.
		 * @tparam T Type of the value to read.
		 * @return A copy of the value at the address.
		 */
		template <typename T>
		[[nodiscard]] T Get() noexcept {
			return *reinterpret_cast<T*>(m_value);
		}

		/**
		 * @brief Read the value stored at the address.
		 * @tparam T Type of the value to read.
		 * @return A copy of the value at the address.
		 */
		template <typename T>
		[[nodiscard]] T Get() const noexcept {
			return *reinterpret_cast<T*>(m_value);
		}

		/**
		 * @brief Get the raw address.
		 * @return The address as an integer.
		 */
		[[nodiscard]] uintptr_t GetPtr() const noexcept {
			return m_value;
		}

		/**
		 * @brief Check that the address looks like usable user-space memory.
		 * @return True if the address is past the first page and below the top of the user-space
		 * range.
		 * @note A cheap plausibility check, not a guarantee that the memory is mapped.
		 */
		[[nodiscard]] bool IsValid() const noexcept {
			return m_value >= 0x1000 && m_value < 0x7FFFFFFEFFFF;
		}

		/**
		 * @brief Get the address shifted by an offset.
		 * @param offset How far to shift, in bytes.
		 * @return The shifted address.
		 */
		[[nodiscard]] Address Offset(auto offset) const noexcept {
			return m_value + static_cast<uintptr_t>(offset);
		}

		/**
		 * @brief Shift this address by an offset.
		 * @param offset How far to shift, in bytes.
		 * @return A reference to this instance after shifting.
		 */
		Address& OffsetSelf(auto offset) noexcept {
			m_value += static_cast<uintptr_t>(offset);
			return *this;
		}

		/**
		 * @brief Follow a chain of pointers from this address.
		 * @param offset Byte offset added before each read.
		 * @param deref How many times to read a pointer.
		 * @return The address the chain ends at.
		 */
		[[nodiscard]] Address Deref(auto offset, ptrdiff_t deref = 1) const noexcept {
			uintptr_t base = m_value;

			while (deref--) {
				base = *reinterpret_cast<uintptr_t*>(base + static_cast<uintptr_t>(offset));
			}

			return base;
		}

		/**
		 * @brief Follow a chain of pointers, in place.
		 * @param offset Byte offset added before each read.
		 * @param deref How many times to read a pointer.
		 * @return A reference to this instance after following the chain.
		 */
		Address& DerefSelf(auto offset, ptrdiff_t deref = 1) noexcept {
			while (deref--) {
				m_value = *reinterpret_cast<uintptr_t*>(m_value + static_cast<uintptr_t>(offset));
			}

			return *this;
		}

		// ── Equality operators ────────────────────────────────────────────────────

		/**
		 * @brief Compare this address with another address or number.
		 * @param other The value to compare against.
		 * @return True if both denote the same address.
		 */
		[[nodiscard]] bool operator==(auto other) const noexcept {
			return m_value == static_cast<uintptr_t>(other);
		}

		/**
		 * @brief Order this address against another address or number.
		 * @param other The value to compare against.
		 * @return The relative ordering of the two addresses.
		 */
		[[nodiscard]] auto operator<=>(auto other) const noexcept {
			return m_value <=> static_cast<uintptr_t>(other);
		}

		// ── Arithmetic operators ────────────────────────────────────────────────────

		/**
		 * @brief Add a byte offset.
		 * @param offset How far to shift.
		 * @return The shifted address.
		 */
		[[nodiscard]] Address operator+(auto offset) const noexcept {
			return m_value + static_cast<uintptr_t>(offset);
		}

		/**
		 * @brief Subtract a byte offset.
		 * @param offset How far to shift back.
		 * @return The shifted address.
		 */
		[[nodiscard]] Address operator-(auto offset) const noexcept {
			return m_value - static_cast<uintptr_t>(offset);
		}

		/**
		 * @brief Add a byte offset in place.
		 * @param offset How far to shift.
		 * @return A reference to this instance after shifting.
		 */
		Address& operator+=(auto offset) noexcept {
			m_value += static_cast<uintptr_t>(offset);
			return *this;
		}

		/**
		 * @brief Subtract a byte offset in place.
		 * @param offset How far to shift back.
		 * @return A reference to this instance after shifting.
		 */
		Address& operator-=(auto offset) noexcept {
			m_value -= static_cast<uintptr_t>(offset);
			return *this;
		}

		/**
		 * @brief Get the distance between two addresses.
		 * @param other The address to measure from.
		 * @return The signed distance in bytes.
		 */
		[[nodiscard]] ptrdiff_t operator-(Address other) const noexcept {
			return static_cast<ptrdiff_t>(m_value) - static_cast<ptrdiff_t>(other.m_value);
		}

		/**
		 * @brief Add a byte offset, with the offset on the left.
		 * @tparam T Any type other than Address.
		 * @param offset How far to shift.
		 * @param addr The address to shift.
		 * @return The shifted address.
		 */
		template <typename T> requires(!std::is_same_v<T, Address>)
		[[nodiscard]] friend Address operator+(T offset, Address addr) noexcept {
			return static_cast<uintptr_t>(offset) + addr.m_value;
		}

		/**
		 * @brief Subtract an address from a number.
		 * @tparam T Any type other than Address.
		 * @param offset The number to subtract from.
		 * @param addr The address to subtract.
		 * @return The resulting address.
		 */
		template <typename T> requires(!std::is_same_v<T, Address>)
		[[nodiscard]] friend Address operator-(T offset, Address addr) noexcept {
			return static_cast<uintptr_t>(offset) - addr.m_value;
		}

		// ── Bitwise operators ───────────────────────────────────────────────────────

		/**
		 * @brief Mask the address.
		 * @param mask The mask to apply.
		 * @return The masked address.
		 */
		[[nodiscard]] Address operator&(auto mask) const noexcept {
			return m_value & static_cast<uintptr_t>(mask);
		}

		/**
		 * @brief Set bits in the address.
		 * @param mask The bits to set.
		 * @return The resulting address.
		 */
		[[nodiscard]] Address operator|(auto mask) const noexcept {
			return m_value | static_cast<uintptr_t>(mask);
		}

		/**
		 * @brief Flip bits in the address.
		 * @param mask The bits to flip.
		 * @return The resulting address.
		 */
		[[nodiscard]] Address operator^(auto mask) const noexcept {
			return m_value ^ static_cast<uintptr_t>(mask);
		}

		/**
		 * @brief Shift the address right.
		 * @param shift How many bits to shift by.
		 * @return The resulting address.
		 */
		[[nodiscard]] Address operator>>(auto shift) const noexcept {
			return m_value >> static_cast<uintptr_t>(shift);
		}

		/**
		 * @brief Shift the address left.
		 * @param shift How many bits to shift by.
		 * @return The resulting address.
		 */
		[[nodiscard]] Address operator<<(auto shift) const noexcept {
			return m_value << static_cast<uintptr_t>(shift);
		}

		// ── Increment / Decrement ───────────────────────────────────────────────────

		/**
		 * @brief Advance the address by one byte.
		 * @return A reference to this instance after advancing.
		 */
		Address& operator++() noexcept {
			++m_value;
			return *this;
		}

		/**
		 * @brief Advance the address by one byte.
		 * @return The address as it was before advancing.
		 */
		[[nodiscard]] Address operator++(int) noexcept {
			const Address tmp = *this;
			++m_value;
			return tmp;
		}

		/**
		 * @brief Step the address back by one byte.
		 * @return A reference to this instance after stepping back.
		 */
		Address& operator--() noexcept {
			--m_value;
			return *this;
		}

		/**
		 * @brief Step the address back by one byte.
		 * @return The address as it was before stepping back.
		 */
		[[nodiscard]] Address operator--(int) noexcept {
			const Address tmp = *this;
			--m_value;
			return tmp;
		}

	private:
		uintptr_t m_value; //!< The memory address.
	};
}  // namespace plugify
