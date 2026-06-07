#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <bit>

namespace plugify {
	/**
	 * @brief A wrapper class for memory addresses, providing utility functions for pointer
	 * manipulation.
	 */
	class MemAddr { // TODO: Rename to Address in next major
	public:
		constexpr MemAddr() noexcept = default;

		constexpr MemAddr(uintptr_t ptr) noexcept
			: m_value(ptr) {
		}

		constexpr MemAddr(const void* addr) noexcept
			: m_value(std::bit_cast<uintptr_t>(addr)) {
		}

		constexpr operator uintptr_t() const noexcept {
			return m_value;
		}

		constexpr explicit operator bool() const noexcept {
			return m_value != 0;
		}

		constexpr operator uint8_t*() const noexcept {
			return std::bit_cast<uint8_t*>(m_value);
		}

		constexpr operator void*() const noexcept {
			return std::bit_cast<void*>(m_value);
		}

		template <typename T>
		[[nodiscard]] constexpr T As() noexcept {
			return reinterpret_cast<T>(m_value);
		}

		template <typename T>
		[[nodiscard]] constexpr T As() const noexcept {
			return reinterpret_cast<T>(m_value);
		}

		template <typename T>
		[[nodiscard]] constexpr T Get() noexcept {
			return *reinterpret_cast<T*>(m_value);
		}

		template <typename T>
		[[nodiscard]] constexpr T Get() const noexcept {
			return *reinterpret_cast<T*>(m_value);
		}

		[[nodiscard]] constexpr uintptr_t GetPtr() const noexcept {
			return m_value;
		}

		[[nodiscard]] constexpr bool IsValid() const noexcept {
			return m_value >= 0x1000 && m_value < 0x7FFFFFFEFFFF;
		}

		[[nodiscard]] constexpr MemAddr Offset(ptrdiff_t offset) const noexcept {
			return m_value + offset;
		}

		constexpr MemAddr& OffsetSelf(ptrdiff_t offset) noexcept {
			m_value += offset;
			return *this;
		}

		[[nodiscard]] constexpr MemAddr Deref(ptrdiff_t offset, ptrdiff_t deref = 1) const noexcept {
			uintptr_t base = m_value;

			while (deref--) {
				base = *reinterpret_cast<uintptr_t*>(base + offset);
			}

			return base;
		}

		constexpr MemAddr& DerefSelf(ptrdiff_t offset, ptrdiff_t deref = 1) noexcept {
			while (deref--) {
				m_value = *reinterpret_cast<uintptr_t*>(m_value + offset);
			}

			return *this;
		}

		// ── Equality operators ────────────────────────────────────────────────────

		[[nodiscard]] constexpr bool operator==(const auto& other) const noexcept {
			return m_value == static_cast<uintptr_t>(other);
		}

		[[nodiscard]] constexpr auto operator<=>(const auto& other) const noexcept {
			return m_value <=> static_cast<uintptr_t>(other);
		}

		// ── Arithmetic operators ────────────────────────────────────────────────────

		[[nodiscard]] constexpr MemAddr operator+(const auto& offset) const noexcept {
			return m_value + static_cast<uintptr_t>(offset);
		}

		[[nodiscard]] constexpr MemAddr operator-(const auto& offset) const noexcept {
			return m_value - static_cast<uintptr_t>(offset);
		}

		constexpr MemAddr& operator+=(const auto& offset) noexcept {
			m_value += static_cast<uintptr_t>(offset);
			return *this;
		}

		constexpr MemAddr& operator-=(const auto& offset) noexcept {
			m_value -= static_cast<uintptr_t>(offset);
			return *this;
		}

		[[nodiscard]] constexpr ptrdiff_t operator-(const MemAddr& other) const noexcept {
			return static_cast<ptrdiff_t>(m_value) - static_cast<ptrdiff_t>(other.m_value);
		}

		template <typename T>
			requires(!std::is_same_v<T, MemAddr>)
		[[nodiscard]] friend constexpr MemAddr
		operator+(const T& offset, const MemAddr& addr) noexcept {
			return static_cast<uintptr_t>(offset) + addr.m_value;
		}

		template <typename T>
			requires(!std::is_same_v<T, MemAddr>)
		[[nodiscard]] friend constexpr MemAddr
		operator-(const T& offset, const MemAddr& addr) noexcept {
			return static_cast<uintptr_t>(offset) - addr.m_value;
		}

		// ── Bitwise operators ───────────────────────────────────────────────────────

		[[nodiscard]] constexpr MemAddr operator&(const auto& mask) const noexcept {
			return m_value & static_cast<uintptr_t>(mask);
		}

		[[nodiscard]] constexpr MemAddr operator|(const auto& mask) const noexcept {
			return m_value | static_cast<uintptr_t>(mask);
		}

		[[nodiscard]] constexpr MemAddr operator^(const auto& mask) const noexcept {
			return m_value ^ static_cast<uintptr_t>(mask);
		}

		[[nodiscard]] constexpr MemAddr operator>>(const auto& shift) const noexcept {
			return m_value >> static_cast<uintptr_t>(shift);
		}

		[[nodiscard]] constexpr MemAddr operator<<(const auto& shift) const noexcept {
			return m_value << static_cast<uintptr_t>(shift);
		}

		// ── Increment / Decrement ───────────────────────────────────────────────────

		constexpr MemAddr& operator++() noexcept {
			++m_value;
			return *this;
		}

		[[nodiscard]] constexpr MemAddr operator++(int) noexcept {
			const auto t = *this;
			++m_value;
			return t;
		}

		constexpr MemAddr& operator--() noexcept {
			--m_value;
			return *this;
		}

		[[nodiscard]] constexpr MemAddr operator--(int) noexcept {
			const auto t = *this;
			--m_value;
			return t;
		}

	private:
		uintptr_t m_value;//!< The memory address.
	};
	using Address = MemAddr;
}  // namespace plugify
