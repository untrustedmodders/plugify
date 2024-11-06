#pragma once

#include <type_traits>

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
		Ref() noexcept : _impl{nullptr} {}
		Ref(T& impl) noexcept : _impl{std::addressof(impl)} {}

		Ref(Ref const&) = default;
		Ref(Ref&&) = default;

		bool operator==(const Ref& other) const noexcept { return _impl == other._impl; }
		bool operator==(const T* impl) const noexcept { return _impl == impl; }

		Ref& operator=(const Ref&) & = default;
		Ref& operator=(const Ref&) && = delete;
		Ref& operator=(Ref&&) & = default;
		Ref& operator=(Ref&&) && = delete;

		explicit operator bool() const noexcept { return _impl != nullptr; }

	protected:
		T* _impl;
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
