#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace plugify {
	/**
	 * @brief A wrapper class for memory addresses, providing utility functions for pointer manipulation.
	 */
	class MemAddr {
	public:
		/**
		 * @brief Default constructor initializing the pointer to 0.
		 */
		constexpr MemAddr() noexcept : _ptr{0} {}

		/**
		 * @brief Constructor initializing the pointer with a uintptr_t value.
		 * @param ptr The uintptr_t value to initialize the pointer with.
		 */
		constexpr MemAddr(const uintptr_t ptr) noexcept : _ptr{ptr} {}

		/**
		 * @brief Template constructor initializing the pointer with a typed pointer.
		 * @tparam T The type of the pointer.
		 * @param ptr The typed pointer to initialize with.
		 */
		template<typename T> requires (std::is_pointer_v<T> or std::is_null_pointer_v<T>)
		MemAddr(T ptr) noexcept : _ptr{reinterpret_cast<uintptr_t>(ptr)} {}

		/**
		 * @brief Constructor initializing the pointer with a void pointer.
		 * @param other The void pointer to initialize with.
		 */
		constexpr MemAddr(const MemAddr& other) noexcept = default;

		/**
		 * @brief Move constructor initializing the pointer with another MemAddr.
		 * @param other The MemAddr to move from.
		 */
		constexpr MemAddr(MemAddr&& other) noexcept = default;

		/**
		 * @brief Assignment operator to copy another MemAddr.
		 * @param other The MemAddr to copy from.
		 * @return A reference to the current MemAddr object.
		 */
		constexpr MemAddr& operator=(const MemAddr& other) noexcept = default;

		/**
		 * @brief Move assignment operator to move another MemAddr.
		 * @param other The MemAddr to move from.
		 * @return A reference to the current MemAddr object.
		 */
		constexpr MemAddr& operator=(MemAddr&& other) noexcept = default;

		/**
		 * @brief Converts the MemAddr object to a uintptr_t.
		 * @return The uintptr_t representation of the memory address.
		 */
		constexpr operator uintptr_t() const noexcept {
			return _ptr;
		}

		/**
		 * @brief Converts the MemAddr object to a void pointer.
		 * @return The void pointer representation of the memory address.
		 */
		constexpr operator void*() const noexcept {
			return _addr;
		}

		/**
		 * @brief Explicit conversion operator to bool, indicating if the pointer is non-zero.
		 * @return True if the pointer is non-zero, false otherwise.
		 */
		constexpr explicit operator bool() const noexcept {
			return _ptr != 0;
		}

		/**
		 * @brief Inequality operator.
		 * @param addr The MemAddr object to compare with.
		 * @return True if the pointers are not equal, false otherwise.
		 */
		constexpr bool operator!=(const MemAddr addr) const noexcept {
			return _ptr != addr._ptr;
		}

		/**
		 * @brief Less-than comparison operator.
		 * @param addr The MemAddr object to compare with.
		 * @return True if this pointer is less than the other, false otherwise.
		 */
		constexpr bool operator<(const MemAddr addr) const noexcept {
			return _ptr < addr._ptr;
		}

		/**
		 * @brief Equality operator.
		 * @param addr The MemAddr object to compare with.
		 * @return True if the pointers are equal, false otherwise.
		 */
		constexpr bool operator==(const MemAddr addr) const noexcept {
			return _ptr == addr._ptr;
		}

		/**
		 * @brief Equality operator for comparing with uintptr_t.
		 * @param addr The uintptr_t value to compare with.
		 * @return True if the pointer is equal to the uintptr_t value, false otherwise.
		 */
		constexpr bool operator==(const uintptr_t addr) const noexcept {
			return _ptr == addr;
		}

		/**
		 * @brief Returns the uintptr_t representation of the pointer.
		 * @return The uintptr_t value of the pointer.
		 */
		constexpr uintptr_t GetPtr() const noexcept {
			return _ptr;
		}

		/**
		 * @brief Retrieves the value at the memory address.
		 * @tparam T The type of the value.
		 * @return The value at the memory address.
		 */
		template<class T>
		constexpr T GetValue() const noexcept {
			return *reinterpret_cast<T*>(_ptr);
		}

		/**
		 * @brief Casts the pointer to a specified type using C-style cast.
		 * @tparam T The type to cast to.
		 * @return The casted pointer.
		 */
		template<typename T>
		constexpr T CCast() const noexcept {
			return (T) _ptr;
		}

		/**
		 * @brief Casts the pointer to a specified type using reinterpret_cast.
		 * @tparam T The type to cast to.
		 * @return The casted pointer.
		 */
		template<typename T>
		constexpr T RCast() const noexcept {
			return reinterpret_cast<T>(_ptr);
		}

		/**
		 * @brief Casts the pointer to a specified type using a union cast.
		 * @tparam T The type to cast to.
		 * @return The casted pointer.
		 */
		template<typename T>
		constexpr T UCast() const noexcept {
			union {
				uintptr_t ptr;
				T val;
			} cast;
			return cast.ptr = _ptr, cast.val;
		}

		/**
		 * @brief Offsets the memory address by a specified amount.
		 * @param offset The offset value.
		 * @return A new MemAddr object with the offset applied.
		 */
		constexpr MemAddr Offset(const ptrdiff_t offset) const noexcept {
			return _ptr + static_cast<uintptr_t>(offset);
		}

		/**
		 * @brief Offsets the memory address by a specified amount in-place.
		 * @param offset The offset value.
		 * @return A reference to the current MemAddr object.
		 */
		constexpr MemAddr& OffsetSelf(const ptrdiff_t offset) noexcept {
			_ptr += static_cast<uintptr_t>(offset);
			return *this;
		}

		/**
		 * @brief Dereferences the memory address a specified number of times.
		 * @param deref The number of times to dereference.
		 * @return A new MemAddr object after dereferencing.
		 */
		constexpr MemAddr Deref(ptrdiff_t deref = 1) const {
			uintptr_t reference = _ptr;

			while (deref--) {
				if (reference)
					reference = *reinterpret_cast<uintptr_t*>(reference);
			}

			return reference;
		}

		/**
		 * @brief Dereferences the memory address a specified number of times in-place.
		 * @param deref The number of times to dereference.
		 * @return A reference to the current MemAddr object.
		 */
		constexpr MemAddr& DerefSelf(ptrdiff_t deref = 1) {
			while (deref--) {
				if (_ptr)
					_ptr = *reinterpret_cast<uintptr_t*>(_ptr);
			}

			return *this;
		}

		/**
		 * @brief Follows a near call to resolve the address.
		 * @param opcodeOffset The offset to the opcode.
		 * @param nextInstructionOffset The offset to the next instruction.
		 * @return A new MemAddr object with the resolved address.
		 */
		MemAddr FollowNearCall(const ptrdiff_t opcodeOffset = 0x1, const ptrdiff_t nextInstructionOffset = 0x5) const {
			return ResolveRelativeAddress(opcodeOffset, nextInstructionOffset);
		}

		/**
		 * @brief Follows a near call to resolve the address in-place.
		 * @param opcodeOffset The offset to the opcode.
		 * @param nextInstructionOffset The offset to the next instruction.
		 * @return A reference to the current MemAddr object with the resolved address.
		 */
		MemAddr& FollowNearCallSelf(const ptrdiff_t opcodeOffset = 0x1, const ptrdiff_t nextInstructionOffset = 0x5) {
			return ResolveRelativeAddressSelf(opcodeOffset, nextInstructionOffset);
		}

		/**
		 * @brief Resolves a relative address.
		 * @param registerOffset The offset to the register.
		 * @param nextInstructionOffset The offset to the next instruction.
		 * @return A new MemAddr object with the resolved address.
		 */
		MemAddr ResolveRelativeAddress(const ptrdiff_t registerOffset = 0x0, const ptrdiff_t nextInstructionOffset = 0x4) const {
			const uintptr_t skipRegister = _ptr + static_cast<uintptr_t>(registerOffset);
			const int32_t relativeAddress = *reinterpret_cast<int32_t*>(skipRegister);
			const uintptr_t nextInstruction = _ptr + static_cast<uintptr_t>(nextInstructionOffset);
			return nextInstruction + static_cast<uintptr_t>(relativeAddress);
		}

		/**
		 * @brief Resolves a relative address in-place.
		 * @param registerOffset The offset to the register.
		 * @param nextInstructionOffset The offset to the next instruction.
		 * @return A reference to the current MemAddr object with the resolved address.
		 */
		MemAddr& ResolveRelativeAddressSelf(const ptrdiff_t registerOffset = 0x0, const ptrdiff_t nextInstructionOffset = 0x4) {
			const uintptr_t skipRegister = _ptr + static_cast<uintptr_t>(registerOffset);
			const int32_t relativeAddress = *reinterpret_cast<int32_t*>(skipRegister);
			const uintptr_t nextInstruction = _ptr + static_cast<uintptr_t>(nextInstructionOffset);
			_ptr = nextInstruction + static_cast<uintptr_t>(relativeAddress);
			return *this;
		}

	private:
		union {
			void* _addr;
			uintptr_t _ptr;
		}; //!< The memory address.
	};
}// namespace plugify
