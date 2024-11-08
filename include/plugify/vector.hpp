#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <memory>
#include <initializer_list>
#include <algorithm>
#include <numeric>
#include <execution>
#include <span>
#include <limits>
#include <iostream>
#include <optional>
#include <ranges>

#include <cstdint>
#include <cstddef>
#include <cstring>

#ifndef PLUGIFY_VECTOR_SMALL_BUFFER_OPTIMIZATION
#  define PLUGIFY_VECTOR_SMALL_BUFFER_OPTIMIZATION 1
#endif

#ifndef PLUGIFY_VECTOR_CONTAINERS_RANGES
#  define PLUGIFY_VECTOR_CONTAINERS_RANGES 1
#endif

#if PLUGIFY_VECTOR_CONTAINERS_RANGES && (__cplusplus <= 202002L || !__has_include(<ranges>) || !defined(__cpp_lib_containers_ranges))
#  undef PLUGIFY_VECTOR_CONTAINERS_RANGES
#  define PLUGIFY_VECTOR_CONTAINERS_RANGES 0
#endif

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
#  include <ranges>
#endif

#define _PLUGIFY_VECTOR_HAS_EXCEPTIONS (__cpp_exceptions || __EXCEPTIONS || _HAS_EXCEPTIONS)

#ifndef PLUGIFY_VECTOR_EXCEPTIONS
#  if _PLUGIFY_VECTOR_HAS_EXCEPTIONS
#    define PLUGIFY_VECTOR_EXCEPTIONS 1
#  else
#    define PLUGIFY_VECTOR_EXCEPTIONS 0
#  endif
#endif

#if PLUGIFY_VECTOR_EXCEPTIONS && (!_PLUGIFY_VECTOR_HAS_EXCEPTIONS || !__has_include(<stdexcept>))
#  undef PLUGIFY_VECTOR_EXCEPTIONS
#  define PLUGIFY_VECTOR_EXCEPTIONS 0
#endif

#ifndef PLUGIFY_VECTOR_FALLBACK_ASSERT
#  define PLUGIFY_VECTOR_FALLBACK_ASSERT 1
#endif

#if PLUGIFY_VECTOR_FALLBACK_ASSERT && !__has_include(<cassert>)
#  undef PLUGIFY_VECTOR_FALLBACK_ASSERT
#  define PLUGIFY_VECTOR_FALLBACK_ASSERT 0
#endif

#ifndef PLUGIFY_VECTOR_FALLBACK_ABORT
#  define PLUGIFY_VECTOR_FALLBACK_ABORT 1
#endif

#if PLUGIFY_VECTOR_FALLBACK_ABORT && !__has_include(<cstdlib>)
#  undef PLUGIFY_VECTOR_FALLBACK_ABORT
#  define PLUGIFY_VECTOR_FALLBACK_ABORT 0
#endif

#ifndef PLUGIFY_VECTOR_FALLBACK_ABORT_FUNCTION
#  define PLUGIFY_VECTOR_FALLBACK_ABORT_FUNCTION [] (auto) { }
#endif

#if PLUGIFY_VECTOR_EXCEPTIONS
#  include <stdexcept>
#  define _PLUGIFY_VECTOR_ASSERT(x, str, e) do { if (!(x)) [[unlikely]] throw e(str); } while (0)
#elif PLUGIFY_VECTOR_FALLBACK_ASSERT
#  include <cassert>
#  define _PLUGIFY_VECTOR_ASSERT(x, str, ...) assert(x && str)
#elif PLUGIFY_VECTOR_FALLBACK_ABORT
#  if !PLUGIFY_VECTOR_NUMERIC_CONVERSIONS
#    include <cstdlib>
#  endif
#  define _PLUGIFY_VECTOR_ASSERT(x, ...) do { if (!(x)) [[unlikely]] { std::abort(); } } while (0)
#else
#  define _PLUGIFY_VECTOR_ASSERT(x, str, ...) do { if (!(x)) [[unlikely]] { PLUGIFY_VECTOR_FALLBACK_ABORT_FUNCTION (str); { while (true) { [] { } (); } } } } while (0)
#endif

#define _PLUGIFY_VECTOR_PRAGMA_IMPL(x) _Pragma(#x)
#define _PLUGIFY_VECTOR_PRAGMA(x) _PLUGIFY_VECTOR_PRAGMA_IMPL(x)

#if defined(__clang__)
#  define _PLUGIFY_VECTOR_PRAGMA_DIAG_PREFIX clang
#elif defined(__GNUC__)
#  define _PLUGIFY_VECTOR_PRAGMA_DIAG_PREFIX GCC
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_VECTOR_WARN_PUSH() _PLUGIFY_VECTOR_PRAGMA(_PLUGIFY_VECTOR_PRAGMA_DIAG_PREFIX diagnostic push)
#  define _PLUGIFY_VECTOR_WARN_IGNORE(wrn) _PLUGIFY_VECTOR_PRAGMA(_PLUGIFY_VECTOR_PRAGMA_DIAG_PREFIX diagnostic ignored wrn)
#  define _PLUGIFY_VECTOR_WARN_POP() _PLUGIFY_VECTOR_PRAGMA(_PLUGIFY_VECTOR_PRAGMA_DIAG_PREFIX diagnostic pop)
#elif defined(_MSC_VER)
#  define _PLUGIFY_VECTOR_WARN_PUSH()	__pragma(warning(push))
#  define _PLUGIFY_VECTOR_WARN_IGNORE(wrn) __pragma(warning(disable: wrn))
#  define _PLUGIFY_VECTOR_WARN_POP() __pragma(warning(pop))
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_VECTOR_ALWAYS_INLINE __attribute__((always_inline)) inline
#  define _PLUGIFY_VECTOR_ALWAYS_RESTRICT __restrict__
#elif defined(_MSC_VER)
#  define _PLUGIFY_VECTOR_ALWAYS_INLINE __forceinline
#  define _PLUGIFY_VECTOR_ALWAYS_RESTRICT __restrict
#else
#  define _PLUGIFY_VECTOR_ALWAYS_INLINE inline
#  define _PLUGIFY_VECTOR_ALWAYS_RESTRICT
#endif

#if defined(_MSC_VER)
#  define _PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#  define _PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

namespace plg {
	template<typename Allocator>
	struct vector_iterator {
		using allocator_traits = std::allocator_traits<Allocator>;
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = typename allocator_traits::value_type;
		using difference_type = std::ptrdiff_t;
		using pointer = typename allocator_traits::pointer;
		using reference = value_type&;
	protected:
		pointer _current;
	public:
		constexpr vector_iterator() = default;
		constexpr vector_iterator(const vector_iterator& other) = default;
		constexpr vector_iterator(vector_iterator&& other) = default;
		constexpr vector_iterator(pointer ptr)
			: _current(ptr) {}
		constexpr vector_iterator& operator=(const vector_iterator& other) = default;
		constexpr vector_iterator& operator=(vector_iterator&& other) = default;
		constexpr ~vector_iterator() = default;
	public:
		constexpr reference operator*() const noexcept {
			return *_current;
		}
		constexpr pointer operator->() const noexcept {
			return _current;
		}
		constexpr vector_iterator& operator++() noexcept {
			++_current;
			return *this;
		}
		constexpr vector_iterator operator++(int) const noexcept {
			return vector_iterator(_current++);
		}
		constexpr vector_iterator& operator--() noexcept {
			--_current;
			return *this;
		}
		constexpr vector_iterator operator--(int) const noexcept {
			return vector_iterator(_current--);
		}
		constexpr vector_iterator& operator+=(const difference_type n) noexcept {
			_current += n;
			return *this;
		}
		constexpr vector_iterator operator+(const difference_type n) const noexcept {
			vector_iterator temp = *this;
			return temp += n;
		}
		constexpr vector_iterator& operator-=(const difference_type n) noexcept {
			_current -= n;
			return *this;
		}
		constexpr vector_iterator operator-(const difference_type n) const noexcept {
			vector_iterator temp = *this;
			return temp -= n;
		}
		constexpr reference operator[](const difference_type n) const noexcept {
			return _current[n];
		}
		template<typename Alloc>
		constexpr friend typename vector_iterator<Alloc>::difference_type operator-(const vector_iterator<Alloc>& lhs, const vector_iterator<Alloc>& rhs) noexcept;
		template<typename Alloc>
		constexpr friend bool operator==(const vector_iterator<Alloc>& lhs, const vector_iterator<Alloc>& rhs) noexcept;
		template<typename Alloc>
		constexpr friend auto operator<=>(const vector_iterator<Alloc>& lhs, const vector_iterator<Alloc>& rhs) noexcept;
		[[nodiscard]] operator const pointer() const noexcept {
			return _current;
		}
		[[nodiscard]] pointer base() const noexcept {
			return _current;
		}
	};

	template<typename Allocator>
	[[nodiscard]] constexpr typename vector_iterator<Allocator>::difference_type operator-(const vector_iterator<Allocator>& lhs, const vector_iterator<Allocator>& rhs) noexcept {
		using difference_type = typename vector_iterator<Allocator>::difference_type;
		return difference_type(lhs.base() - rhs.base());
	}
	template<typename Allocator>
	[[nodiscard]] constexpr bool operator==(const vector_iterator<Allocator>& lhs, const vector_iterator<Allocator>& rhs) noexcept {
		return lhs.base() == rhs.base();
	}
	template<typename Allocator>
	[[nodiscard]] constexpr auto operator<=>(const vector_iterator<Allocator>& lhs, const vector_iterator<Allocator>& rhs) noexcept {
		return lhs.base() <=> rhs.base();
	}

	template<typename Allocator>
	struct vector_const_iterator {
		using allocator_traits = std::allocator_traits<Allocator>;
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = typename allocator_traits::value_type;
		using difference_type = std::ptrdiff_t;
		using pointer = typename allocator_traits::const_pointer;
		using reference = const value_type&;
	protected:
		pointer _current;
	public:
		constexpr vector_const_iterator() = default;
		constexpr vector_const_iterator(const vector_const_iterator& other) = default;
		constexpr vector_const_iterator(vector_const_iterator&& other) = default;
		constexpr vector_const_iterator(pointer ptr)
			: _current(ptr) {}
		constexpr vector_const_iterator(const vector_iterator<Allocator>& other)  // allow only iterator to const_iterator conversion
			: _current(other.base()) {}
		constexpr vector_const_iterator& operator=(const vector_const_iterator& other) = default;
		constexpr vector_const_iterator& operator=(vector_const_iterator&& other) = default;
		constexpr ~vector_const_iterator() = default;
	public:
		constexpr reference operator*() const noexcept {
			return *_current;
		}
		constexpr pointer operator->() const noexcept {
			return _current;
		}
		constexpr vector_const_iterator& operator++() noexcept {
			++_current;
			return *this;
		}
		constexpr vector_const_iterator operator++(int) const noexcept {
			return vector_const_iterator(_current++);
		}
		constexpr vector_const_iterator& operator--() noexcept {
			--_current;
			return *this;
		}
		constexpr vector_const_iterator operator--(int) const noexcept {
			return vector_const_iterator(_current--);
		}
		constexpr vector_const_iterator& operator+=(const difference_type n) noexcept {
			_current += n;
			return *this;
		}
		constexpr vector_const_iterator operator+(const difference_type n) const noexcept {
			vector_const_iterator temp = *this;
			return temp += n;
		}
		constexpr vector_const_iterator& operator-=(const difference_type n) noexcept {
			_current -= n;
			return *this;
		}
		constexpr vector_const_iterator operator-(const difference_type n) const noexcept {
			vector_const_iterator temp = *this;
			return temp -= n;
		}
		constexpr reference operator[](const difference_type n) const noexcept {
			return _current[n];
		}
		template<typename Alloc>
		constexpr friend typename vector_const_iterator<Alloc>::difference_type operator-(const vector_const_iterator<Alloc>& lhs, const vector_const_iterator<Alloc>& rhs) noexcept;
		template<typename Alloc>
		constexpr friend bool operator==(const vector_const_iterator<Alloc>& lhs, const vector_const_iterator<Alloc>& rhs) noexcept;
		template<typename Alloc>
		constexpr friend auto operator<=>(const vector_const_iterator<Alloc>& lhs, const vector_const_iterator<Alloc>& rhs) noexcept;
		template<typename Alloc>
		constexpr friend typename vector_const_iterator<Alloc>::difference_type operator-(const vector_const_iterator<Alloc>& lhs, const vector_iterator<Alloc>& rhs) noexcept;
		template<typename Alloc>
		constexpr friend bool operator==(const vector_const_iterator<Alloc>& lhs, const vector_iterator<Alloc>& rhs) noexcept;
		template<typename Alloc>
		constexpr friend auto operator<=>(const vector_const_iterator<Alloc>& lhs, const vector_iterator<Alloc>& rhs) noexcept;
		[[nodiscard]] operator const pointer() const noexcept {
			return _current;
		}
		[[nodiscard]] pointer base() const noexcept {
			return _current;
		}
	};

	template<typename Allocator>
	[[nodiscard]] constexpr typename vector_const_iterator<Allocator>::difference_type operator-(const vector_const_iterator<Allocator>& lhs, const vector_const_iterator<Allocator>& rhs) noexcept {
		using difference_type = typename vector_const_iterator<Allocator>::difference_type;
		return difference_type(lhs.base() - rhs.base());
	}
	template<typename Allocator>
	[[nodiscard]] constexpr bool operator==(const vector_const_iterator<Allocator>& lhs, const vector_const_iterator<Allocator>& rhs) noexcept {
		return lhs.base() == rhs.base();
	}
	template<typename Allocator>
	[[nodiscard]] constexpr auto operator<=>(const vector_const_iterator<Allocator>& lhs, const vector_const_iterator<Allocator>& rhs) noexcept {
		return lhs.base() <=> rhs.base();
	}
	template<typename Allocator>
	[[nodiscard]] constexpr typename vector_const_iterator<Allocator>::difference_type operator-(const vector_const_iterator<Allocator>& lhs, const vector_iterator<Allocator>& rhs) noexcept {
		using difference_type = typename vector_const_iterator<Allocator>::difference_type;
		return difference_type(lhs.base() - rhs.base());
	}
	template<typename Allocator>
	[[nodiscard]] constexpr bool operator==(const vector_const_iterator<Allocator>& lhs, const vector_iterator<Allocator>& rhs) noexcept {
		return lhs.base() == rhs.base();
	}
	template<typename Allocator>
	[[nodiscard]] constexpr auto operator<=>(const vector_const_iterator<Allocator>& lhs, const vector_iterator<Allocator>& rhs) noexcept {
		return lhs.base() <=> rhs.base();
	}

	namespace detail {
		template<typename T>
		[[nodiscard]] constexpr T align_up(T val, T align) { return val + (align - (val % align)) % align; }

		template<std::forward_iterator InputIterator, typename Allocator, typename... Args>
		constexpr void construct(Allocator allocator, InputIterator first, InputIterator last, Args&&... args) {
			using allocator_traits = std::allocator_traits<Allocator>;
			while (first != last) {
				allocator_traits::construct(allocator, first, std::forward<Args>(args)...);
				++first;
			}
		}

		/*template<std::input_iterator InputIterator, std::forward_iterator ForwardIterator, typename Allocator>
		constexpr void uninitialized_copy(Allocator allocator, InputIterator first, InputIterator last, ForwardIterator d_first) {
			using allocator_traits = std::allocator_traits<Allocator>;
			auto dest = d_first;
			for (auto iter = first; iter != last; ++iter, (void) ++dest) {
				allocator_traits::construct(allocator, dest, *iter);
			}
		}

		template<std::input_iterator InputIterator, std::forward_iterator ForwardIterator, typename Allocator>
		constexpr void uninitialized_move(Allocator allocator, InputIterator first, InputIterator last, ForwardIterator d_first) {
			using allocator_traits = std::allocator_traits<Allocator>;
			auto dest = d_first;
			for (auto iter = first; iter != last; ++iter, (void) ++dest) {
				allocator_traits::construct(allocator, dest, std::move(*iter));
			}
		}*/

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<typename Range, typename Type>
		concept vector_compatible_range = std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, Type>;
#endif
	} // namespace detail

#if PLUGIFY_VECTOR_SMALL_BUFFER_OPTIMIZATION
	template<typename T, bool EnableSBO = false, std::size_t SBOPadding = 0, typename Allocator = std::allocator<T>>
	class small_vector {
		using allocator_traits = std::allocator_traits<Allocator>;
	public:
		using value_type = T;
		using allocator_type = Allocator;
		using size_type = typename allocator_traits::size_type;
		using difference_type = typename allocator_traits::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = typename allocator_traits::pointer;
		using const_pointer = typename allocator_traits::const_pointer;
		using iterator = vector_iterator<Allocator>;
		using const_iterator = vector_const_iterator<Allocator>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	private:
		constexpr static float growth_factor = 2.0f;// When resizing, what number to scale by

		_PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS
		allocator_type _allocator;

		_PLUGIFY_VECTOR_WARN_PUSH()

#if defined(__clang__)
		_PLUGIFY_VECTOR_WARN_IGNORE("-Wgnu-anonymous-struct")
		_PLUGIFY_VECTOR_WARN_IGNORE("-Wzero-length-array")
#elif defined(__GNUC__)
		_PLUGIFY_VECTOR_WARN_IGNORE("-Wpedantic")// this doesn't work
#elif defined(_MSC_VER)
		_PLUGIFY_VECTOR_WARN_IGNORE(4201)
		_PLUGIFY_VECTOR_WARN_IGNORE(4200)
#endif

		static constexpr int char_bit = std::numeric_limits<int8_t>::digits + std::numeric_limits<int8_t>::is_signed;
		static_assert(char_bit == 8, "assumes an 8 bit byte.");

		struct sbo_size {
			union {
				struct {
					size_type sbo_enabled : 1;
					size_type size : sizeof(size_type) * char_bit - 1;
				} big;
				struct {
					uint8_t sbo_enabled : 1;
					uint8_t size : 7;
					[[maybe_unused]] uint8_t padding[sizeof(size_type) - 1];
				} small;
			};

			sbo_size() noexcept {
				big.sbo_enabled = false;
				big.size = 0;
			}
		} _size = {};
		size_type _capacity = 0;
		union {
			pointer _data = nullptr;
			[[maybe_unused]] uint8_t _padding[SBOPadding + sizeof(pointer)];
		};

		_PLUGIFY_VECTOR_WARN_POP()

		static_assert(sizeof(sbo_size) == sizeof(size_type));
		static_assert(alignof(sbo_size) == alignof(size_type));

		constexpr void reallocate(size_type new_capacity) {
			reallocate(new_capacity, [](pointer) {});
		}

		constexpr void reallocate(size_type new_capacity, auto construct) {
			const auto old_size = size();
			const auto old_capacity = capacity();

			_PLUGIFY_VECTOR_ASSERT(new_capacity >= old_size, "plg::small_vector::reallocate(): resulted vector size would exceed size()", std::length_error);

			if (new_capacity == old_capacity) {
				return;
			}

			const bool can_use_sbo = sbo_max_objects() >= new_capacity;
			if (sbo_active() && can_use_sbo) {
				return;
			}

			// Allocate new memory
			pointer new_data = can_use_sbo ? sbo_data() : (new_capacity > 0 ? allocator_traits::allocate(_allocator, new_capacity) : nullptr);
			construct(new_data);

			// Move old objects
			auto* old_data = data();
			if (old_data && old_capacity > 0) {
				if constexpr (std::is_trivially_copyable_v<T>) {
					if (new_data) {
						std::memcpy(new_data, old_data, old_size * sizeof(T));
					}
				} else {
					for (size_type i = 0; i < old_size; ++i) {
						if (new_data) {
							allocator_traits::construct(_allocator, new_data + i, std::move(old_data[i]));
						}
						if constexpr (!std::is_trivially_destructible_v<T>) {
							allocator_traits::destroy(_allocator, old_data + i);
						}
					}
				}

				// Deallocate old memory
				if (!sbo_active()) {
					allocator_traits::deallocate(_allocator, old_data, old_capacity);
				}
			}

			if (sbo_active() != can_use_sbo) {
				if constexpr (sbo_enabled()) {
					_size.small.sbo_enabled = can_use_sbo;
				}
				set_size(old_size);
			}

			if (!can_use_sbo) {
				_data = new_data;
				_capacity = new_capacity;
			}
		}

		constexpr void ensure_capacity(size_type min_capacity) {
			if (capacity() < min_capacity) {
				reallocate(calculate_new_capacity(min_capacity));
			}
		}

		constexpr void construct_with_ensure_capacity(size_type min_capacity, auto construct) {
			if (capacity() < min_capacity) {
				reallocate(calculate_new_capacity(min_capacity), construct);
			} else {
				construct(data());
			}
		}

		constexpr void resize_down(size_type new_size) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				const auto old_size = size();
				_PLUGIFY_VECTOR_ASSERT(new_size <= old_size, "plg::small_vector::resize_down(): resulted vector size would exceed size()", std::length_error);
				auto* dst = data();
				for (size_type i = new_size; i < old_size; ++i) {
					allocator_traits::destroy(_allocator, dst + i);
				}
			}
			set_size(new_size);
		}

		constexpr void resize_to(size_type new_size) {
			const auto old_size = size();
			if (new_size > old_size) {
				if (new_size > capacity()) {
					reallocate(new_size);
				}
				set_size(new_size);
			} else if (new_size < old_size) {
				resize_down(new_size);
			}
		}

		constexpr void resize_to(size_type new_size, auto construct) {
			const auto old_size = size();
			if (new_size > old_size) {
				if (new_size > capacity()) {
					reallocate(new_size);
				}
				auto* dst = data();
				for (size_type i = old_size; i < new_size; ++i) {
					construct(dst + i);
				}
				set_size(new_size);
			} else if (new_size < old_size) {
				resize_down(new_size);
			}
		}

		constexpr iterator insert_at(const_iterator pos, auto func) {
			const auto old_size = size();
			const auto index = pos - begin();

			func(old_size);

			if (pos != end()) {
				const auto first = begin() + index;
				std::rotate(first, begin() + old_size, end());
				return first;
			} else {
				return begin() + old_size;
			}
		}

		template<std::input_iterator InputIterator>
		constexpr iterator insert_at(const_pointer pos, size_type count, InputIterator first, InputIterator last) {
			return insert_at(pos, [&](auto old_size) {
				const auto new_size = old_size + count;
				if constexpr (std::is_trivially_copyable_v<T>) {
					resize_to(new_size);
					std::copy(first, last, data() + old_size);
				} else {
					ensure_capacity(new_size);
					std::uninitialized_copy(first, last, data() + old_size);
					set_size(new_size);
				}
			});
		}

		constexpr void move_container(small_vector&& other) {
			if (other.sbo_active()) {
				// Using SBO, move elements
				_size = other._size;

				if constexpr (std::is_trivially_copyable_v<T>) {
					std::memcpy(data(), other.data(), other.size() * sizeof(T));
					other.set_size(0);
				} else {
					std::uninitialized_move(other.begin(), other.end(), data());
					other.clear();
				}
			} else {
				// No SBO, steal data
				_size = std::exchange(other._size, {});
				_capacity = std::exchange(other._capacity, 0);
				_data = std::exchange(other._data, nullptr);
			}
		}

		constexpr void move_elements(small_vector&& other) {
			const auto size = other.size();
			if constexpr (std::is_trivially_copyable_v<T>) {
				if (sbo_active() && other.sbo_active()) {
					std::memcpy(static_cast<void*>(this), &other, sizeof(*this));
				} else {
					resize_to(size);
					std::memcpy(data(), other.data(), size * sizeof(T));
				}
			} else {
				ensure_capacity(size);
				std::uninitialized_move(other.begin(), other.end(), data());
				set_size(size);
			}
		}

		template<std::input_iterator InputIterator>
		constexpr void copy_elements(InputIterator first, InputIterator last) {
			const auto size = static_cast<size_type>(std::distance(first, last));
			if constexpr (std::is_trivially_copyable_v<T>) {
				resize_to(size);
				std::copy(first, last, data());
			} else {
				ensure_capacity(size);
				std::uninitialized_copy(first, last, data());
				set_size(size);
			}
		}

		constexpr void copy_elements(const small_vector& other) {
			const auto size = other.size();
			if constexpr (std::is_trivially_copyable_v<T>) {
				if (sbo_active() && other.sbo_active()) {
					std::memcpy(static_cast<void*>(this), &other, sizeof(*this));
				} else {
					resize_to(size);
					std::memcpy(data(), other.data(), size * sizeof(T));
				}
			} else {
				ensure_capacity(size);
				std::uninitialized_copy(other.begin(), other.end(), data());
				set_size(size);
			}
		}

		constexpr void destroy() {
			clear();
			deallocate();
		}

		constexpr void deallocate() noexcept {
			if (!sbo_active() && _data && _capacity > 0) {
				allocator_traits::deallocate(_allocator, _data, _capacity);
				_data = nullptr;
				_capacity = 0;
			}
		}

		[[nodiscard]] constexpr reference elem(size_type pos) noexcept {
			return data()[pos];
		}

		[[nodiscard]] constexpr const_reference elem(size_type pos) const noexcept {
			return data()[pos];
		}

		[[nodiscard]] constexpr iterator de_const_iter(const_iterator iter) noexcept {
			return iterator(begin() + (iter - begin()));
		}

		[[nodiscard]] constexpr bool addr_in_range(const_pointer ptr) const noexcept {
			if (std::is_constant_evaluated()) {
				return false;
			} else {
				return data() <= ptr && ptr <= data() + size();
			}
		}

		constexpr void set_size(size_type sz) noexcept {
			if (sbo_active()) {
				_size.small.size = (sz & 0x7F);
			} else {
				_size.big.size = (sz & 0x7FFFFFFFFFFFFFFF);
			}
		}

		[[nodiscard]] constexpr size_type calculate_new_capacity(size_t min_capacity) const {
			return static_cast<size_type>(std::max(min_capacity, static_cast<size_type>(static_cast<float>(capacity()) * growth_factor)));
		}

		[[nodiscard]] constexpr pointer sbo_data() noexcept {
			return reinterpret_cast<pointer>(reinterpret_cast<char*>(this) + sbo_start_offset_bytes());
		}

		[[nodiscard]] constexpr const_pointer sbo_data() const noexcept {
			return reinterpret_cast<const_pointer>(reinterpret_cast<const char*>(this) + sbo_start_offset_bytes());
		}

		[[nodiscard]] constexpr static size_type sbo_start_offset_bytes() noexcept {
			return detail::align_up<size_type>(1ull, alignof(T));
		}

	public:
		constexpr small_vector() noexcept(std::is_nothrow_default_constructible<Allocator>::value) = default;

		explicit small_vector(const Allocator& alloc) noexcept
			: _allocator(alloc) {}

		constexpr small_vector(size_type count, const T& value, const Allocator& alloc = Allocator())
			: _allocator(alloc) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::small_vector::small_vector(): constructed vector size would exceed max_size()", std::length_error);
			resize_to(count, [&](pointer bytes) {
				allocator_traits::construct(_allocator, bytes, value);
			});
		}

		constexpr explicit small_vector(size_type count, const Allocator& alloc = Allocator())
			: _allocator(alloc) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::small_vector::small_vector(): constructed vector size would exceed max_size()", std::length_error);
			resize_to(count, [&](pointer bytes) {
				allocator_traits::construct(_allocator, bytes);
			});
		}

		template<std::input_iterator InputIterator>
		constexpr small_vector(InputIterator first, InputIterator last, const Allocator& alloc = Allocator())
			: _allocator(alloc) {
			_PLUGIFY_VECTOR_ASSERT(static_cast<size_type>(std::distance(first, last)) <= max_size(), "plg::small_vector::small_vector(): constructed vector size would exceed max_size()", std::length_error);
			copy_elements(first, last);
		}

		constexpr small_vector(const small_vector& other, const Allocator& alloc)
			: _allocator(alloc) {
			_PLUGIFY_VECTOR_ASSERT(other.size() <= max_size(), "plg::small_vector::small_vector(): constructed vector size would exceed max_size()", std::length_error);
			copy_elements(other);
		}

		constexpr small_vector(const small_vector& other)
			: small_vector(other, other.get_allocator()) {}

		constexpr small_vector(small_vector&& other) noexcept(std::is_nothrow_move_constructible<Allocator>::value)
			: _allocator(std::move(other._allocator)) {
			move_container(std::move(other));
		}

		constexpr small_vector(small_vector&& other, const Allocator& alloc)
			: _allocator(alloc) {
			if constexpr (allocator_traits::is_always_equal::value) {
				move_container(std::move(other));
			} else {
				if (get_allocator() == other.get_allocator()) {
					move_container(std::move(other));
				} else {
					move_elements(std::move(other));
				}
			}
		}

		constexpr small_vector(std::initializer_list<T> list, const Allocator& alloc = Allocator())
			: small_vector(list.begin(), list.end(), alloc) {}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<T> Range>
		constexpr small_vector(std::from_range_t, Range&& range, const Allocator& alloc = Allocator())
			: small_vector(std::ranges::begin(range), std::ranges::end(range), alloc) {}
#endif // PLUGIFY_VECTOR_CONTAINERS_RANGES

		constexpr ~small_vector() {
			destroy();
		}

		constexpr small_vector& operator=(const small_vector& other) {
			if (this == &other) [[unlikely]] {
				return *this;
			}

			clear();
			if constexpr (allocator_traits::propagate_on_container_copy_assignment::value) {
				if constexpr (!allocator_traits::is_always_equal::value) {
					if (get_allocator() != other.get_allocator()) {
						deallocate();
					}
				}
				_allocator = other._allocator;
			}
			copy_elements(other);
			return *this;
		}

		template<typename A, bool S, std::size_t P>
		constexpr small_vector& operator=(const small_vector<T, S, P, A>& other) {
			if (this == &other) [[unlikely]] {
				return *this;
			}

			clear();
			if constexpr (allocator_traits::propagate_on_container_copy_assignment::value) {
				if constexpr (!allocator_traits::is_always_equal::value) {
					if (get_allocator() != other.get_allocator()) {
						deallocate();
					}
				}
				_allocator = other._allocator;
			}
			if constexpr (std::is_same_v<decltype(this), decltype(&other)>) {
				copy_elements(other);
			} else {
				copy_elements(other.begin(), other.end());
			}
			return *this;
		}

		constexpr small_vector& operator=(small_vector&& other) noexcept(
				allocator_traits::propagate_on_container_move_assignment::value ||
				allocator_traits::is_always_equal::value) {
			if (this == &other) [[unlikely]] {
				return *this;
			}

			clear();
			if constexpr (allocator_traits::propagate_on_container_move_assignment::value) {
				if constexpr (!allocator_traits::is_always_equal::value) {
					if (get_allocator() != other.get_allocator()) {
						deallocate();
					}
				}
				_allocator = std::move(other._allocator);
			}

			if constexpr (allocator_traits::propagate_on_container_move_assignment::value || allocator_traits::is_always_equal::value) {
				deallocate();
				move_container(std::move(other));
			} else {
				if (get_allocator() == other.get_allocator()) {
					deallocate();
					move_container(std::move(other));
				} else {
					move_elements(std::move(other));
				}
			}
			return *this;
		}

		constexpr small_vector& operator=(std::initializer_list<T> list) {
			assign(list.begin(), list.end());
			return *this;
		}

		constexpr void assign(size_type count, const T& value) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::small_vector::assign(): resulted vector size would exceed max_size()", std::length_error);
			clear();
			resize_to(count, [&](pointer bytes) {
				allocator_traits::construct(_allocator, bytes, value);
			});
		}

		template<std::input_iterator InputIterator>
		constexpr void assign(InputIterator first, InputIterator last) {
			_PLUGIFY_VECTOR_ASSERT(static_cast<size_type>(std::distance(first, last)) <= max_size(), "plg::small_vector::assign(): resulted vector size would exceed max_size()", std::length_error);
			clear();
			copy_elements(first, last);
		}

		constexpr void assign(std::initializer_list<T> list) {
			assign(list.begin(), list.end());
		}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<T> Range>
		constexpr void assign_range(Range&& range) {
			assign(std::ranges::begin(range), std::ranges::end(range));
		}
#endif // PLUGIFY_VECTOR_CONTAINERS_RANGES

		[[nodiscard]] constexpr allocator_type get_allocator() const {
			return _allocator;
		}

		[[nodiscard]] constexpr pointer data() noexcept {
			return sbo_active() ? sbo_data() : _data;
		}

		[[nodiscard]] constexpr const_pointer data() const noexcept {
			return sbo_active() ? sbo_data() : _data;
		}

		[[nodiscard]] constexpr size_type size() const noexcept {
			return sbo_active() ? _size.small.size : _size.big.size;
		}

		[[nodiscard]] constexpr size_type max_size() const noexcept {
			return std::numeric_limits<size_type>::max() / 2;
		}

		[[nodiscard]] constexpr bool empty() const {
			return size() == 0;
		}

		[[nodiscard]] constexpr size_type capacity() const noexcept {
			return sbo_active() ? sbo_max_objects() : _capacity;
		}

		[[nodiscard]] constexpr reference operator[](size_type index) noexcept {
			return data()[index];
		}

		[[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept {
			return data()[index];
		}

		[[nodiscard]] constexpr reference at(size_type index) {
			_PLUGIFY_VECTOR_ASSERT(index < size(), "plg::small_vector::at(): input index is out of bounds", std::out_of_range);
			return data()[index];
		}

		[[nodiscard]] constexpr const_reference at(size_type index) const {
			_PLUGIFY_VECTOR_ASSERT(index < size(), "plg::small_vector::at(): input index is out of bounds", std::out_of_range);
			return data()[index];
		}

		[[nodiscard]] constexpr reference front() {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::small_vector::front(): vector is empty", std::length_error);
			return data()[0];
		}

		[[nodiscard]] constexpr reference back() {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::small_vector::back(): vector is empty", std::length_error);
			return data()[size() - 1];
		}

		[[nodiscard]] constexpr const_reference front() const {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::small_vector::front(): vector is empty", std::length_error);
			return data()[0];
		}

		[[nodiscard]] constexpr const_reference back() const {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::small_vector::back(): vector is empty", std::length_error);
			return data()[size() - 1];
		}

		constexpr void resize(size_type count) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::small_vector::resize(): allocated memory size would exceed max_size()", std::length_error);
			resize_to(count, [&](pointer bytes) {
				allocator_traits::construct(_allocator, bytes);
			});
		}

		constexpr void resize(size_type count, const T& value) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::small_vector::resize(): allocated memory size would exceed max_size()", std::length_error);
			resize_to(count, [&](pointer bytes) {
				allocator_traits::construct(_allocator, bytes, value);
			});
		}

		constexpr void reserve(size_type new_capacity) {
			_PLUGIFY_VECTOR_ASSERT(new_capacity <= max_size(), "plg::small_vector::reserve(): allocated memory size would exceed max_size()", std::length_error);
			if (new_capacity > capacity()) {
				reallocate(calculate_new_capacity(new_capacity));
			}
		}

		constexpr void shrink_to_fit() {
			reallocate(size());
		}

		constexpr void clear() {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				auto* dst = data();
				for (size_type i = 0; i < size(); ++i) {
					allocator_traits::destroy(_allocator, dst + i);
				}
			}
			set_size(0);
		}

		constexpr iterator insert(const_iterator pos, const T& value) {
			const auto sz = size();
			_PLUGIFY_VECTOR_ASSERT(sz + 1 <= max_size(), "plg::small_vector::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(sz), "plg::small_vector::insert(): pos out of range", std::out_of_range);
			return insert_at(pos, [&](auto) {
				push_back(value);
			});
		}

		constexpr iterator insert(const_iterator pos, T&& value) {
			const auto sz = size();
			_PLUGIFY_VECTOR_ASSERT(sz + 1 <= max_size(), "plg::small_vector::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(sz), "plg::small_vector::insert(): pos out of range", std::out_of_range);
			return insert_at(pos, [&](auto) {
				push_back(std::move(value));
			});
		}

		constexpr iterator insert(const_iterator pos, size_type count, const T& value) {
			const auto sz = size();
			_PLUGIFY_VECTOR_ASSERT(sz + count <= max_size(), "plg::small_vector::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(sz), "plg::small_vector::insert(): pos out of range", std::out_of_range);
			return insert_at(pos, [&](auto old_size) {
				resize(old_size + count, value);
			});
		}

		template<std::input_iterator InputIterator>
		constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
			if (first == last) [[unlikely]] {
				return de_const_iter(last);
			}

			const auto sz = size();
			const auto count = static_cast<size_type>(std::distance(first, last));
			_PLUGIFY_VECTOR_ASSERT(sz + count <= max_size(), "plg::small_vector::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(sz), "plg::small_vector::insert(): pos out of range", std::out_of_range);

			if (addr_in_range(first)) {
				small_vector vec(first, last);
				return insert_at(pos, count, std::make_move_iterator(vec.begin()), std::make_move_iterator(vec.end()));
			} else {
				return insert_at(pos, count, first, last);
			}
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<T> list) {
			return insert(pos, list.begin(), list.end());
		}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<T> Range>
		constexpr iterator insert_range(const_iterator pos, Range&& range) {
			return insert(pos - begin(), std::ranges::begin(range), std::ranges::end(range));
		}
#endif // PLUGIFY_VECTOR_CONTAINERS_RANGES

		template<typename... Args>
		constexpr iterator emplace(const_iterator pos, Args&&... args) {
			const auto sz = size();
			_PLUGIFY_VECTOR_ASSERT(sz + 1 <= max_size(), "plg::small_vector::emplace(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(sz), "plg::small_vector::emplace(): pos out of range", std::out_of_range);
			emplace_back(std::forward<Args>(args)...);
			const auto first = begin() + index;
			std::rotate(first, end() - 1, end());
			return first;
		}

		constexpr iterator erase(const_iterator first, const_iterator last) {
			if (first == last) [[unlikely]] {
				return de_const_iter(last);
			}

			const auto sz = size();
			const auto count = static_cast<size_type>(std::distance(first, last));
			_PLUGIFY_VECTOR_ASSERT(count >= 0 && count <= sz, "plg::small_vector::erase(): last out of range", std::out_of_range);
			const auto index = first - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(sz), "plg::small_vector::erase(): first out of range", std::out_of_range);
			std::rotate(de_const_iter(first), de_const_iter(last), end());
			resize_down(sz - count);
			return begin() + index;
		}

		constexpr iterator erase(const_iterator pos) {
			return erase(pos, pos + 1);
		}

		template<typename... Args>
		constexpr reference emplace_back(Args&&... args) {
			const auto sz = size();
			_PLUGIFY_VECTOR_ASSERT(sz + 1 <= max_size(), "plg::small_vector::emplace_back(): resulted vector size would exceed max_size()", std::length_error);
			construct_with_ensure_capacity(sz + 1, [&](pointer data) {
				allocator_traits::construct(_allocator, data + sz, std::forward<Args>(args)...);
			});
			set_size(sz + 1);
			return elem(sz);
		}

		constexpr void push_back(const T& value) {
			const auto sz = size();
			_PLUGIFY_VECTOR_ASSERT(sz + 1 <= max_size(), "plg::small_vector::push_back(): resulted vector size would exceed max_size()", std::length_error);
			construct_with_ensure_capacity(sz + 1, [&](pointer data) {
				allocator_traits::construct(_allocator, data + sz, value);
			});
			set_size(sz + 1);
		}

		constexpr void push_back(T&& value) {
			const auto sz = size();
			_PLUGIFY_VECTOR_ASSERT(sz + 1 <= max_size(), "plg::small_vector::push_back(): resulted vector size would exceed max_size()", std::length_error);
			construct_with_ensure_capacity(sz + 1, [&](pointer data) {
				allocator_traits::construct(_allocator, data + sz, std::move(value));
			});
			set_size(sz + 1);
		}

		constexpr void pop_back() {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::small_vector::pop_back(): vector is empty", std::length_error);
			if constexpr (!std::is_trivially_destructible_v<T>) {
				allocator_traits::destroy(_allocator, &back());
			}
			set_size(size() - 1);
		}

		constexpr small_vector& operator+=(const T& value) {
			push_back(value);
			return *this;
		}

		constexpr small_vector& operator+=(T&& value) {
			push_back(std::move(value));
			return *this;
		}

		constexpr small_vector& operator+=(const small_vector& other) {
			insert(end(), other.begin(), other.end());
			return *this;
		}

		constexpr small_vector& operator+=(small_vector&& other) {
			if (this == &other) [[unlikely]] {
				return *this;
			}

			insert(end(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
			return *this;
		}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<T> Range>
		constexpr void append_range(Range&& range) {
			return insert(end(), std::ranges::begin(range), std::ranges::end(range));
		}
#endif // PLUGIFY_VECTOR_CONTAINERS_RANGES

		constexpr void swap(small_vector& other) noexcept(allocator_traits::propagate_on_container_swap::value || allocator_traits::is_always_equal::value) {
			using std::swap;
			if constexpr (allocator_traits::propagate_on_container_swap::value) {
				swap(_allocator, other._allocator);
			}
			swap(_data, other._data);
			swap(_size, other._size);
			swap(_capacity, other._capacity);
		}

		[[nodiscard]] constexpr iterator begin() noexcept {
			return iterator(&elem(0));
		}

		[[nodiscard]] constexpr iterator end() noexcept {
			return iterator(&elem(size()));
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept {
			return const_iterator(&elem(0));
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept {
			return const_iterator(&elem(size()));
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept {
			return const_iterator(&elem(0));
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept {
			return const_iterator(&elem(size()));
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end())++;
		}

		[[nodiscard]] constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin())++;
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end())++;
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin())++;
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(end())++;
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(begin())++;
		}

		[[nodiscard]] constexpr static bool sbo_enabled() noexcept {
			return sbo_max_objects() > 0;
		}

		[[nodiscard]] constexpr bool sbo_active() const noexcept {
			if constexpr (sbo_enabled()) {
				return _size.small.sbo_enabled;
			} else {
				return false;
			}
		}

		[[nodiscard]] constexpr static size_type sbo_max_objects() noexcept {
			if constexpr (EnableSBO) {
				constexpr auto size_bytes = sizeof(small_vector);
				constexpr auto sbo_align_enabled = alignof(small_vector) >= alignof(T);
				constexpr auto first_sbo_offset = sbo_start_offset_bytes();
				constexpr auto result = sbo_align_enabled && first_sbo_offset < size_bytes ? (size_bytes - first_sbo_offset) / sizeof(T) : 0ull;
				static_assert(result <= 127);// More than 127 wouldn't fit in the 7-bit size field
				return static_cast<size_type>(result);
			} else {
				return 0;
			}
		}

		[[nodiscard]] constexpr std::span<const T> span() const noexcept {
			return std::span<const T>(data(), size());
		}

		[[nodiscard]] constexpr std::span<T> span() noexcept {
			return std::span<T>(data(), size());
		}

		[[nodiscard]] constexpr std::span<const T> const_span() const noexcept {
			return std::span<const T>(data(), size());
		}

		template<size_type Size>
		[[nodiscard]] constexpr std::span<T, Size> span_size() noexcept {
			_PLUGIFY_VECTOR_ASSERT(size() == Size, "plg::small_vector::span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<T, Size>(data(), size());
		}

		template<size_type Size>
		[[nodiscard]] constexpr std::span<const T, Size> const_span_size() const noexcept {
			_PLUGIFY_VECTOR_ASSERT(size() == Size, "plg::small_vector::const_span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<const T, Size>(data(), size());
		}

		[[nodiscard]] constexpr std::span<const std::byte> byte_span() const noexcept {
			return std::as_bytes(span());
		}

		[[nodiscard]] constexpr std::span<std::byte> byte_span() noexcept {
			return std::as_writable_bytes(span());
		}

		[[nodiscard]] constexpr std::span<const std::byte> const_byte_span() const noexcept {
			return std::as_bytes(span());
		}

		[[nodiscard]] constexpr bool contains(const T& elem) const {
			return std::find(begin(), end(), elem) != end();
		}

		template<typename F>
		[[nodiscard]] constexpr bool contains_if(F predicate) {
			return std::find_if(begin(), end(), predicate) != end();
		}

		[[nodiscard]] constexpr auto find(const T& value) const {
			return std::find(begin(), end(), value);
		}

		[[nodiscard]] constexpr auto find(const T& value) {
			return std::find(begin(), end(), value);
		}

		template<typename F>
		[[nodiscard]] constexpr auto find_if(F predicate) const {
			return std::find_if(begin(), end(), predicate);
		}

		template<typename F>
		[[nodiscard]] constexpr auto find_if(F predicate) {
			return std::find_if(begin(), end(), predicate);
		}

		[[nodiscard]] constexpr std::optional<size_type> find_index(const T& value) {
			const auto iter = std::find(begin(), end(), value);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		[[nodiscard]] constexpr std::optional<size_type> find_index(const T& value) const {
			const auto iter = std::find(begin(), end(), value);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		template<typename F>
		[[nodiscard]] constexpr std::optional<size_type> find_index_if(F predicate) {
			const auto iter = std::find_if(begin(), end(), predicate);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		template<typename F>
		[[nodiscard]] constexpr std::optional<size_type> find_index_if(F predicate) const {
			const auto iter = std::find_if(begin(), end(), predicate);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}
	};

	// comparisons
	template<typename T, bool EnableSBO, std::size_t SBOPadding, typename Allocator>
	[[nodiscard]] constexpr bool operator==(const small_vector<T, EnableSBO, SBOPadding, Allocator>& lhs, const small_vector<T, EnableSBO, SBOPadding, Allocator>& rhs) {
		return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	template<typename T, bool EnableSBO, std::size_t SBOPadding, typename Allocator>
	[[nodiscard]] constexpr auto operator<=>(const small_vector<T, EnableSBO, SBOPadding, Allocator>& lhs, const small_vector<T, EnableSBO, SBOPadding, Allocator>& rhs) {
		return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	// global swap for vector
	template<typename T, bool EnableSBO, std::size_t SBOPadding, typename Allocator>
	constexpr void swap(small_vector<T, EnableSBO, SBOPadding, Allocator>& lhs, small_vector<T, EnableSBO, SBOPadding, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
		lhs.swap(rhs);
	}

	template<typename T, bool EnableSBO, std::size_t SBOPadding, typename Allocator, typename U>
	constexpr typename small_vector<T, EnableSBO, SBOPadding, Allocator>::size_type erase(small_vector<T, EnableSBO, SBOPadding, Allocator>& c, const U& value) {
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	template<typename T, bool EnableSBO, std::size_t SBOPadding, typename Allocator, typename Pred>
	constexpr typename small_vector<T, EnableSBO, SBOPadding, Allocator>::size_type erase_if(small_vector<T, EnableSBO, SBOPadding, Allocator>& c, Pred pred) {
		auto it = std::remove_if(c.begin(), c.end(), pred);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	// Default versions
	template<typename T, typename Allocator = std::allocator<T>, int Padding = 0, bool EnableSBO = true>
	using vector = small_vector<T, EnableSBO, Padding, Allocator>;
#else
	// vector
	// based on implementations from libc++, libstdc++ and Microsoft STL
	template<typename T, typename Allocator = std::allocator<T>>
	class vector {
		using allocator_traits = std::allocator_traits<Allocator>;
	public:
		using value_type = T;
		using allocator_type = Allocator;
		using size_type = typename allocator_traits::size_type;
		using difference_type = typename allocator_traits::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = typename allocator_traits::pointer;
		using const_pointer = typename allocator_traits::const_pointer;
		using iterator = vector_iterator<Allocator>;
		using const_iterator = vector_const_iterator<Allocator>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	protected:
		_PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS
		allocator_type _allocator;
		pointer _begin;
		pointer _end;
		pointer _capacity;

	private:
		constexpr static size_type growth_factor = 2; // When resizing, what number to scale by

		constexpr void copy_constructor(const vector& other) {
			size_type capacity = other.capacity();
			_begin = allocator_traits::allocate(_allocator, capacity);
			std::uninitialized_copy(other.begin(), other.end(), begin());
			_end = _begin + other.size();
			_capacity = _begin + capacity;
		}

		template<std::input_iterator InputIterator>
		constexpr void range_constructor(InputIterator first, InputIterator last) {
			size_type count = static_cast<size_type>(std::distance(first, last));
			_begin = allocator_traits::allocate(_allocator, count);
			std::uninitialized_copy(first, last, _begin);
			_capacity = _begin + count;
			_end = _begin + count;
		}

		[[nodiscard]] constexpr bool is_full() const {
			return _end == _capacity;
		}

		[[nodiscard]] constexpr size_type calculate_new_capacity() const {
			size_type old_capacity = capacity();
			return old_capacity == 0 ? 1 : growth_factor * old_capacity;
		}

		[[nodiscard]] constexpr iterator de_const_iter(const_iterator iter) noexcept {
			return iterator(begin() + (iter - begin()));
		}

		[[nodiscard]] constexpr bool addr_in_range(const_pointer ptr) const noexcept {
			if (std::is_constant_evaluated()) {
				return false;
			} else {
				return data() <= ptr && ptr <= data() + size();
			}
		}

		constexpr void reallocate(size_type new_capacity) {
			pointer const new_begin = allocator_traits::allocate(_allocator, new_capacity);
			size_type old_size = size();
			std::uninitialized_move(_begin, _end, new_begin);
			std::destroy(_begin, _end);
			allocator_traits::deallocate(_allocator, _begin, capacity());
			_begin = new_begin;
			_end = _begin + old_size;
			_capacity = _begin + new_capacity;
		}

		constexpr void resize_down(size_type count) {
			std::destroy(_begin + count, _end);
			_end = _begin + count;
		}

		template<typename... Args>
		constexpr void resize_to(size_type count, Args&&... args) {
			if (count < size()) {
				resize_down(count);
			} else if (count > size()) {
				size_type old_size = size();
				if (count > capacity()) {
					reallocate(count);
				}
				detail::construct(_allocator, _begin + old_size, _begin + count, std::forward<Args>(args)...);
				_end = _begin + count;
			}
		}

		constexpr void swap_without_allocator(vector&& other) noexcept {
			using std::swap;
			swap(_begin, other._begin);
			swap(_end, other._end);
			swap(_capacity, other._capacity);
		}

	public:
		// constructor
		constexpr vector() noexcept(std::is_nothrow_default_constructible<Allocator>::value)
			: _allocator(Allocator()), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
		}

		constexpr explicit vector(const Allocator& allocator) noexcept
			: _allocator(allocator), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
		}

		constexpr vector(size_type count, const T& value, const Allocator& allocator = Allocator())
			: _allocator(allocator), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector::vector(): constructed vector size would exceed max_size()", std::length_error);
			_begin = allocator_traits::allocate(_allocator, count);
			std::uninitialized_fill_n(_begin, count, value);
			_capacity = _begin + count;
			_end = _begin + count;
		}

		constexpr explicit vector(size_type count, const Allocator& allocator = Allocator())
			: _allocator(allocator), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector::vector(): constructed vector size would exceed max_size()", std::length_error);
			_begin = allocator_traits::allocate(_allocator, count);
			std::uninitialized_value_construct_n(_begin, count);
			_capacity = _begin + count;
			_end = _begin + count;
		}

		template<std::input_iterator InputIterator>
		constexpr vector(InputIterator first, InputIterator last, const Allocator& allocator = Allocator())
			: _allocator(allocator), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
			_PLUGIFY_VECTOR_ASSERT(static_cast<size_type>(std::distance(first, last)) <= max_size(), "plg::vector::vector(): constructed vector size would exceed max_size()", std::length_error);
			range_constructor(first, last);
		}

		constexpr vector(const vector& other)
			: _allocator(other.get_allocator()), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
			copy_constructor(other);
		}

		constexpr vector(const vector& other, const Allocator& allocator)
			: _allocator(allocator), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
			copy_constructor(other);
		}

		constexpr vector(vector&& other) noexcept(std::is_nothrow_move_constructible<Allocator>::value)
			: _allocator(Allocator()), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
			swap(other);
		}

		constexpr vector(vector&& other, const Allocator& allocator)
			: _allocator(allocator), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
			if constexpr (allocator_traits::is_always_equal::value) {
				swap_without_allocator(std::move(other));
			} else {
				if (get_allocator() == other.get_allocator()) {
					swap_without_allocator(std::move(other));
				} else {
					size_type capacity = other.capacity();
					_begin = allocator_traits::allocate(_allocator, capacity);
					std::uninitialized_move(other.begin(), other.end(), begin());
					_end = _begin + other.size();
					_capacity = _begin + capacity;
				}
			}
		}

		constexpr vector(std::initializer_list<T> list, const Allocator& allocator = Allocator())
			: _allocator(allocator), _begin(nullptr), _end(nullptr), _capacity(nullptr) {
			_PLUGIFY_VECTOR_ASSERT(list.size() <= max_size(), "plg::vector::vector(): constructed vector size would exceed max_size()", std::length_error);
			range_constructor(list.begin(), list.end());
		}

		// destructor
		constexpr ~vector() {
			std::destroy(_begin, _end);
			allocator_traits::deallocate(_allocator, _begin, capacity());
		}

		// operator=
		constexpr vector& operator=(const vector& other) {
			if (this == &other) [[unlikely]] {
				return *this;
			}

			if constexpr (allocator_traits::propagate_on_container_copy_assignment::value) {
				if constexpr (!allocator_traits::is_always_equal::value) {
					if (get_allocator() != other.get_allocator()) {
						clear();
					}
				}
				_allocator = other.get_allocator();
			}

			size_type other_size = other.size();
			if (other_size > capacity()) {
				pointer const new_begin = allocator_traits::allocate(_allocator, other_size);
				std::uninitialized_copy(other.begin(), other.end(), new_begin);
				std::destroy(_begin, _end);
				allocator_traits::deallocate(_allocator, _begin, capacity());
				_begin = new_begin;
				_capacity = _begin + other_size;
			} else if (size() >= other_size) {
				std::copy(other.begin(), other.end(), begin());
				std::destroy(_begin + other_size, _end);
			} else {
				std::copy(other.begin(), other.begin() + size(), begin());
				std::uninitialized_copy(other.begin() + size(), other.end(), begin() + size());
			}
			_end = _begin + other_size;
			return *this;
		}

		constexpr vector& operator=(vector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value) {
			if (this == &other) [[unlikely]] {
				return *this;
			}

			if constexpr (allocator_traits::propagate_on_container_move_assignment::value || allocator_traits::is_always_equal::value) {
				swap(other);
			} else {
				if (get_allocator() == other.get_allocator()) {
					swap(other);
				} else {
					assign(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
					other.clear();
				}
			}
			return *this;
		}

		constexpr vector& operator=(std::initializer_list<T> list) {
			assign(list.begin(), list.end());
			return *this;
		}

		// assign
		constexpr void assign(size_type count, const T& value) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector::assign(): resulted vector size would exceed max_size()", std::length_error);
			if (count > capacity()) {
				pointer const new_begin = allocator_traits::allocate(_allocator, count);
				std::uninitialized_fill_n(new_begin, count, value);
				std::destroy(_begin, _end);
				allocator_traits::deallocate(_allocator, _begin, capacity());
				_begin = new_begin;
				_capacity = _begin + count;
			} else if (size() >= count) {
				std::fill_n(_begin, count, value);
				std::destroy(_begin + count, _end);
			} else {
				std::fill_n(_begin, size(), value);
				std::uninitialized_fill_n(_begin + size(), count - size(), value);
			}
			_end = _begin + count;
		}

		template<std::input_iterator InputIterator>
		constexpr void assign(InputIterator first, InputIterator last) {
			size_type count = static_cast<size_type>(std::distance(first, last));
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector::assign(): resulted vector size would exceed max_size()", std::length_error);
			if (count > capacity()) {
				pointer const new_begin = allocator_traits::allocate(_allocator, count);
				std::uninitialized_copy(first, last, new_begin);
				std::destroy(_begin, _end);
				allocator_traits::deallocate(_allocator, _begin, capacity());
				_begin = new_begin;
				_capacity = _begin + count;
			} else if (size() >= count) {
				std::copy(first, last, _begin);
				std::destroy(_begin + count, _end);
			} else {
				std::copy(first, first + size(), _begin);
				std::uninitialized_copy(first + size(), last, _begin + size());
			}
			_end = _begin + count;
		}

		constexpr void assign(std::initializer_list<T> list) {
			assign(list.begin(), list.end());
		}

		// get_allocator
		[[nodiscard]] constexpr allocator_type get_allocator() const {
			return _allocator;
		}

		// element access
		[[nodiscard]] constexpr reference at(size_type position) {
			_PLUGIFY_VECTOR_ASSERT(position < size(), "plg::vector::at(): input index is out of bounds", std::out_of_range);
			return *(_begin + position);
		}

		[[nodiscard]] constexpr const_reference at(size_type position) const {
			_PLUGIFY_VECTOR_ASSERT(position < size(), "plg::vector::at(): input index is out of bounds", std::out_of_range);
			return *(_begin + position);
		}

		[[nodiscard]] constexpr reference operator[](size_type position) noexcept {
			return *(_begin + position);
		}

		[[nodiscard]] constexpr const_reference operator[](size_type position) const noexcept {
			return *(_begin + position);
		}

		[[nodiscard]] constexpr reference front() {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::front(): vector is empty", std::length_error);
			return *_begin;
		}

		[[nodiscard]] constexpr const_reference front() const {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::front(): vector is empty", std::length_error);
			return *_begin;
		}

		[[nodiscard]] constexpr reference back() {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::back(): vector is empty", std::length_error);
			return *(_end - 1);
		}

		[[nodiscard]] constexpr const_reference back() const {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::back(): vector is empty", std::length_error);
			return *(_end - 1);
		}

		[[nodiscard]] constexpr T* data() noexcept {
			return _begin;
		}

		[[nodiscard]] constexpr const T* data() const noexcept {
			return _begin;
		}

		// iterators
		[[nodiscard]] constexpr iterator begin() noexcept {
			return iterator(_begin);
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept {
			return const_iterator(_begin);
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept {
			return begin();
		}

		[[nodiscard]] constexpr iterator end() noexcept {
			return iterator(_end);
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept {
			return const_iterator(_end);
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept {
			return end();
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(_end);
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(_end);
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}

		[[nodiscard]] constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(_begin);
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(_begin);
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
			return rend();
		}

		// capacity
		[[nodiscard]] constexpr bool empty() const {
			return (_begin == _end);
		}

		[[nodiscard]] constexpr size_type size() const noexcept {
			return static_cast<size_type>(_end - _begin);
		}

		[[nodiscard]] constexpr size_type max_size() const noexcept {
			return allocator_traits::max_size(_allocator);
		}

		constexpr void reserve(size_type new_capacity) {
			_PLUGIFY_VECTOR_ASSERT(new_capacity <= max_size(), "plg::vector::reserve(): allocated memory size would exceed max_size()", std::length_error);
			if (new_capacity > capacity()) {
				reallocate(new_capacity);
			}
		}

		[[nodiscard]] constexpr size_type capacity() const noexcept {
			return static_cast<size_type>(_capacity - _begin);
		}

		constexpr void shrink_to_fit() {
			reallocate(size());
		}

		// modifiers
		constexpr void clear() noexcept {
			std::destroy(_begin, _end);
			_end = _begin;
		}

		constexpr iterator insert(const_iterator position, const T& value) {
			return emplace(position, value);
		}

		constexpr iterator insert(const_iterator position, T&& value) {
			return emplace(position, std::move(value));
		}

		constexpr iterator insert(const_iterator position, size_type count, const T& value) {
			size_type sz = size();
			size_type new_size = sz + count;
			_PLUGIFY_VECTOR_ASSERT(new_size <= max_size(), "plg::vector::insert(): resulted vector size would exceed max_size()", std::length_error);
			size_type position_distance = static_cast<size_type>(position - cbegin());
			_PLUGIFY_VECTOR_ASSERT(position_distance >= 0 && position_distance <= sz, "plg::vector::insert(): pos out of range", std::out_of_range);
			if (count != 0) {
				if (new_size > capacity()) {
					pointer const new_begin = allocator_traits::allocate(_allocator, new_size);
					pointer const old_position = _begin + position_distance;
					std::uninitialized_move(_begin, old_position, new_begin);
					detail::construct(_allocator, new_begin + position_distance, new_begin + position_distance + count, value);
					std::uninitialized_move(old_position, _end, new_begin + position_distance + count);
					std::destroy(_begin, _end);
					allocator_traits::deallocate(_allocator, _begin, capacity());
					_begin = new_begin;
					_end = _begin + new_size;
					_capacity = _end;
				} else {
					pointer const pointer_position = _begin + position_distance;
					std::copy_backward(pointer_position, _end, _end + count);
					std::fill(pointer_position, pointer_position + count, value);
					_end += count;
				}
			}
			return begin() + position_distance;
		}

		template<std::input_iterator InputIterator>
		constexpr iterator insert(const_iterator position, InputIterator first, InputIterator last) {
			size_type sz = size();
			size_type count = static_cast<size_type>(std::distance(first, last));
			size_type new_size = sz + count;
			_PLUGIFY_VECTOR_ASSERT(new_size <= max_size(), "plg::small_vector::insert(): resulted vector size would exceed max_size()", std::length_error);
			size_type position_distance = static_cast<size_type>(position - cbegin());
			_PLUGIFY_VECTOR_ASSERT(position_distance >= 0 && position_distance <= sz, "plg::small_vector::insert(): pos out of range", std::out_of_range);
			if (count != 0) {
				if (new_size > capacity()) {
					pointer const new_begin = allocator_traits::allocate(_allocator, new_size);
					pointer const old_position = _begin + position_distance;
					pointer const new_position = new_begin + position_distance;
					std::uninitialized_move(_begin, old_position, new_begin);
					std::uninitialized_copy(first, last, new_position);
					std::uninitialized_move(old_position, _end, new_position + count);
					std::destroy(_begin, _end);
					allocator_traits::deallocate(_allocator, _begin, capacity());
					_begin = new_begin;
					_end = _begin + new_size;
					_capacity = _end;
				} else {
					pointer const pointer_position = _begin + position_distance;
					std::copy_backward(pointer_position, _end, _end + count);
					std::copy(first, last, pointer_position);
					_end += count;
				}
			}
			return begin() + position_distance;
		}

		constexpr iterator insert(const_iterator position, std::initializer_list<T> list) {
			return insert(position, list.begin(), list.end());
		}

		template<typename... Args>
		constexpr iterator emplace(const_iterator position, Args&&... args) {
			const auto sz = size();
			_PLUGIFY_VECTOR_ASSERT(sz + 1 <= max_size(), "plg::small_vector::emplace(): resulted vector size would exceed max_size()", std::length_error);
			size_type position_distance = static_cast<size_type>(position - cbegin());
			_PLUGIFY_VECTOR_ASSERT(position_distance >= 0 && position_distance <= sz, "plg::small_vector::emplace(): pos out of range", std::out_of_range);
			emplace_back(std::forward<Args>(args)...);
			const auto first = begin() + position_distance;
			std::rotate(first, end() - 1, end());
			return first;
		}

		constexpr iterator erase(const_iterator first, const_iterator last) {
			if (first == last) [[unlikely]] {
				return de_const_iter(last);
			}

			size_type sz = size();
			size_type count = static_cast<size_type>(std::distance(first, last));
			_PLUGIFY_VECTOR_ASSERT(count >= 0 && count <= sz, "plg::vector::erase(): last out of range", std::out_of_range);
			size_type position_distance = static_cast<size_type>(first - cbegin());
			_PLUGIFY_VECTOR_ASSERT(position_distance >= 0 && position_distance <= sz, "plg::vector::erase(): first out of range", std::out_of_range);
			std::rotate(de_const_iter(first), de_const_iter(last), end());
			resize_down(sz - count);
			return begin() + position_distance;
		}

		constexpr iterator erase(const_iterator pos) {
			return erase(pos, pos + 1);
		}

		constexpr void push_back(const T& value) {
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector::push_back(): resulted vector size would exceed max_size()", std::length_error);
			if (is_full()) {
				if (addr_in_range(&value)) {
					T tmp(value);
					push_back(std::move(tmp));
					return;
				}
				reallocate(calculate_new_capacity());
			}
			allocator_traits::construct(_allocator, _end, value);
			++_end;
		}

		constexpr void push_back(T&& value) {
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector::push_back(): resulted vector size would exceed max_size()", std::length_error);
			if (is_full()) {
				if (addr_in_range(&value)) {
					T tmp(std::move(value));
					push_back(std::move(tmp));
					return;
				}
				reallocate(calculate_new_capacity());
			}
			allocator_traits::construct(_allocator, _end, std::move(value));
			++_end;
		}

		template<typename... Args>
		constexpr reference emplace_back(Args&&... args) {
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector::emplace_back(): resulted vector size would exceed max_size()", std::length_error);
			if (is_full()) {
				reallocate(calculate_new_capacity());
			}
			allocator_traits::construct(_allocator, _end, std::forward<Args>(args)...);
			++_end;
			return back();
		}

		constexpr void pop_back() {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::pop_back(): vector is empty", std::length_error);
			--_end;
			std::destroy_at(_end);
		}

		constexpr void resize(size_type count) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector::resize(): allocated memory size would exceed max_size()", std::length_error);
			resize_to(count);
		}

		constexpr void resize(size_type count, const T& value) {
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector::resize(): allocated memory size would exceed max_size()", std::length_error);
			resize_to(count, value);
		}

		constexpr void swap(vector& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value || std::allocator_traits<Allocator>::is_always_equal::value) {
			using std::swap;
			if constexpr (allocator_traits::propagate_on_container_swap::value) {
				swap(_allocator, other._allocator);
			}
			swap(_begin, other._begin);
			swap(_end, other._end);
			swap(_capacity, other._capacity);
		}

		[[nodiscard]] constexpr std::span<const T> span() const noexcept {
			return std::span<const T>(data(), size());
		}

		[[nodiscard]] constexpr std::span<T> span() noexcept {
			return std::span<T>(data(), size());
		}

		[[nodiscard]] constexpr std::span<const T> const_span() const noexcept {
			return std::span<const T>(data(), size());
		}

		template<size_type Size>
		[[nodiscard]] constexpr std::span<T, Size> span_size() noexcept {
			_PLUGIFY_VECTOR_ASSERT(size() == Size, "plg::vector::span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<T, Size>(data(), size());
		}

		template<size_type Size>
		[[nodiscard]] constexpr std::span<const T, Size> const_span_size() const noexcept {
			_PLUGIFY_VECTOR_ASSERT(size() == Size, "plg::vector::const_span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<const T, Size>(data(), size());
		}

		[[nodiscard]] constexpr std::span<const std::byte> byte_span() const noexcept {
			return std::as_bytes(span());
		}

		[[nodiscard]] constexpr std::span<std::byte> byte_span() noexcept {
			return std::as_writable_bytes(span());
		}

		[[nodiscard]] constexpr std::span<const std::byte> const_byte_span() const noexcept {
			return std::as_bytes(span());
		}

		[[nodiscard]] constexpr bool contains(const T& elem) const {
			return std::find(begin(), end(), elem) != end();
		}

		template<typename F>
		[[nodiscard]] constexpr bool contains_if(F predicate) {
			return std::find_if(begin(), end(), predicate) != end();
		}

		[[nodiscard]] constexpr auto find(const T& value) const {
			return std::find(begin(), end(), value);
		}

		[[nodiscard]] constexpr auto find(const T& value) {
			return std::find(begin(), end(), value);
		}

		template<typename F>
		[[nodiscard]] constexpr auto find_if(F predicate) const {
			return std::find_if(begin(), end(), predicate);
		}

		template<typename F>
		[[nodiscard]] constexpr auto find_if(F predicate) {
			return std::find_if(begin(), end(), predicate);
		}

		[[nodiscard]] constexpr std::optional<size_type> find_index(const T& value) {
			const auto iter = std::find(begin(), end(), value);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		[[nodiscard]] constexpr std::optional<size_type> find_index(const T& value) const {
			const auto iter = std::find(begin(), end(), value);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		template<typename F>
		[[nodiscard]] constexpr std::optional<size_type> find_index_if(F predicate) {
			const auto iter = std::find_if(begin(), end(), predicate);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		template<typename F>
		[[nodiscard]] constexpr std::optional<size_type> find_index_if(F predicate) const {
			const auto iter = std::find_if(begin(), end(), predicate);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}
	};

	// comparisons
	template<typename T, typename Allocator>
	[[nodiscard]] constexpr bool operator==(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	template<typename T, typename Allocator>
	[[nodiscard]] constexpr auto operator<=>(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	// global swap for vector
	template<typename T, typename Allocator>
	constexpr void swap(vector<T, Allocator>& lhs, vector<T, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
		lhs.swap(rhs);
	}

	template<typename T, typename Allocator, typename U>
	constexpr typename vector<T, Allocator>::size_type erase(vector<T, Allocator>& c, const U& value) {
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	template<typename T, typename Allocator, typename Pred>
	constexpr typename vector<T, Allocator>::size_type erase_if(vector<T, Allocator>& c, Pred pred) {
		auto it = std::remove_if(c.begin(), c.end(), pred);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	// deduction guides
	template<typename InputIterator, typename Allocator = std::allocator<typename std::iterator_traits<InputIterator>::value_type>>
	vector(InputIterator, InputIterator, Allocator = Allocator()) -> vector<typename std::iterator_traits<InputIterator>::value_type, Allocator>;

#endif // PLUGIFY_VECTOR_SMALL_BUFFER_OPTIMIZATION

	namespace pmr {
		template<typename T>
		using vector = ::plg::vector<T, std::pmr::polymorphic_allocator<T>>;
	} // namespace pmr

} // namespace plg
