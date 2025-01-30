#pragma once

#include <type_traits>
#include <utility>

namespace plugify {
	/**
	 * @class Handle
	 * @brief A generic handle class that manages a pointer to an object of type T.
	 *
	 * The Handle class provides a lightweight wrapper around a raw pointer, allowing
	 * safe access and management of resources. It can be null and provides comparison
	 * operators and conversion functions.
	 *
	 * @tparam T The type of the object being referenced.
	 */
	template<typename T>
	class Handle {
	public:
		/**
		 * @brief Default constructor. Initializes the handle with a null pointer.
		 */
		Handle() noexcept : _impl{nullptr} {}

		/**
		 * @brief Constructs a Handle object from an instance of type T.
		 * @param impl A reference to an object of type T to be wrapped.
		 */
		Handle(T& impl) noexcept : _impl{&impl} {}

		/**
		 * @brief Copy constructor. Creates a new Handle object from another Handle object.
		 * @param other The Handle object to copy from.
		 */
		Handle(const Handle&) = default;
		/**
		 * @brief Move constructor. Transfers ownership from another Handle object.
		 * @param other The Handle object to move from.
		 */
		Handle(Handle&&) = default;

		/**
		 * @brief Comparison operator (<=>) for comparing two Handle objects.
		 * @param rhs The right-hand side Handle object for comparison.
		 * @return The result of the comparison.
		 */
		auto operator<=>(const Handle&) const = default;

		/**
		 * @brief Copy assignment operator. Copies the handle from another Handle object.
		 * @param other The Handle object to copy from.
		 * @return A reference to the current object.
		 */
		Handle& operator=(const Handle&) & = default;
		/**
		 * @brief Copy assignment operator for rvalue references is deleted.
		 * @param other The Handle object to copy from.
		 */
		Handle& operator=(const Handle&) && = delete;
		/**
		 * @brief Move assignment operator. Transfers ownership from another Handle object.
		 * @param other The Handle object to move from.
		 * @return A reference to the current object.
		 */
		Handle& operator=(Handle&&) & = default;
		/**
		 * @brief Move assignment operator for rvalue references is deleted.
		 * @param other The Handle object to move from.
		 */
		Handle& operator=(Handle&&) && = delete;

		/**
		 * @brief Explicit conversion operator to bool, indicating if the pointer is non-zero.
		 * @return True if the pointer is non-zero, false otherwise.
		 */
		explicit operator bool() const noexcept {
			return _impl != nullptr;
		}

		/**
		 * @brief Converts the Handle object to a uintptr_t.
		 * @return The uintptr_t representation of the memory address.
		 */
		operator uintptr_t() const noexcept {
			return reinterpret_cast<uintptr_t>(_impl);
		}

		/**
		 * @brief Converts the Handle object to a void pointer.
		 * @return The void pointer representation of the memory address.
		 */
		operator void*() const noexcept {
			return (void*) _impl;
		}

	protected:
		T* _impl; //!< A pointer to the referenced implementation of type T.
	};
}
