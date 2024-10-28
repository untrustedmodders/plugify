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
#  define _PLUGIFY_VECTOR_DIAG_PUSH() _PLUGIFY_VECTOR_PRAGMA(_PLUGIFY_VECTOR_PRAGMA_DIAG_PREFIX diagnostic push)
#  define _PLUGIFY_VECTOR_DIAG_IGN(wrn) _PLUGIFY_VECTOR_PRAGMA(_PLUGIFY_VECTOR_PRAGMA_DIAG_PREFIX diagnostic ignored wrn)
#  define _PLUGIFY_VECTOR_DIAG_POP() _PLUGIFY_VECTOR_PRAGMA(_PLUGIFY_VECTOR_PRAGMA_DIAG_PREFIX diagnostic pop)
#elif defined(_MSC_VER)
#  define _PLUGIFY_VECTOR_DIAG_PUSH()	__pragma(warning(push))
#  define _PLUGIFY_VECTOR_DIAG_IGN(wrn) __pragma(warning(disable: wrn))
#  define _PLUGIFY_VECTOR_DIAG_POP() __pragma(warning(pop))
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
		[[nodiscard]] constexpr T align_up(T val, T align) { return val + (align - (val % align)) % align; }
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
		
		constexpr vector_base() noexcept(std::is_nothrow_default_constructible<allocator_type>::value) = default;

		explicit vector_base(const allocator_type& alloc) noexcept
			: _allocator(alloc)
		{
		}

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
			assign(std::move(first), std::move(last));
		}

		_PLUGIFY_VECTOR_DIAG_PUSH()

#if defined(__clang__) || defined(__GNUC__)
		_PLUGIFY_VECTOR_DIAG_IGN("-Wclass-memaccess")
#endif

		constexpr vector_base(const vector_base& other)
		{
			const auto size = other.st_size();
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::vector_base(): constructed vector size would exceed max_size()", std::length_error);
			if constexpr (std::is_trivially_copyable_v<T>) {
				if (sbo_active() && other.sbo_active()) {
					std::memcpy(this, &other, sizeof(*this));
				} else {
					resize_no_init(size);
					std::memcpy(data(), other.data(), size * sizeof(T));
				}
			} else {
				change_capacity(other.st_capacity());
				auto* dst = data();
				const auto iterEnd = other.end();
				for (auto iter = other.begin(); iter != iterEnd; ++iter) {
					allocator_traits::construct(_allocator, dst++, *iter);
				}
				set_size(size);
			}
		}

		_PLUGIFY_VECTOR_DIAG_POP()

		constexpr vector_base(const vector_base& other, const allocator_type& alloc)
			: _allocator(alloc)
		{
			const auto size = other.st_size();
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::vector_base(): constructed vector size would exceed max_size()", std::length_error);
			change_capacity(other.st_capacity());
			auto* dst = data();
			const auto iterEnd = other.end();
			for (auto iter = other.begin(); iter != iterEnd; ++iter) {
				allocator_traits::construct(_allocator, dst++, *iter);
			}
			set_size(size);
		}

		constexpr vector_base(vector_base&& other) noexcept(std::is_nothrow_move_constructible<allocator_type>::value)
			: _allocator(std::move(other._allocator))
		{
			move_data_from(other);
		}

		constexpr vector_base(vector_base&& other, const allocator_type& alloc)
			: _allocator(alloc)
		{
			move_data_from(other);
		}

		constexpr vector_base(std::initializer_list<T> list, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			const auto size = list.size();
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::vector_base(): constructed vector size would exceed max_size()", std::length_error);
			reserve(size);
			for (auto&& e : list) {
				push_back(std::move(e));
			}
		}

		constexpr ~vector_base() noexcept
		{
			destroy();
		}

		_PLUGIFY_VECTOR_DIAG_PUSH()

#if defined(__clang__) || defined(__GNUC__)
		_PLUGIFY_VECTOR_DIAG_IGN("-Wclass-memaccess")
#endif

		constexpr vector_base& operator=(const vector_base& other)
		{
			if (this == &other) [[unlikely]] {
				return *this;
			}

			if constexpr (std::is_trivially_copyable_v<T>) {
				if (sbo_active() && other.sbo_active()) {
					std::memcpy(this, &other, sizeof(*this));
				} else {
					resize_no_init(other.size());
					std::memcpy(data(), other.data(), other.size() * sizeof(T));
				}
			} else {
				assign(other.begin(), other.end());
			}

			return *this;
		}

		_PLUGIFY_VECTOR_DIAG_POP()

		template<typename A, bool SBO, size_t SBOP>
		constexpr vector_base& operator=(const vector_base<T, SBO, SBOP, A>& other)
		{
			if constexpr (std::is_same_v<decltype(this), decltype(&other)>) {
				if (this == &other) [[unlikely]] {
					return *this;
				}
			}

			if constexpr (std::is_trivially_copyable_v<T>) {
				resize_no_init(other.size());
				std::memcpy(data(), other.data(), other.size() * sizeof(T));
			} else {
				assign(other.begin(), other.end());
			}

			return *this;
		}

		constexpr vector_base& operator=(vector_base&& other) noexcept(
				allocator_type::propagate_on_container_move_assignment::value ||
				allocator_type::is_always_equal::value)
		{
			if (this == &other) [[unlikely]] {
				return *this;
			}

			destroy();
			_allocator = other._allocator;
			move_data_from(other);
			return *this;
		}

		constexpr vector_base& operator=(std::initializer_list<T> list)
		{
			assign(list.begin(), list.end());
			return *this;
		}

		constexpr void assign(size_t count, const T& value)
		{
			//_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector_base::assign(): resulted vector size would exceed max_size()", std::length_error);
			clear();
			resize(count, value);
		}

		template<class InputIt> requires (detail::is_iterator_v<InputIt>)
		constexpr void assign(InputIt begin, InputIt end)
		{
			//_PLUGIFY_VECTOR_ASSERT(count <= max_size(), "plg::vector_base::assign(): resulted vector size would exceed max_size()", std::length_error);
			clear();

			if constexpr (std::is_same_v<typename std::iterator_traits<InputIt>::iterator_category, std::random_access_iterator_tag>) {
				// RandomAccessIterator
				const auto sz = static_cast<size_t>(end - begin);
				reserve(sz);
				auto* dst = data();

				for (auto iter = begin; iter != end; ++iter) {
					allocator_traits::construct(_allocator, dst, *iter);
					++dst;
				}
				set_size(static_cast<size_type>(sz));
			} else {
				// Non-Random Access Iterator
				for (auto iter = begin; iter != end; ++iter) {
					push_back(*iter);
				}
			}
		}

		constexpr void assign(std::initializer_list<T> list)
		{
			assign(list.begin(), list.end());
		}

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
			return st_size();
		}

		[[nodiscard]] constexpr size_t max_size() const noexcept
		{
			return std::numeric_limits<size_type>::max() / 2;
		}

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return st_size() == 0;
		}

		[[nodiscard]] constexpr size_t capacity() const noexcept
		{
			return st_capacity();
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
			_PLUGIFY_VECTOR_ASSERT(index < size(), "plg::vector_base::at(): input index is out of bounds", std::out_of_range);
			return data()[index];
		}

		[[nodiscard]] constexpr const_reference at(size_t index) const
		{
			_PLUGIFY_VECTOR_ASSERT(index < size(), "plg::vector_base::at(): input index is out of bounds", std::out_of_range);
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
			do_resize(size, [&] (pointer bytes) {
				allocator_traits::construct(_allocator, bytes);
			});
		}

		constexpr void resize_no_init(size_t size)
		{
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::resize_no_init(): allocated memory size would exceed max_size()", std::length_error);
			do_resize(size);
		}

		constexpr void resize(size_t size, const value_type& value)
		{
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::resize(): allocated memory size would exceed max_size()", std::length_error);
			do_resize(size, [&] (pointer bytes)  {
				allocator_traits::construct(_allocator, bytes, value);
			});
		}

		constexpr void reserve(size_t size)
		{
			_PLUGIFY_VECTOR_ASSERT(size <= max_size(), "plg::vector_base::reserve(): allocated memory size would exceed max_size()", std::length_error);
			if (size > capacity()) {
				change_capacity(static_cast<size_type>(std::ranges::max(size, static_cast<size_t>(static_cast<float>(capacity()) * growth_factor))));
			}
		}

		constexpr void shrink_to_fit()
		{
			change_capacity(st_size());
		}

		constexpr void clear() noexcept
		{
			for (size_type i = 0; i < st_size(); ++i) {
				allocator_traits::destroy(_allocator, data() + i);
			}
			set_size(0);
		}

		constexpr iterator insert(const_iterator pos, const T& value)
		{
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return do_insert(pos, [&] (size_t) {
				push_back(value);
			});
		}

		constexpr iterator insert(const_iterator pos, T&& value)
		{
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return do_insert(pos, [&] (size_t) {
				push_back(std::move(value));
			});
		}

		constexpr iterator insert(const_iterator pos, size_t count, const T& value)
		{
			_PLUGIFY_VECTOR_ASSERT(size() + count <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return do_insert(pos, [&](size_t prevSize) {
				reserve(size() + count);
				for (size_t i = 0; i < count; ++i) {
					allocator_traits::construct(_allocator, data() + (i + prevSize), value);
				}
				set_size(static_cast<size_type>(prevSize + count));
			});
		}

		template<class InputIt> requires (detail::is_iterator_v<InputIt>)
		constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
		{
			const auto count = last - first;
			_PLUGIFY_VECTOR_ASSERT(size() + count <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return do_insert(pos, [&](size_t prevSize) {
				if constexpr (std::is_same_v<typename std::iterator_traits<InputIt>::iterator_category, std::random_access_iterator_tag>) {
					reserve(size() + count);
					size_t i = 0;
					for (auto iter = first; iter != last; ++iter) {
						allocator_traits::construct(_allocator, data() + (i + prevSize), *iter);
						++i;
					}
					set_size(static_cast<size_type>(prevSize + count));
				} else {
					for (auto iter = first; iter != last; ++iter) {
						push_back(*iter);
					}
				}
			});
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<T> initializerList)
		{
			_PLUGIFY_VECTOR_ASSERT(size() + initializerList.size() <= max_size(), "plg::vector_base::insert(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(size()), "plg::vector_base::insert(): pos out of range", std::out_of_range);
			return insert(pos, initializerList.begin(), initializerList.end());
		}

		template<class... Args>
		constexpr iterator emplace(const_iterator pos, Args&&... args)
		{
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector_base::emplace(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(size()), "plg::vector_base::emplace(): pos out of range", std::out_of_range);
			emplace_back(std::forward<Args>(args)...);
			std::ranges::rotate(begin() + index, end() - 1, end());
			return begin() + index;
		}

		constexpr iterator erase(const_iterator first, const_iterator last)
		{
			if (first == last) [[unlikely]] {
				return de_const_iter(last);
			}
			const auto count = last - first;
			_PLUGIFY_VECTOR_ASSERT(count >= 0 && count <= static_cast<difference_type>(size()), "plg::vector_base::erase(): last out of range", std::out_of_range);
			const auto index = first - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(size()), "plg::vector_base::erase(): first out of range", std::out_of_range);
			std::ranges::rotate(de_const_iter(first), de_const_iter(last), end());
			resize_down(static_cast<size_type>(size() - count));
			return begin() + index;
		}

		constexpr iterator erase(const_iterator pos)
		{
			const auto index = pos - begin();
			_PLUGIFY_VECTOR_ASSERT(index >= 0 && index <= static_cast<difference_type>(size()), "plg::vector_base::erase(): pos out of range", std::out_of_range);
			return erase(pos, pos + 1);
		}

		template<class... Args>
		constexpr reference emplace_back(Args&&... args)
		{
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector_base::emplace_back(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = size();
			if constexpr (std::is_trivially_copyable_v<T>) {
				ensure_capacity(st_size() + 1);
				data()[index] = T(std::forward<Args>(args)...);
			} else {
				construct_with_ensure_capacity(st_size() + 1, [&] (pointer data) {
					allocator_traits::construct(_allocator, data + index, std::forward<Args>(args)...);
				});
			}
			set_size(st_size() + 1);
			return elem(index);
		}

		constexpr void push_back(const T& value)
		{
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector_base::push_back(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = size();
			if constexpr (std::is_trivially_copyable_v<T>) {
				ensure_capacity(st_size() + 1);
				data()[index] = value;
			} else {
				construct_with_ensure_capacity(st_size() + 1, [&](pointer data)  {
				   allocator_traits::construct(_allocator, data + size(), value);
			   });
			}
			set_size(st_size() + 1);
		}

		constexpr void push_back(T&& value)
		{
			_PLUGIFY_VECTOR_ASSERT(size() + 1 <= max_size(), "plg::vector_base::push_back(): resulted vector size would exceed max_size()", std::length_error);
			const auto index = size();
			if constexpr (std::is_trivially_copyable_v<T>) {
				ensure_capacity(st_size() + 1);
				data()[index] = std::move(value);
			} else {
				construct_with_ensure_capacity(st_size() + 1, [&](pointer data) {
				   allocator_traits::construct(_allocator, data + size(), std::move(value));
			   	});
			}
			set_size(st_size() + 1);
		}

		constexpr void pop_back()
		{
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector_base::pop_back(): vector is empty", std::length_error);
			allocator_traits::destroy(_allocator, &back());
			set_size(st_size() - 1);
		}

		constexpr vector_base& operator+=(const T& value)
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

		constexpr vector_base& operator+=(T&& value)
		{
			push_back(std::move(value));
			return *this;
		}

		constexpr void swap(vector_base& other) noexcept(allocator_traits::propagate_on_container_swap::value || allocator_traits::is_always_equal::value)
		{
			using std::ranges::swap;
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
				constexpr auto sbo_align_enabled = alignof(vector_base) >= alignof(T);
				constexpr auto first_sbo_offset = sbo_start_offset_bytes();
				constexpr auto result = sbo_align_enabled && first_sbo_offset < size_bytes ? (size_bytes - first_sbo_offset) / sizeof(T) : 0ull;
				static_assert(result <= 127); // More than 127 wouldn't fit in the 7-bit size field
				return static_cast<size_type>(result);
			} else {
				return 0;
			}
		}

		[[nodiscard]] constexpr std::span<const T> span() const noexcept
		{
			return std::span<const T>(data(), size());
		}

		[[nodiscard]] constexpr std::span<T> span() noexcept
		{
			return std::span<T>(data(), size());
		}

		[[nodiscard]] constexpr std::span<const T> const_span() const noexcept
		{
			return std::span<const T>(data(), size());
		}

		template<size_t Size>
		[[nodiscard]] constexpr std::span<T, Size> span_size() noexcept
		{
			_PLUGIFY_VECTOR_ASSERT(size() == Size, "plg::vector_base::span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<T, Size>(data(), size());
		}

		template<size_t Size>
		[[nodiscard]] constexpr std::span<const T, Size> const_span_size() const noexcept
		{
			_PLUGIFY_VECTOR_ASSERT(size() == Size, "plg::vector_base::const_span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<const T, Size>(data(), size());
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
			return std::ranges::find(begin(), end(), elem) != end();
		}

		template<typename F>
		[[nodiscard]] constexpr bool contains_if(F predicate)
		{
			return std::ranges::find_if(begin(), end(), predicate) != end();
		}

		[[nodiscard]] constexpr auto find(const T& value) const
		{
			return std::ranges::find(begin(), end(), value);
		}

		[[nodiscard]] constexpr auto find(const T& value)
		{
			return std::ranges::find(begin(), end(), value);
		}

		template<typename F>
		[[nodiscard]] constexpr auto find_if(F predicate) const
		{
			return std::ranges::find_if(begin(), end(), predicate);
		}

		template<typename F>
		[[nodiscard]] constexpr auto find_if(F predicate)
		{
			return std::ranges::find_if(begin(), end(), predicate);
		}

		[[nodiscard]] constexpr std::optional<size_t> find_index(const T& value)
		{
			const auto iter = std::ranges::find(begin(), end(), value);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		[[nodiscard]] constexpr std::optional<size_t> find_index(const T& value) const
		{
			const auto iter = std::ranges::find(begin(), end(), value);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		template<typename F>
		[[nodiscard]] constexpr std::optional<size_t> find_index_if(F predicate)
		{
			const auto iter = std::ranges::find_if(begin(), end(), predicate);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

		template<typename F>
		[[nodiscard]] constexpr std::optional<size_t> find_index_if(F predicate) const
		{
			const auto iter = std::ranges::find_if(begin(), end(), predicate);
			if (iter == end()) {
				return {};
			} else {
				return iter - begin();
			}
		}

	private:

		_PLUGIFY_VECTOR_DIAG_PUSH()

#if defined(__clang__)
		_PLUGIFY_VECTOR_DIAG_IGN("-Wgnu-anonymous-struct")
		_PLUGIFY_VECTOR_DIAG_IGN("-Wzero-length-array")
#elif defined(__GNUC__)
		_PLUGIFY_VECTOR_DIAG_IGN("-Wpedantic")// this doesn't work
#elif defined(_MSC_VER)
		_PLUGIFY_VECTOR_DIAG_IGN(4201)
		_PLUGIFY_VECTOR_DIAG_IGN(4200)
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

		_PLUGIFY_VECTOR_DIAG_POP()

		static_assert(sizeof(sbo_size) == sizeof(size_type));
		static_assert(alignof(sbo_size) == alignof(size_type));

		constexpr void change_capacity(size_type newCapacity)
		{
			change_capacity(newCapacity, [](pointer) {});
		}

		constexpr void change_capacity(size_type newCapacity, auto construct)
		{
			const auto size = st_size();
			const auto capacity = st_capacity();
			_PLUGIFY_VECTOR_ASSERT(newCapacity >= size, "plg::vector_base::change_capacity(): resulted vector size would exceed size()", std::length_error);
			if (newCapacity != capacity) {
				// Allocate new memory
				const bool canUseSBO = sbo_max_objects() >= newCapacity;
				pointer newData = canUseSBO ? sbo_data() : (newCapacity > 0 ? allocator_traits::allocate(_allocator, newCapacity) : nullptr);
				construct(newData);

				// Move old objects
				auto* oldData = data();
				if constexpr (std::is_trivially_copyable_v<T>) {
					if (size > 0) {
						std::memcpy(newData, oldData, size * sizeof(T));
					}
				} else {
					for (size_type i = 0; i < size; ++i) {
						allocator_traits::construct(_allocator, newData + i, std::move(oldData[i]));
						allocator_traits::destroy(_allocator, oldData + i);
					}
				}

				// Deallocate old memory
				if (!sbo_active() && oldData && capacity > 0) {
					allocator_traits::deallocate(_allocator, oldData, capacity);
				}

				if (sbo_active() != canUseSBO) {
					if constexpr (sbo_enabled()) {
						_size.small.sbo_enabled = canUseSBO;
					}
					set_size(size);
				}

				if (!canUseSBO) {
					_data = newData;
					_capacity = newCapacity;
				}
			}
		}

		constexpr void do_resize(size_t sz)
		{
			const auto newSize = static_cast<size_type>(sz);
			if (newSize > st_size()) {
				if (newSize > capacity()) {
					change_capacity(newSize);
				}
				set_size(newSize);
			} else if (newSize < st_size()) {
				resize_down(newSize);
			}
		}

		constexpr void do_resize(size_t sz, auto construct)
		{
			const auto newSize = static_cast<size_type>(sz);
			if (newSize > st_size()) {
				if (newSize > capacity()) {
					change_capacity(newSize);
				}
				auto* d = data();
				for (size_type i = st_size(); i < newSize; ++i) {
					construct(d + i);
				}
				set_size(newSize);
			} else if (newSize < st_size()) {
				resize_down(newSize);
			}
		}

		constexpr void resize_down(size_type newSize)
		{
			_PLUGIFY_VECTOR_ASSERT(newSize <= st_size(), "plg::vector_base::resize_down(): resulted vector size would exceed size()", std::length_error);
			auto* d = data();
			for (size_type i = newSize; i < st_size(); ++i) {
				allocator_traits::destroy(_allocator, d + i);
			}
			set_size(newSize);
		}

		constexpr void set_size(size_type sz)
		{
			if (sbo_active()) {
				_size.small.size = (sz & 0x7F);
			} else {
				_size.big.size = (sz & 0x7FFFFFFFFFFFFFFF);
			}
		}

		constexpr void ensure_capacity(size_type minCapacity)
		{
			if (capacity() < minCapacity) {
				change_capacity(std::ranges::max(minCapacity, static_cast<size_type>(static_cast<float>(capacity()) * growth_factor)));
			}
		}

		constexpr void construct_with_ensure_capacity(size_type minCapacity, auto construct)
		{
			if (capacity() >= minCapacity) {
				construct(data());
			} else {
				change_capacity(std::ranges::max(minCapacity, static_cast<size_type>(static_cast<float>(capacity()) * growth_factor)), construct);
			}
		}

		constexpr iterator do_insert(const_iterator pos, auto func)
		{
			const auto oldSize = size();
			const auto index = pos - begin();

			func(oldSize);

			if (pos != end()) {
				std::ranges::rotate(begin() + index, begin() + oldSize, end());
				return begin() + index;
			} else {
				return begin() + oldSize;
			}
		}

		constexpr void move_data_from(vector_base& other)
		{
			if (other.sbo_active()) {
				// Using SBO, move elements
				_size = other._size;

				if constexpr (std::is_trivially_copyable_v<T>) {
					std::memcpy(data(), other.data(), size() * sizeof(T));
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

		[[nodiscard]] constexpr size_type st_size() const noexcept
		{
			return sbo_active() ? _size.small.size : _size.big.size;
		}

		[[nodiscard]] constexpr size_type st_capacity() const noexcept
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
			return detail::align_up<size_t>(1ull, alignof(T));
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
