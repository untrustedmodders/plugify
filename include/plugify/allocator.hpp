#pragma once

#include <cstddef> // for std::size_t, std::ptrdiff_t
#include <memory>  // for std::allocator and std::allocator_traits

namespace plg {
	// Forward declaration for allocator<void>
	template <typename T>
	class allocator;

	// Specialization for `void`, but we no longer need to define `pointer` and `const_pointer`
	template <>
	class allocator<void>
	{
	public:
		typedef void value_type;

		// Rebind struct
		template <class U>
		struct rebind { using other = allocator<U>; };
	};

	// Define the custom allocator inheriting from std::allocator
	template <typename T>
	class allocator : public std::allocator<T> {
	public:
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;

		// Default constructor
		allocator() {}

		// Copy constructor
		template <class U>
		allocator(const allocator<U>&) {}

		// Rebind struct
		template <class U>
		struct rebind { using other = allocator<U>; };

		// Override allocate method to use custom allocation function
		pointer allocate(size_type n, std::allocator_traits<allocator<void>>::const_pointer hint = nullptr)  {
			return static_cast<pointer>(std::malloc(n * sizeof(T)));
		}

		// Override deallocate method to use custom deallocation function
		void deallocate(pointer p, size_type n) {
			std::free(p);
		}

		// You can inherit other methods like construct and destroy from std::allocator
	};

	// Comparison operators for compatibility
	template <typename T, typename U>
	constexpr bool operator==(const allocator<T>&, const allocator<U>) { return true; }

	template <typename T, typename U>
	constexpr bool operator!=(const allocator<T>&, const allocator<U>) { return false; }

} // namespace plg
