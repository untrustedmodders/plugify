#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace plugify {
	/**
	 * @brief A wrapper class for memory addresses, providing utility functions for pointer
	 * manipulation.
	 */
	class MemAddr { // TODO: Rename to Address in next major
	public:
		MemAddr() noexcept = default;

		MemAddr(nullptr_t) noexcept : m_value(0) {}

		MemAddr(uintptr_t addr) noexcept : m_value(addr) {}

		template <typename T> requires std::is_pointer_v<T>
		MemAddr(T addr) noexcept : m_value(reinterpret_cast<uintptr_t>(addr)) {}

		template <typename T> requires std::is_integral_v<T> && (sizeof(T) <= sizeof(uintptr_t))
		MemAddr(T addr) noexcept : m_value(static_cast<uintptr_t>(addr)) {}

		operator uintptr_t() const noexcept {
			return m_value;
		}

		explicit operator bool() const noexcept {
			return m_value != 0;
		}

		operator uint8_t*() const noexcept {
			return reinterpret_cast<uint8_t*>(m_value);
		}

		operator void*() const noexcept {
			return reinterpret_cast<void*>(m_value);
		}

		template <typename T>
		[[nodiscard]] T As() noexcept {
			return reinterpret_cast<T>(m_value);
		}

		template <typename T>
		[[nodiscard]] T As() const noexcept {
			return reinterpret_cast<T>(m_value);
		}

		template <typename T>
		[[nodiscard]] T Get() noexcept {
			return *reinterpret_cast<T*>(m_value);
		}

		template <typename T>
		[[nodiscard]] T Get() const noexcept {
			return *reinterpret_cast<T*>(m_value);
		}

		[[nodiscard]] uintptr_t GetPtr() const noexcept {
			return m_value;
		}

		[[nodiscard]] bool IsValid() const noexcept {
			return m_value >= 0x1000 && m_value < 0x7FFFFFFEFFFF;
		}

		[[nodiscard]] MemAddr Offset(auto offset) const noexcept {
			return m_value + static_cast<uintptr_t>(offset);
		}

		MemAddr& OffsetSelf(auto offset) noexcept {
			m_value += static_cast<uintptr_t>(offset);
			return *this;
		}

		[[nodiscard]] MemAddr Deref(auto offset, ptrdiff_t deref = 1) const noexcept {
			uintptr_t base = m_value;

			while (deref--) {
				base = *reinterpret_cast<uintptr_t*>(base + static_cast<uintptr_t>(offset));
			}

			return base;
		}

		MemAddr& DerefSelf(auto offset, ptrdiff_t deref = 1) noexcept {
			while (deref--) {
				m_value = *reinterpret_cast<uintptr_t*>(m_value + static_cast<uintptr_t>(offset));
			}

			return *this;
		}

		// ── Equality operators ────────────────────────────────────────────────────

		[[nodiscard]] bool operator==(auto other) const noexcept {
			return m_value == static_cast<uintptr_t>(other);
		}

		[[nodiscard]] auto operator<=>(auto other) const noexcept {
			return m_value <=> static_cast<uintptr_t>(other);
		}

		// ── Arithmetic operators ────────────────────────────────────────────────────

		[[nodiscard]] MemAddr operator+(auto offset) const noexcept {
			return m_value + static_cast<uintptr_t>(offset);
		}

		[[nodiscard]] MemAddr operator-(auto offset) const noexcept {
			return m_value - static_cast<uintptr_t>(offset);
		}

		MemAddr& operator+=(auto offset) noexcept {
			m_value += static_cast<uintptr_t>(offset);
			return *this;
		}

		MemAddr& operator-=(auto offset) noexcept {
			m_value -= static_cast<uintptr_t>(offset);
			return *this;
		}

		[[nodiscard]] ptrdiff_t operator-(MemAddr other) const noexcept {
			return static_cast<ptrdiff_t>(m_value) - static_cast<ptrdiff_t>(other.m_value);
		}

		template <typename T> requires(!std::is_same_v<T, MemAddr>)
		[[nodiscard]] friend MemAddr operator+(T offset, MemAddr addr) noexcept {
			return static_cast<uintptr_t>(offset) + addr.m_value;
		}

		template <typename T> requires(!std::is_same_v<T, MemAddr>)
		[[nodiscard]] friend MemAddr operator-(T offset, MemAddr addr) noexcept {
			return static_cast<uintptr_t>(offset) - addr.m_value;
		}

		// ── Bitwise operators ───────────────────────────────────────────────────────

		[[nodiscard]] MemAddr operator&(auto mask) const noexcept {
			return m_value & static_cast<uintptr_t>(mask);
		}

		[[nodiscard]] MemAddr operator|(auto mask) const noexcept {
			return m_value | static_cast<uintptr_t>(mask);
		}

		[[nodiscard]] MemAddr operator^(auto mask) const noexcept {
			return m_value ^ static_cast<uintptr_t>(mask);
		}

		[[nodiscard]] MemAddr operator>>(auto shift) const noexcept {
			return m_value >> static_cast<uintptr_t>(shift);
		}

		[[nodiscard]] MemAddr operator<<(auto shift) const noexcept {
			return m_value << static_cast<uintptr_t>(shift);
		}

		// ── Increment / Decrement ───────────────────────────────────────────────────

		MemAddr& operator++() noexcept {
			++m_value;
			return *this;
		}

		[[nodiscard]] MemAddr operator++(int) noexcept {
			const auto t = *this;
			++m_value;
			return t;
		}

		MemAddr& operator--() noexcept {
			--m_value;
			return *this;
		}

		[[nodiscard]] MemAddr operator--(int) noexcept {
			const auto t = *this;
			--m_value;
			return t;
		}

	private:
		uintptr_t m_value;//!< The memory address.
	};
	using Address = MemAddr;
}  // namespace plugify
