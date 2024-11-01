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
	namespace detail {
		template<class T, class = void>
		constexpr bool is_iterator_v = false;

		template<class T>
		constexpr bool is_iterator_v<T, std::void_t<typename std::iterator_traits<T>::iterator_category>> = true;

		template<typename T>
		using iterator_category_t = typename std::iterator_traits<T>::iterator_category;

		template<typename T>
		constexpr bool is_random_access_iterator_v = std::is_same_v<iterator_category_t<T>, std::random_access_iterator_tag>;

		template<typename T>
		[[nodiscard]] constexpr T align_up(T val, T align) { return val + (align - (val % align)) % align; }

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<typename Range, typename Type>
		concept vector_compatible_range = std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, Type>;
#endif
	} // namespace detail

	template<typename T, bool EnableSBO = false, size_t SBOPadding = 0, class Allocator = std::allocator<T>>
	class vector_base {
		// Purely to make notation easier
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
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr static float growth_factor = 2.0f; // When resizing, what number to scale by

		_PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS
		allocator_type _allocator;

		constexpr vector_base() noexcept(std::is_nothrow_default_constructible<allocator_type>::value)= default;

		explicit vector_base(const allocator_type& alloc) noexcept
			: _allocator(alloc)
		{}

		constexpr vector_base(size_t count, const value_type& value, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			resize(count, value);
		}

		constexpr explicit vector_base(size_t count, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			resize(count);
		}

		template<class InputIt> requires (detail::is_iterator_v<InputIt>)
		constexpr vector_base(InputIt first, InputIt last, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			const auto count = static_cast<size_type>(std::distance(first, last));
			_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector_base::vector_base(): constructed vector size would exceed max_size()", std::length_error);
			// RandomAccessIterator
			if constexpr (detail::is_random_access_iterator_v<InputIt> && std::is_trivially_copyable_v<value_type>) {
				resize_no_init(count);
				std::memcpy(data(), first, count * sizeof(value_type));
			} else {
				ensure_capacity(count);
				auto* dst = data();
				for (auto iter = first; iter != last; ++iter) {
					allocator_traits::construct(_allocator, dst, *iter);
					++dst;
				}
				set_size(static_cast<size_type>(count));
			}
		}

		_PLUGIFY_VECTOR_WARN_PUSH()

#if defined(__clang__) || defined(__GNUC__)
		_PLUGIFY_VECTOR_WARN_IGNORE("-Wclass-memaccess")
#endif

		constexpr vector_base(const vector_base& other, const allocator_type& alloc)
			: _allocator(alloc)
		{
			const auto size = other.get_size();
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::vector_base(): constructed vector size would exceed max_size()", std::length_error);
			if constexpr (std::is_trivially_copyable_v<value_type>) {
				if (sbo_active() && other.sbo_active()) {
					std::memcpy(this, &other, sizeof(*this));
				} else {
					resize_no_init(size);
					std::memcpy(data(), other.data(), size * sizeof(value_type));
				}
			} else {
				ensure_capacity(other.capacity());
				auto* dst = data();
				const auto iterEnd = other.end();
				for (auto iter = other.begin(); iter != iterEnd; ++iter) {
					allocator_traits::construct(_allocator, dst++, *iter);
				}
				set_size(size);
			}
		}

		_PLUGIFY_VECTOR_WARN_POP()

		constexpr vector_base(const vector_base& other)
			: vector_base(other, other.get_allocator())
		{}

		constexpr vector_base(vector_base&& other) noexcept(std::is_nothrow_move_constructible<allocator_type>::value)
			: _allocator(std::move(other._allocator))
		{
			move_data_from(std::move(other));
		}

		constexpr vector_base(vector_base&& other, const allocator_type& alloc)
			: _allocator(alloc)
		{
			move_data_from(std::move(other));
		}

		constexpr vector_base(std::initializer_list<value_type> list, const allocator_type& alloc = allocator_type())
			: vector_base(list.begin(), list.end(), alloc)
		{}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<T> Range>
		constexpr vector_base(std::from_range_t, Range&& range, const Allocator& alloc = allocator_type())
			: vector_base(std::ranges::begin(range), std::ranges::end(range), alloc)
		{}
#endif

		constexpr ~vector_base() noexcept
		{
			destroy();
		}

		_PLUGIFY_VECTOR_WARN_PUSH()

#if defined(__clang__) || defined(__GNUC__)
		_PLUGIFY_VECTOR_WARN_IGNORE("-Wclass-memaccess")
#endif

		constexpr vector_base& operator=(const vector_base& other)
		{
			if (this == &other) [[unlikely]] {
				return *this;
			}

			if constexpr (std::is_trivially_copyable_v<value_type>) {
				if (sbo_active() && other.sbo_active()) {
					std::memcpy(this, &other, sizeof(*this));
				} else {
					resize_no_init(other.size());
					std::memcpy(data(), other.data(), other.size() * sizeof(value_type));
				}
			} else {
				assign(other.begin(), other.end());
			}

			return *this;
		}

		_PLUGIFY_VECTOR_WARN_POP()

		template<typename A, bool SBO, size_t SBOP>
		constexpr vector_base& operator=(const vector_base<T, SBO, SBOP, A>& other)
		{
			if constexpr (std::is_same_v<decltype(this), decltype(&other)>) {
				if (this == &other) [[unlikely]] {
					return *this;
				}
			}

			if constexpr (std::is_trivially_copyable_v<value_type>) {
				resize_no_init(other.size());
				std::memcpy(data(), other.data(), other.size() * sizeof(value_type));
			} else {
				assign(other.begin(), other.end());
			}

			return *this;
		}

		constexpr vector_base& operator=(vector_base&& other) noexcept(
				allocator_traits::propagate_on_container_move_assignment::value ||
				allocator_traits::is_always_equal::value)
		{
			if (this == &other) [[unlikely]] {
				return *this;
			}

			destroy();
			_allocator = std::move(other._allocator);
			move_data_from(std::move(other));
			return *this;
		}

		constexpr vector_base& operator=(std::initializer_list<value_type> list)
		{
			assign(list.begin(), list.end());
			return *this;
		}

		constexpr void assign(size_t count, const value_type& value)
		{
			clear();
			resize(count, value); // replace resize for max_size()
		}

		template<class InputIt> requires (detail::is_iterator_v<InputIt>)
		constexpr void assign(InputIt first, InputIt last)
		{
			auto vec = vector_base(first, last, get_allocator());
			_PLUGIFY_VECTOR_ASSERT(vec.get_size() <= max_size(), "plg::vector_base::assign(): resulted vector size would exceed max_size()", std::length_error);
			destroy();
			move_data_from(std::move(vec));
		}

		constexpr void assign(std::initializer_list<value_type> list)
		{
			assign(list.begin(), list.end());
		}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<value_type> Range>
		constexpr void assign_range(Range&& range)
		{
			assign(std::ranges::begin(range), std::ranges::end(range));
		}
#endif

		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept
		{
			return _allocator;
		}

		[[nodiscard]] constexpr pointer data() noexcept
		{
			return sbo_active() ? sbo_data() : _data;
		}

		[[nodiscard]] constexpr const_pointer data() const noexcept
		{
			return sbo_active() ? sbo_data() : _data;
		}

		[[nodiscard]] constexpr size_t size() const noexcept
		{
			return get_size();
		}

		[[nodiscard]] constexpr size_t max_size() const noexcept
		{
			return std::numeric_limits<size_type>::max() / 2;
		}

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return get_size() == 0;
		}

		[[nodiscard]] constexpr size_t capacity() const noexcept
		{
			return get_capacity();
		}

		[[nodiscard]] constexpr reference operator[](size_t index) noexcept
		{
			return data()[index];
		}

		[[nodiscard]] constexpr const_reference operator[](size_t index) const noexcept
		{
			return data()[index];
		}

		[[nodiscard]] constexpr reference at(size_t index)
		{
			_PLUGIFY_VECTOR_ASSERT(index < get_size(), "plg::vector_base::at(): input index is out of bounds", std::out_of_range);
			return data()[index];
		}

		[[nodiscard]] constexpr const_reference at(size_t index) const
		{
			_PLUGIFY_VECTOR_ASSERT(index < get_size(), "plg::vector_base::at(): input index is out of bounds", std::out_of_range);
			return data()[index];
		}

		[[nodiscard]] constexpr reference front()
		{
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector_base::front(): vector is empty", std::length_error);
			return data()[0];
		}

		[[nodiscard]] constexpr reference back()
		{
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector_base::back(): vector is empty", std::length_error);
			return data()[size() - 1];
		}

		[[nodiscard]] constexpr const_reference front() const
		{
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector_base::front(): vector is empty", std::length_error);
			return data()[0];
		}

		[[nodiscard]] constexpr const_reference back() const
		{
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector_base::back(): vector is empty", std::length_error);
			return data()[size() - 1];
		}

		constexpr void resize(size_t size)
		{
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::resize(): allocated memory size would exceed max_size()", std::length_error);
			internal_resize(size, [&](pointer bytes) {
				allocator_traits::construct(_allocator, bytes);
			});
		}

		constexpr void resize_no_init(size_t size)
		{
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::resize(): allocated memory size would exceed max_size()", std::length_error);
			internal_resize(size);
		}

		constexpr void resize(size_t size, const value_type& value)
		{
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::resize(): allocated memory size would exceed max_size()", std::length_error);
			internal_resize(size, [&](pointer bytes) {
				allocator_traits::construct(_allocator, bytes, value);
			});
		}

		constexpr void reserve(size_t size)
		{
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::reserve(): allocated memory size would exceed max_size()", std::length_error);
			if (size > capacity()) {
				change_capacity(static_cast<size_type>(std::max(size, static_cast<size_t>(static_cast<float>(capacity()) * growth_factor))));
			}
		}

		constexpr void shrink_to_fit()
		{
			change_capacity(get_size());
		}

		constexpr void clear() noexcept
		{
			for (size_type i = 0; i < get_size(); ++i) {
				allocator_traits::destroy(_allocator, data() + i);
			}
			set_size(0);
		}

		constexpr iterator insert(const_iterator pos, const value_type& value)
		{
			_PLUGIFY_VECTOR_ASSERT(get_size() + 1 <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(get_size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return internal_insert(pos, [&](size_t) {
				push_back(value);
			});
		}

		constexpr iterator insert(const_iterator pos, value_type&& value)
		{
			_PLUGIFY_VECTOR_ASSERT(get_size() + 1 <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(get_size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return internal_insert(pos, [&](size_t) {
				push_back(std::move(value));
			});
		}

		constexpr iterator insert(const_iterator pos, size_t count, const value_type& value)
		{
			_PLUGIFY_VECTOR_ASSERT(get_size() + count <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(get_size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return internal_insert(pos, [&](size_t prevSize) {
				resize(prevSize + count, value);
			});
		}

		template<class InputIt> requires (detail::is_iterator_v<InputIt>)
		constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
		{
			if (first == last) [[unlikely]] {
				return de_const_iter(last);
			}

			if (addr_in_range(const_pointer(first))) {
				auto vec = vector_base(first, last, get_allocator());
				return insert(pos, vec.begin(), vec.end());
			}

			const auto count = std::distance(first, last);
			_PLUGIFY_VECTOR_ASSERT(get_size() + count <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(get_size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return internal_insert(pos, [&](size_t prevSize) {
				const auto new_size = prevSize + static_cast<size_type>(count);
				if constexpr (detail::is_random_access_iterator_v<InputIt> && std::is_trivially_copyable_v<value_type>) {
					resize_no_init(new_size);
					std::memcpy(data() + prevSize, first, count * sizeof(value_type));
				} else {
					ensure_capacity(new_size);
					auto* dst = data() + prevSize;
					for (auto iter = first; iter != last; ++iter) {
						allocator_traits::construct(_allocator, dst, *iter);
						++dst;
					}
					set_size(new_size);
				}
			});
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> list)
		{
			_PLUGIFY_VECTOR_ASSERT(get_size() + list.size() <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(get_size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return insert(pos, list.begin(), list.end());
		}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<value_type> Range>
		constexpr iterator insert_range(const_iterator pos, Range&& range)
		{
			return insert(pos - begin(), std::ranges::begin(range), std::ranges::end(range));
		}
#endif

		template<class... Args>
		constexpr iterator emplace(const_iterator pos, Args&&... args)
		{
			_PLUGIFY_VECTOR_ASSERT(get_size() + 1 <= max_size(), "plg::vector_base::emplace(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(get_size()), "plg::vector_base::emplace(): pos out of range", std::out_of_range);
			emplace_back(std::forward<Args>(args)...);
			std::rotate(begin() + index, end() - 1, end());
			return begin() + index;
		}

		constexpr iterator erase(const_iterator first, const_iterator last)
		{
			if (first == last) [[unlikely]] {
				return de_const_iter(last);
			}

			const auto count = std::distance(first, last);
			_PLUGIFY_VECTOR_ASSERT(count >= 0 && count <= static_cast<difference_type>(get_size()), "plg::vector_base::erase(): last out of range", std::out_of_range);
			const auto index = first - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(get_size()), "plg::vector_base::erase(): first out of range", std::out_of_range);
			std::rotate(de_const_iter(first), de_const_iter(last), end());
			resize_down(static_cast<size_type>(get_size() - count));
			return begin() + index;
		}

		constexpr iterator erase(const_iterator pos)
		{
			return erase(pos, pos + 1);
		}

		template<class... Args>
		constexpr reference emplace_back(Args&&... args)
		{
			const auto size = get_size();
			_PLUGIFY_VECTOR_ASSERT(size + 1 <= max_size(), "plg::vector_base::emplace_back(): resulted vector size would exceed max_size()", std::length_error);
			construct_with_ensure_capacity(size + 1, [&] (pointer data) {
				allocator_traits::construct(_allocator, data + size, std::forward<Args>(args)...);
			});
			set_size(size + 1);
			return elem(size);
		}

		constexpr void push_back(const value_type& value)
		{
			const auto size = get_size();
			_PLUGIFY_VECTOR_ASSERT(size + 1 <= max_size(), "plg::vector_base::push_back(): resulted vector size would exceed max_size()", std::length_error);
			construct_with_ensure_capacity(size + 1, [&](pointer data)  {
				allocator_traits::construct(_allocator, data + size, value);
			});
			set_size(size + 1);
		}

		constexpr void push_back(value_type&& value)
		{
			const auto size = get_size();
			_PLUGIFY_VECTOR_ASSERT(size + 1 <= max_size(), "plg::vector_base::push_back(): resulted vector size would exceed max_size()", std::length_error);
			construct_with_ensure_capacity(size + 1, [&](pointer data) {
				allocator_traits::construct(_allocator, data + size, std::move(value));
			});
			set_size(size + 1);
		}

		constexpr void pop_back()
		{
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector_base::pop_back(): vector is empty", std::length_error);
			allocator_traits::destroy(_allocator, &back());
			set_size(get_size() - 1);
		}

		constexpr vector_base& operator+=(const value_type& value)
		{
			push_back(value);
			return *this;
		}

		constexpr vector_base& operator+=(const vector_base& other)
		{
			insert(end(), other.begin(), other.end());
			return *this;
		}

		constexpr vector_base& operator+=(vector_base&& other)
		{
			reserve(size() + other.size());
			for (auto&& o : other) {
				push_back(std::move(o));
			}
			return *this;
		}

		constexpr vector_base& operator+=(value_type&& value)
		{
			push_back(std::move(value));
			return *this;
		}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<value_type> Range>
		constexpr void append_range(Range&& range)
		{
			return insert(end(), std::ranges::begin(range), std::ranges::end(range));
		}
#endif

		constexpr void swap(vector_base& other) noexcept(allocator_traits::propagate_on_container_swap::value || allocator_traits::is_always_equal::value)
		{
			using std::swap;
			if constexpr (allocator_traits::propagate_on_container_swap::value) {
				swap(_allocator, other._allocator);
			}
			swap(_data, other._data);
			swap(_size, other._size);
			swap(_capacity, other._capacity);
		}

		[[nodiscard]] constexpr iterator begin() noexcept
		{
			return iterator(&elem(0));
		}

		[[nodiscard]] constexpr iterator end() noexcept
		{
			return iterator(&elem(size()));
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			return const_iterator(&elem(0));
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			return const_iterator(&elem(size()));
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept
		{
			return const_iterator(&elem(0));
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept
		{
			return const_iterator(&elem(size()));
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end())++;
		}

		[[nodiscard]] constexpr reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin())++;
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end())++;
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin())++;
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(end())++;
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(begin())++;
		}

		[[nodiscard]] constexpr static bool sbo_enabled() noexcept
		{
			return sbo_max_objects() > 0;
		}

		[[nodiscard]] constexpr bool sbo_active() const noexcept
		{
			if constexpr (sbo_enabled()) {
				return _size.small.sbo_enabled;
			} else {
				return false;
			}
		}

		[[nodiscard]] constexpr static size_type sbo_max_objects() noexcept
		{
			if constexpr (EnableSBO) {
				constexpr auto size_bytes = sizeof(vector_base);
				constexpr auto sbo_align_enabled = alignof(vector_base) >= alignof(value_type);
				constexpr auto first_sbo_offset = sbo_start_offset_bytes();
				constexpr auto result = sbo_align_enabled && first_sbo_offset < size_bytes ? (size_bytes - first_sbo_offset) / sizeof(value_type) : 0ull;
				static_assert(result <= 127); // More than 127 wouldn't fit in the 7-bit size field
				return static_cast<size_type>(result);
			} else {
				return 0;
			}
		}

		[[nodiscard]] constexpr std::span<const value_type> span() const noexcept
		{
			return std::span<const value_type>(data(), size());
		}

		[[nodiscard]] constexpr std::span<value_type> span() noexcept
		{
			return std::span<value_type>(data(), size());
		}

		[[nodiscard]] constexpr std::span<const value_type> const_span() const noexcept
		{
			return std::span<const value_type>(data(), size());
		}

		template<size_t Size>
		[[nodiscard]] constexpr std::span<value_type, Size> span_size() noexcept
		{
			_PLUGIFY_VECTOR_ASSERT(size() == Size, "plg::vector_base::span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<value_type, Size>(data(), size());
		}

		template<size_t Size>
		[[nodiscard]] constexpr std::span<const value_type, Size> const_span_size() const noexcept
		{
			_PLUGIFY_VECTOR_ASSERT(size() == Size, "plg::vector_base::const_span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<const value_type, Size>(data(), size());
		}

		[[nodiscard]] constexpr std::span<const std::byte> byte_span() const noexcept
		{
			return std::as_bytes(span());
		}

		[[nodiscard]] constexpr std::span<std::byte> byte_span() noexcept
		{
			return std::as_writable_bytes(span());
		}

		[[nodiscard]] constexpr std::span<const std::byte> const_byte_span() const noexcept
		{
			return std::as_bytes(span());
		}

		[[nodiscard]] constexpr bool contains(const T& elem) const
		{
			return std::find(begin(), end(), elem) != end();
		}

		template<typename F>
		[[nodiscard]] constexpr bool contains_if(F predicate)
		{
			return std::find_if(begin(), end(), predicate) != end();
		}

		[[nodiscard]] constexpr auto find(const T& value) const
		{
			return std::find(begin(), end(), value);
		}

		[[nodiscard]] constexpr auto find(const T& value)
		{
			return std::find(begin(), end(), value);
		}

		template<typename F>
		[[nodiscard]] constexpr auto find_if(F predicate) const
		{
			return std::find_if(begin(), end(), predicate);
		}

		template<typename F>
		[[nodiscard]] constexpr auto find_if(F predicate)
		{
			return std::find_if(begin(), end(), predicate);
		}

		[[nodiscard]] constexpr std::optional<size_t> find_index(const T& value)
		{
			const auto iter = std::find(begin(), end(), value);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		[[nodiscard]] constexpr std::optional<size_t> find_index(const T& value) const
		{
			const auto iter = std::find(begin(), end(), value);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		template<typename F>
		[[nodiscard]] constexpr std::optional<size_t> find_index_if(F predicate)
		{
			const auto iter = std::find_if(begin(), end(), predicate);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		template<typename F>
		[[nodiscard]] constexpr std::optional<size_t> find_index_if(F predicate) const
		{
			const auto iter = std::find_if(begin(), end(), predicate);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

	private:
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
		} _size;
		size_type _capacity = 0;
		union {
			pointer _data = nullptr;
			[[maybe_unused]] uint8_t _padding[SBOPadding + sizeof(pointer)];
		};

		_PLUGIFY_VECTOR_WARN_POP()

		static_assert(sizeof(sbo_size) == sizeof(size_type));
		static_assert(alignof(sbo_size) == alignof(size_type));

		constexpr void change_capacity(size_type new_capacity)
		{
			change_capacity(new_capacity, [](pointer) {});
		}

		constexpr void change_capacity(size_type new_capacity, auto construct)
		{
			const auto size = get_size();
			const auto capacity = get_capacity();
			_PLUGIFY_VECTOR_ASSERT(new_capacity >= size, "plg::vector_base::change_capacity(): resulted vector size would exceed size()", std::length_error);
			if (new_capacity != capacity) {
				const bool canUseSBO = sbo_max_objects() >= new_capacity;
				if (sbo_active() && canUseSBO) {
					return;
				}

				// Allocate new memory
				pointer new_data = canUseSBO ? sbo_data() : (new_capacity > 0 ? allocator_traits::allocate(_allocator, new_capacity) : nullptr);
				construct(new_data);

				// Move old objects
				auto* old_data = data();
				if constexpr (std::is_trivially_copyable_v<value_type>) {
					if (size > 0) {
						std::memcpy(new_data, old_data, size * sizeof(value_type));
					}
				} else {
					for (size_type i = 0; i < size; ++i) {
						allocator_traits::construct(_allocator, new_data + i, std::move(old_data[i]));
						allocator_traits::destroy(_allocator, old_data + i);
					}
				}

				// Deallocate old memory
				if (!sbo_active() && old_data && capacity > 0) {
					allocator_traits::deallocate(_allocator, old_data, capacity);
				}

				if (sbo_active() != canUseSBO) {
					if constexpr (sbo_enabled()) {
						_size.small.sbo_enabled = canUseSBO;
					}
					set_size(size);
				}

				if (!canUseSBO) {
					_data = new_data;
					_capacity = new_capacity;
				}
			}
		}

		constexpr void internal_resize(size_t size)
		{
			const auto new_size = static_cast<size_type>(size);
			if (new_size > get_size()) {
				if (new_size > capacity()) {
					change_capacity(new_size);
				}
				set_size(new_size);
			} else if (new_size < get_size()) {
				resize_down(new_size);
			}
		}

		constexpr void internal_resize(size_t size, auto construct)
		{
			const auto new_size = static_cast<size_type>(size);
			if (new_size > get_size()) {
				if (new_size > capacity()) {
					change_capacity(new_size);
				}
				auto* d = data();
				for (size_type i = get_size(); i < new_size; ++i) {
					construct(d + i);
				}
				set_size(new_size);
			} else if (new_size < get_size()) {
				resize_down(new_size);
			}
		}

		constexpr void resize_down(size_type new_size)
		{
			_PLUGIFY_VECTOR_ASSERT(new_size <= get_size(), "plg::vector_base::resize_down(): resulted vector size would exceed size()", std::length_error);
			auto* d = data();
			for (size_type i = new_size; i < get_size(); ++i) {
				allocator_traits::destroy(_allocator, d + i);
			}
			set_size(new_size);
		}

		constexpr void ensure_capacity(size_type min_capacity)
		{
			if (capacity() < min_capacity) {
				change_capacity(std::max(min_capacity, static_cast<size_type>(static_cast<float>(capacity()) * growth_factor)));
			}
		}

		constexpr void construct_with_ensure_capacity(size_type min_capacity, auto construct)
		{
			if (capacity() < min_capacity) {
				change_capacity(std::max(min_capacity, static_cast<size_type>(static_cast<float>(capacity()) * growth_factor)), construct);
			} else {
				construct(data());
			}
		}

		constexpr iterator internal_insert(const_iterator pos, auto func)
		{
			const auto old_size = size();
			const auto index = pos - begin();

			func(old_size);

			if (pos != end()) {
				std::rotate(begin() + index, begin() + old_size, end());
				return begin() + index;
			} else {
				return begin() + old_size;
			}
		}

		constexpr void move_data_from(vector_base&& other)
		{
			if (other.sbo_active()) {
				// Using SBO, move elements
				_size = other._size;

				if constexpr (std::is_trivially_copyable_v<value_type>) {
					std::memcpy(data(), other.data(), size() * sizeof(value_type));
					other.set_size(0);
				} else {
					auto* dst = data();
					auto iter = other.begin();
					const auto iterEnd = other.end();
					for (; iter != iterEnd; ++iter) {
						allocator_traits::construct(_allocator, dst++, std::move(*iter));
					}
					other.clear();
				}
			} else {
				// No SBO, steal data
				_size = other._size;
				_capacity = other._capacity;
				_data = other._data;

				other._data = nullptr;
				other._size = {};
				other._capacity = 0;
			}
		}

		constexpr bool addr_in_range(const_pointer ptr) const
		{
			if (std::is_constant_evaluated())
				return false;
			return data() <= ptr && ptr <= data() + size();
		}

		constexpr void destroy()
		{
			clear();
			change_capacity(0);
		}

		[[nodiscard]] constexpr reference elem(size_t pos) noexcept
		{
			return data()[pos];
		}

		[[nodiscard]] constexpr const_reference elem(size_t pos) const noexcept
		{
			return data()[pos];
		}

		[[nodiscard]] constexpr iterator de_const_iter(const_iterator iter) noexcept
		{
			return iterator(begin() + (iter - begin()));
		}

		constexpr void set_size(size_type sz)
		{
			if (sbo_active()) {
				_size.small.size = (sz & 0x7F);
			} else {
				_size.big.size = (sz & 0x7FFFFFFFFFFFFFFF);
			}
		}

		[[nodiscard]] constexpr size_type get_size() const noexcept
		{
			return sbo_active() ? _size.small.size : _size.big.size;
		}

		[[nodiscard]] constexpr size_type get_capacity() const noexcept
		{
			return sbo_active() ? sbo_max_objects() : _capacity;
		}

		[[nodiscard]] constexpr pointer sbo_data() noexcept
		{
			return reinterpret_cast<pointer>(reinterpret_cast<char*>(this) + sbo_start_offset_bytes());
		}

		[[nodiscard]] constexpr const_pointer sbo_data() const noexcept
		{
			return reinterpret_cast<const_pointer>(reinterpret_cast<const char*>(this) + sbo_start_offset_bytes());
		}

		[[nodiscard]] constexpr static std::size_t sbo_start_offset_bytes() noexcept
		{
			return detail::align_up<size_t>(1ull, alignof(value_type));
		}
	};

	// comparisons
	template<typename T, bool EnableSBO, size_t SBOPadding, class Allocator>
	[[nodiscard]] constexpr bool operator==(const vector_base<T, EnableSBO, SBOPadding, Allocator>& lhs, const vector_base<T, EnableSBO, SBOPadding, Allocator>& rhs)
	{
		return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	template<typename T, bool EnableSBO, size_t SBOPadding, class Allocator>
	[[nodiscard]] constexpr auto operator<=>(const vector_base<T, EnableSBO, SBOPadding, Allocator>& lhs, const vector_base<T, EnableSBO, SBOPadding, Allocator>& rhs)
	{
		return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	// global swap for vector
	template<typename T, bool EnableSBO, size_t SBOPadding, class Allocator>
	constexpr void swap(vector_base<T, EnableSBO, SBOPadding, Allocator>& lhs, vector_base<T, EnableSBO, SBOPadding, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs)))
	{
		lhs.swap(rhs);
	}

	template<typename T, bool EnableSBO, size_t SBOPadding, class Allocator, typename U>
	constexpr typename vector_base<T, EnableSBO, SBOPadding, Allocator>::size_type erase(vector_base<T, EnableSBO, SBOPadding, Allocator>& c, const U& value) {
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	template<typename T, bool EnableSBO, size_t SBOPadding, class Allocator, typename Pred>
	constexpr typename vector_base<T, EnableSBO, SBOPadding, Allocator>::size_type erase_if(vector_base<T, EnableSBO, SBOPadding, Allocator>& c, Pred pred) {
		auto it = std::remove_if(c.begin(), c.end(), pred);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	// Default versions
	template<typename T, typename Allocator = std::allocator<T>, int Padding = 0, bool EnableSBO = true>
	using vector = vector_base<T, EnableSBO, Padding, Allocator>;

	namespace pmr {
		template<typename T>
		using vector = ::plg::vector<T, std::pmr::polymorphic_allocator<T>>;
	} // namespace pmr

} // namespace plg
