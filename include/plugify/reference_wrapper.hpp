#pragma once

#include <type_traits>
#include <utility>

namespace plugify {
	/**
	 * @class Ref
	 * @brief A lightweight reference wrapper for objects of type `T`.
	 *
	 * This template class provides a reference-like wrapper around an object of type `T`, ensuring
	 * the object can be safely referenced and compared. It handles copying and moving references, while
	 * maintaining a non-owning pointer to the wrapped object.
	 *
	 * @tparam T The type of the object to be referenced.
	 */
	template<typename T>
	class Ref {
	public:
		/**
		 * @brief Default constructor. Initializes the reference with a null pointer.
		 */
		Ref() noexcept : _impl{nullptr} {}

		/**
		 * @brief Constructs a Ref object from an instance of type T.
		 * @param impl A reference to an object of type T to be wrapped.
		 */
		Ref(T& impl) noexcept : _impl{std::addressof(impl)} {}

		/**
		 * @brief Copy constructor. Creates a new Ref object from another Ref object.
		 * @param other The Ref object to copy from.
		 */
		Ref(Ref const&) = default;
		/**
		 * @brief Move constructor. Transfers ownership from another Ref object.
		 * @param other The Ref object to move from.
		 */
		Ref(Ref&&) = default;

		/**
		 * @brief Equality operator. Checks if two Ref objects point to the same implementation.
		 * @param other The Ref object to compare with.
		 * @return True if both Ref objects reference the same implementation, false otherwise.
		 */
		bool operator==(const Ref& other) const noexcept { return _impl == other._impl; }
		/**
		 * @brief Equality operator. Checks if the Ref object points to a specific implementation.
		 * @param impl A pointer to the implementation to compare with.
		 * @return True if the Ref object references the specified implementation, false otherwise.
		 */
		bool operator==(const T* impl) const noexcept { return _impl == impl; }

		/**
		 * @brief Copy assignment operator. Copies the reference from another Ref object.
		 * @param other The Ref object to copy from.
		 * @return A reference to the current object.
		 */
		Ref& operator=(const Ref&) & = default;
		/**
		 * @brief Copy assignment operator for rvalue references is deleted.
		 * @param other The Ref object to copy from.
		 */
		Ref& operator=(const Ref&) && = delete;
		/**
		 * @brief Move assignment operator. Transfers ownership from another Ref object.
		 * @param other The Ref object to move from.
		 * @return A reference to the current object.
		 */
		Ref& operator=(Ref&&) & = default;
		/**
		 * @brief Move assignment operator for rvalue references is deleted.
		 * @param other The Ref object to move from.
		 */
		Ref& operator=(Ref&&) && = delete;

		/**
		 * @brief Checks whether the Ref object holds a valid reference.
		 * @return True if the Ref object holds a valid reference, false otherwise.
		 */
		explicit operator bool() const noexcept {
			return _impl != nullptr;
		}

		/**
		 * @brief Gets the raw pointer to the referenced implementation.
		 * @return A void pointer to the referenced implementation.
		 */
		void* GetPtr() const noexcept {
			return (void*) _impl;
		}

	protected:
		T* _impl; //!< A pointer to the referenced implementation of type T.
	};

	/**
	 * @var is_ref_v
	 * @brief A type trait that checks if a type `T` is a reference type.
	 *
	 * This type trait ensures that `T` follows the standard layout and has the size of a pointer, making
	 * it compatible for use with `Ref` wrappers.
	 *
	 * @tparam T The type to check.
	 */
	template<typename T> static constexpr bool is_ref_v = std::is_standard_layout_v<T> && sizeof(T) == sizeof(void*);
}

// Specialize std::hash for Ref<T>
/*template<typename T>
struct std::hash<plugify::Ref<T>> {
	std::size_t operator()(const plugify::Ref<T>& ref) const {
		return std::hash<void*>{}(ref.GetPtr());
	}
};*/