#pragma once

#include <cstddef> // for std::size_t, std::ptrdiff_t
#include <cstdlib> // for std::malloc, std::free, std::aligned_alloc
#include <type_traits>  // for std::is_constant_evaluated
#include <new>  // for ::operator new, ::operator delete

#include "plg/macro.hpp"

namespace plg {
	// Forward declaration for allocator<void>
	template<typename T>
	class allocator;

	// Specialization for `void`, but we no longer need to define `pointer` and `const_pointer`
	template<>
	class allocator<void> {
	public:
		using value_type = void;

		// Rebind struct
		template<class U>
		struct rebind { using other = allocator<U>; };
	};

	// Define the custom allocator inheriting from std::allocator
	template<typename T>
	class allocator {
	public:
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		// Default constructor
		constexpr allocator() noexcept = default;

		// Copy constructor
		template<class U>
		constexpr allocator(const allocator<U>&) noexcept {}

		// Rebind struct
		template<class U>
		struct rebind { using other = allocator<U>; };

		// Override allocate method to use custom allocation function
		constexpr pointer allocate(size_type n, [[maybe_unused]] const_pointer hint = nullptr) {
			static_assert(sizeof(T) != 0, "cannot allocate incomplete types");
			static_assert((alignof(T) & (alignof(T) - 1)) == 0, "alignof(T) must be a power of 2");

			if (n > max_size()) [[unlikely]] {
				if (n > static_cast<size_type>(-1) / sizeof(T)) {
					PLUGIFY_ASSERT(false, "plg::allocator::allocate(): bad array new length", std::bad_array_new_length);
				}
				PLUGIFY_ASSERT(false, "plg::allocator::allocate(): too big", std::bad_alloc);
			}

			pointer ret;
			size_type size = n * sizeof(T);
			if (std::is_constant_evaluated()) {
				ret = static_cast<T*>(::operator new(size));
			} else {
				if constexpr (alignof(T) > alignof(std::max_align_t)) {
					size_type aligned_size = (size + (alignof(T) - 1)) & ~(alignof(T) - 1);
					ret = static_cast<T*>(aligned_allocate(alignof(T), aligned_size));
				} else {
					ret = static_cast<T*>(std::malloc(size));
				}

				if (!ret) {
					PLUGIFY_ASSERT(false, "plg::allocator::allocate(): bad allocation", std::bad_alloc);
				}
			}

			return ret;
		}

		// Override deallocate method to use custom deallocation function
		constexpr void deallocate(pointer p, [[maybe_unused]] size_type n) {
			if (std::is_constant_evaluated()) {
				::operator delete(p);
			} else {
				std::free(p);
			}
		}

	private:
		constexpr size_type max_size() noexcept {
#if __PTRDIFF_MAX__ < __SIZE_MAX__
			return static_cast<size_type>(__PTRDIFF_MAX__) / sizeof(T);
#else
			return static_cast<size_type>(-1) / sizeof(T);
#endif // __PTRDIFF_MAX__
		}

		void* aligned_allocate(size_type alignment, size_type size) {
#if _WIN32
			return _aligned_malloc(size, alignment);
#else
			return std::aligned_alloc(alignment, size);
#endif // _WIN32
		}
	};

	// Comparison operators for compatibility
	template<typename T, typename U>
	constexpr bool operator==(const allocator<T>&, const allocator<U>) { return true; }

	template<typename T, typename U>
	constexpr bool operator!=(const allocator<T>&, const allocator<U>) { return false; }

} // namespace plg
