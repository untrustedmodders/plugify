#pragma once

#include <cstddef>		// for std::size_t, std::ptrdiff_t
#include <cstdlib>		// for std::malloc, std::free, std::aligned_alloc
#include <new>			// for ::operator new, ::operator delete
#include <type_traits>	// for std::is_constant_evaluated

#include "plg/config.hpp"

namespace plg {
	template<typename T>
	class allocator;

	template<>
	class allocator<void> {
	public:
		using value_type = void;

		template<class U>
		struct rebind { using other = allocator<U>; };
	};

	template<typename T>
	class allocator {
		static_assert(!std::is_const_v<T>, "plg::allocator does not support const types");
		static_assert(!std::is_volatile_v<T>, "plg::allocator does not support volatile types");
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		constexpr allocator() noexcept = default;

		template<class U>
		constexpr allocator(const allocator<U>&) noexcept {}

		template<class U>
		struct rebind { using other = allocator<U>; };

		[[nodiscard]] constexpr T* allocate(size_type n) {
			static_assert(sizeof(T) != 0, "cannot allocate incomplete types");
			static_assert((alignof(T) & (alignof(T) - 1)) == 0, "alignof(T) must be a power of 2");

			if (n > std::allocator_traits<allocator>::max_size(*this)) {
				throw_bad_array_new_length();
			}

			size_type size = n * sizeof(T);
			if (std::is_constant_evaluated()) {
				return static_cast<T*>(::operator new(size));
			} else {
				return malloc_allocate(size);
			}
		}

		constexpr void deallocate(T* p, [[maybe_unused]] size_type n) {
			if (std::is_constant_evaluated()) {
				::operator delete(p);
			} else {
				std::free(p);
			}
		}

	private:
		static T* malloc_allocate(size_type size) {
			T* ret;
			if constexpr (alignof(T) > alignof(std::max_align_t)) {
				size_type aligned_size = (size + (alignof(T) - 1)) & ~(alignof(T) - 1);
				ret = static_cast<T*>(aligned_allocate(alignof(T), aligned_size));
			} else {
				ret = static_cast<T*>(std::malloc(size));
			}
			if (!ret) {
				throw_bad_alloc();
			}
			return ret;
		}

		[[noreturn]] static void throw_bad_array_new_length() {
			PLUGIFY_THROW("bad array new length", std::bad_array_new_length);
		}

		[[noreturn]] static void throw_bad_alloc() {
			PLUGIFY_THROW("bad allocation", std::bad_alloc);
		}

		static void* aligned_allocate(size_type alignment, size_type size) {
#if PLUGIFY_PLATFORM_WINDOWS
			return _aligned_malloc(size, alignment);
#else
			return std::aligned_alloc(alignment, size);
#endif // PLUGIFY_PLATFORM_WINDOWS
		}
	};

	template<typename T, typename U>
	constexpr bool operator==(const allocator<T>&, const allocator<U>) { return true; }

	template<typename T, typename U>
	constexpr bool operator!=(const allocator<T>&, const allocator<U>) { return false; }

	template <typename Alloc>
	void swap_allocator(Alloc& a1, Alloc& a2, std::true_type) {
		using std::swap;
		swap(a1, a2);
	}

	template <typename Alloc>
	void swap_allocator(Alloc&, Alloc&, std::false_type) noexcept {}

	template <typename Alloc>
	void swap_allocator(Alloc& a1, Alloc& a2) {
		swap_allocator(a1, a2, std::integral_constant<bool, std::allocator_traits<Alloc>::propagate_on_container_swap::value>());
	}

	template <class Pointer, class Size = std::size_t>
	struct allocation_result {
		Pointer ptr;
		Size count;
	};

	template <class Alloc>
	[[nodiscard]] allocation_result<typename std::allocator_traits<Alloc>::pointer>
	allocate_at_least(Alloc& alloc, size_t n) {
		return { alloc.allocate(n), n };
	}

	template <class T, class U>
	constexpr bool is_pointer_in_range(const T* begin, const T* end, const U* ptr) {
		if (std::is_constant_evaluated())
			return false;
		return reinterpret_cast<const char*>(begin) <= reinterpret_cast<const char*>(ptr) &&
			   reinterpret_cast<const char*>(ptr) < reinterpret_cast<const char*>(end);
	}

	template <class T, class U>
	constexpr bool is_overlapping_range(const T* begin, const T* end, const U* begin2) {
		auto size = end - begin;
		auto end2 = begin2 + size;
		return is_pointer_in_range(begin, end, begin2) || is_pointer_in_range(begin2, end2, begin);
	}

	// asan_annotate_container_with_allocator determines whether containers with custom allocators are annotated. This is
	// a public customization point to disable annotations if the custom allocator assumes that the memory isn't poisoned.
	// See the https://libcxx.llvm.org/UsingLibcxx.html#turning-off-asan-annotation-in-containers for more information.
#if PLUGIFY_INSTRUMENTED_WITH_ASAN
	template <class Alloc>
	struct asan_annotate_container_with_allocator : std::true_type {};
#endif // PLUGIFY_INSTRUMENTED_WITH_ASAN

	// Annotate a contiguous range.
	// [__first_storage, __last_storage) is the allocated memory region,
	// __old_last_contained is the previously last allowed (unpoisoned) element, and
	// __new_last_contained is the new last allowed (unpoisoned) element.
	template <class Allocator>
	void annotate_contiguous_container(
		[[maybe_unused]] const void* first_storage,
		[[maybe_unused]] const void* last_storage,
		[[maybe_unused]] const void* old_last_contained,
		[[maybe_unused]] const void* new_last_contained
	) {
#if PLUGIFY_INSTRUMENTED_WITH_ASAN
		if (!std::is_constant_evaluated()
			&& asan_annotate_container_with_allocator<Allocator>::value
			&& first_storage != nullptr) {
			__sanitizer_annotate_contiguous_container(
				first_storage,
				last_storage,
				old_last_contained,
				new_last_contained
			);
		}
#endif // PLUGIFY_INSTRUMENTED_WITH_ASAN
	}
} // namespace plg
