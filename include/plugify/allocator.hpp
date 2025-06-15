#pragma once

#include <cstddef>   // for std::size_t, std::ptrdiff_t
#include <cstdlib>   // for std::malloc, std::free
#include <new>       // for placement new
#include <limits>    // for std::numeric_limits

namespace plg {
	template <typename T>
	struct allocator {
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		template <typename U>
		struct rebind {
			using other = allocator<U>;
		};

		allocator() noexcept = default;

		allocator(const allocator&) noexcept = default;

		template <typename U>
		allocator(const allocator<U>&) noexcept {}

		pointer allocate(size_type n) {
			if (n == 0) return nullptr;
			if (n > max_size()) throw std::bad_alloc();
			void* ptr = std::malloc(n * sizeof(T));
			if (!ptr) throw std::bad_alloc();
			return static_cast<pointer>(ptr);
		}

		void deallocate(pointer p, size_type) noexcept {
			if (p) std::free(p);
		}

		size_type max_size() const noexcept {
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		template <typename U, typename... Args>
		void construct(U* p, Args&&... args) {
			std::construct_at(p, std::forward<Args>(args)...);
		}

		template <typename U>
		void destroy(U* p) {
			std::destroy_at(p);
		}

		bool operator==(const allocator&) const noexcept { return true; }
		bool operator!=(const allocator&) const noexcept { return false; }
	};
} // namespace plg
