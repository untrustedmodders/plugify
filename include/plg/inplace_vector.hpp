#pragma once

#include "plg/macro.hpp"

#if __has_include(<inplace_vector>)
#include <inplace_vector>
#if defined(__cpp_lib_inplace_vector) && __cpp_lib_inplace_vector >= 202406L
#define PLUGIFY_HAS_STD_INPLACE_VECTOR 1
#else
#define PLUGIFY_HAS_STD_INPLACE_VECTOR 0
#endif
#else
#define PLUGIFY_HAS_STD_INPLACE_VECTOR 0
#endif

#if !PLUGIFY_HAS_STD_INPLACE_VECTOR
#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>

#if PLUGIFY_CPP_VERSION >= 202002L
#include <compare>
#include <ranges>
#endif

#ifndef PLUGIFY_INPLACE_VECTOR_TRIVIALLY_RELOCATABLE_IF
#if defined(__cpp_impl_trivially_relocatable) && defined(__cpp_lib_trivially_relocatable)
#define PLUGIFY_INPLACE_VECTOR_TRIVIALLY_RELOCATABLE_IF(x) [[trivially_relocatable(x)]]
#else
#define PLUGIFY_INPLACE_VECTOR_TRIVIALLY_RELOCATABLE_IF(x)
#endif // __cpp_impl_trivially_relocatable
#endif // PLUGIFY_INPLACE_VECTOR_TRIVIALLY_RELOCATABLE_IF

// from https://github.com/Quuxplusone/SG14
namespace plg {
	template<class T, bool = (std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>),
					  bool = (std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)>
	struct ipvbase_assignable {
		// Base for copyable types
	};
	template<class T, bool Copyable>
	struct ipvbase_assignable<T, Copyable, false> {
		// Base for immobile types like std::mutex
		explicit ipvbase_assignable() = default;
		ipvbase_assignable(ipvbase_assignable&&) = delete;
		ipvbase_assignable(const ipvbase_assignable&) = delete;
		void operator=(ipvbase_assignable&&) = delete;
		void operator=(const ipvbase_assignable&) = delete;
		~ipvbase_assignable() = default;
	};
	template<class T>
	struct ipvbase_assignable<T, false, true> {
		explicit ipvbase_assignable() = default;
		ipvbase_assignable(const ipvbase_assignable&) = delete;
		ipvbase_assignable(ipvbase_assignable&&) = default;
		void operator=(const ipvbase_assignable&) = delete;
		ipvbase_assignable& operator=(ipvbase_assignable&&) = default;
		~ipvbase_assignable() = default;
	};

	template<class T, size_t N, class = void>
	struct PLUGIFY_INPLACE_VECTOR_TRIVIALLY_RELOCATABLE_IF(std::is_trivially_relocatable_v<T>) ipvbase
	{
		size_t _size = 0;
		union {
			char _dummy;
			T _data[N];
		};

		constexpr T *base_data() { return _data; }
		constexpr const T *base_data() const { return _data; }
		constexpr void set_size(size_t n) { _size = n; }

		constexpr explicit ipvbase() noexcept {}
		ipvbase(const ipvbase& rhs)
			noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			if constexpr (std::is_trivially_copy_constructible_v<T>) {
				std::memmove((void*)this, (const void*)std::addressof(rhs), sizeof(ipvbase));
			} else {
				std::uninitialized_copy_n(rhs._data, rhs._size, _data);
				_size = rhs._size;
			}
		}
		ipvbase(ipvbase&& rhs)
			noexcept(std::is_nothrow_move_constructible_v<T>
	#if defined(__cpp_lib_trivially_relocatable)
										|| std::is_trivially_relocatable_v<T>
	#endif // __cpp_lib_trivially_relocatable
							   )
		{
			if constexpr (std::is_trivially_move_constructible_v<T>) {
				std::memmove((void*)this, (const void*)std::addressof(rhs), sizeof(ipvbase));
	#if defined(__cpp_lib_trivially_relocatable)
			} else if constexpr (std::is_trivially_relocatable_v<T>) {
				std::uninitialized_relocate_n(rhs._data, rhs._size, _data);
				_size = rhs._size;
				rhs._size = 0;
	#endif // __cpp_lib_trivially_relocatable
			} else {
				std::uninitialized_move_n(rhs._data, rhs._size, _data);
				_size = rhs._size;
			}
		}
		void operator=(const ipvbase& rhs)
			noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
		{
			if constexpr (std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> && std::is_trivially_destructible_v<T>) {
				std::memmove((void*)this, (const void*)std::addressof(rhs), sizeof(ipvbase));
			} else if (this == std::addressof(rhs)) {
				// do nothing
			} else if (rhs._size <= _size) {
				std::copy(rhs._data, rhs._data + rhs._size, _data);
				std::destroy(_data + rhs._size, _data + _size);
				_size = rhs._size;
			} else {
				std::copy(rhs._data, rhs._data + _size, _data);
				std::uninitialized_copy(rhs._data + _size, rhs._data + rhs._size, _data + _size);
				_size = rhs._size;
			}
		}
		void operator=(ipvbase&& rhs)
			noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
		{
			if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> && std::is_trivially_destructible_v<T>) {
				std::memmove((void*)this, (const void*)std::addressof(rhs), sizeof(ipvbase));
			} else if (this == std::addressof(rhs)) {
				// do nothing
			} else if (rhs._size <= _size) {
				std::move(rhs._data, rhs._data + rhs._size, _data);
				std::destroy(_data + rhs._size, _data + _size);
				_size = rhs._size;
			} else {
				std::move(rhs._data, rhs._data + _size, _data);
	#if defined(__cpp_lib_trivially_relocatable)
				if constexpr (std::is_trivially_relocatable_v<T>) {
					std::uninitialized_relocate(rhs._data + _size, rhs._data + rhs._size, _data + _size);
					std::swap(rhs._size, _size);
					return;
				}
	#endif // __cpp_lib_trivially_relocatable
				std::uninitialized_move(rhs._data + _size, rhs._data + rhs._size, _data + _size);
				_size = rhs._size;
			}
		}

	#if __cpp_concepts >= 202002L
		ipvbase(const ipvbase&) requires std::is_trivially_copy_constructible_v<T> = default;
		ipvbase(ipvbase&&) requires std::is_trivially_move_constructible_v<T> = default;
		ipvbase& operator=(const ipvbase&) requires std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> && std::is_trivially_destructible_v<T> = default;
		ipvbase& operator=(ipvbase&&) requires std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> && std::is_trivially_destructible_v<T> = default;
		~ipvbase() requires std::is_trivially_destructible_v<T> = default;
	#endif // __cpp_concepts >= 202002L

	#if PLUGIFY_CPP_VERSION >= 202002L
		constexpr
	#endif // PLUGIFY_CPP_VERSION >= 202002L
		~ipvbase() {
			std::destroy(_data, _data + _size);
		}
	};

	template<class T>
	struct ipvbase_zero {
		static constexpr size_t _size = 0;
		constexpr T *base_data() { return nullptr; }
		constexpr const T *base_data() const { return nullptr; }
		constexpr void set_size(size_t) { }
	};

	template<class T, size_t N>
	struct ipvbase_trivial {
		size_t _size = 0;
		union {
			char _dummy;
			T _data[N];
		};
		constexpr explicit ipvbase_trivial() {}
		constexpr T *base_data() { return _data; }
		constexpr const T *base_data() const { return _data; }
		constexpr void set_size(size_t n) { _size = n; }
	};

	template<class T, size_t N>
	using ipvbase_t = std::conditional_t<
		N == 0,
		ipvbase_zero<T>,
		std::conditional_t<
			std::is_trivially_copyable_v<T>,
			ipvbase_trivial<T, N>,
			ipvbase<T, N>
		>
	>;

	template<class T, size_t N>
	class inplace_vector : ipvbase_assignable<T>, ipvbase_t<T, N> {
		using ipvbase_t<T, N>::_size;
		using ipvbase_t<T, N>::set_size;
	public:
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using iterator = T*;
		using const_iterator = const T*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		// [inplace.vector.cons]

		inplace_vector() = default;
		inplace_vector(inplace_vector&&) = default;
		inplace_vector(const inplace_vector&) = default;
		inplace_vector& operator=(inplace_vector&&) = default;
		inplace_vector& operator=(const inplace_vector&) = default;
		inplace_vector& operator=(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); return *this; }

		constexpr inplace_vector(std::initializer_list<value_type> il) : inplace_vector(il.begin(), il.end()) { }
		constexpr explicit inplace_vector(size_t n) {
			PLUGIFY_ASSERT(n <= N, "resulted vector size would exceed capacity()", std::bad_alloc);
			std::uninitialized_value_construct_n(data(), n);
			set_size(n);
		}
		constexpr explicit inplace_vector(size_t n, const value_type& value) { assign(n, value); }

		template<std::input_iterator InputIterator>
		constexpr explicit inplace_vector(InputIterator first, InputIterator last) {
			if constexpr (std::random_access_iterator<InputIterator>) {
				size_t n = static_cast<size_type>(std::distance(first, last));
				PLUGIFY_ASSERT(n <= N, "resulted vector size would exceed capacity()", std::bad_alloc);
				std::uninitialized_copy_n(first, n, data());
				set_size(n);
			} else {
				for (; first != last; ++first) {
					emplace_back(*first);
				}
			}
		}

		constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }

		constexpr void assign(size_t n, const value_type& value) {
			if (n <= _size) {
				std::fill_n(data(), n, value);
				std::destroy(data() + n, data() + _size);
				set_size(n);
			} else if (n > N) {
				PLUGIFY_ASSERT(false, "memory size would exceed capacity()", std::bad_alloc);
			} else {
				std::fill_n(data(), _size, value);
				std::uninitialized_fill_n(data() + _size, n - _size, value);
			}
		}

		template<std::input_iterator InputIterator>
		constexpr void assign(InputIterator first, InputIterator last) {
			size_t n = _size;
			for (size_t i = 0; i < n; ++i) {
				if (first == last) {
					std::destroy(data() + i, data() + n);
					set_size(i);
					return;
				}
				(*this)[i] = *first;
				++first;
			}
			for (; first != last; ++first) {
				emplace_back(*first);
			}
		}

	#if __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L
		template<std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
		constexpr explicit inplace_vector(std::from_range_t, R&& rg) {
			if constexpr (std::ranges::sized_range<R>) {
				size_t n = std::ranges::size(rg);
				PLUGIFY_ASSERT(n <= N, "resulted vector size would exceed capacity()", std::bad_alloc);
				std::ranges::uninitialized_copy_n(std::ranges::begin(rg), n, data(), std::unreachable_sentinel);
				set_size(n);
			} else {
				for (auto&& e : rg) {
					emplace_back(decltype(e)(e));
				}
			}
		}

		template<std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
		constexpr void assign_range(R&& rg) {
			auto first = std::ranges::begin(rg);
			auto last = std::ranges::end(rg);
			size_t n = _size;
			for (size_t i = 0; i < n; ++i) {
				if (first == last) {
					std::destroy(data() + i, data() + n);
					set_size(i);
					return;
				}
				(*this)[i] = *first;
				++first;
			}
			for (; first != last; ++first) {
				emplace_back(*first);
			}
		}
	#endif // __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L

		// iterators

		constexpr iterator begin() noexcept { return data(); }
		constexpr iterator end() noexcept { return data() + _size; }
		constexpr const_iterator begin() const noexcept { return data(); }
		constexpr const_iterator end() const noexcept { return data() + _size; }
		constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
		constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
		constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
		constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
		constexpr const_iterator cbegin() const noexcept { return data(); }
		constexpr const_iterator cend() const noexcept { return data() + _size; }
		constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
		constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

		constexpr void resize(size_type n) {
			if (n < _size) {
				std::destroy(data() + n, data() + _size);
				set_size(n);
			} else if (n > N) {
				PLUGIFY_ASSERT(false, "memory size would exceed capacity()", std::bad_alloc);
			} else {
				std::uninitialized_value_construct(data() + _size, data() + n);
				set_size(_size + n);
			}
		}

		constexpr void resize(size_type n, const value_type& value) {
			if (n < _size) {
				std::destroy(data() + n, data() + _size);
				set_size(n);
			} else if (n > N) {
				PLUGIFY_ASSERT(false, "memory size would exceed capacity()", std::bad_alloc);
			} else {
				std::uninitialized_fill(data() + _size, data() + n, value);
				set_size(_size + n);
			}
		}

		static constexpr void reserve(size_type n) { PLUGIFY_ASSERT(n <= N, "resulted vector size would exceed capacity()", std::bad_alloc); }
		static constexpr void shrink_to_fit() noexcept {}

		// element access

		constexpr reference operator[](size_type i) { return data()[i]; }
		constexpr reference front() { return data()[0]; }
		constexpr reference back() { return data()[_size - 1]; }

		constexpr const_reference operator[](size_type i) const { return data()[i]; }
		constexpr const_reference front() const { return data()[0]; }
		constexpr const_reference back() const { return data()[_size - 1]; }

		constexpr reference at(size_type i) {
			PLUGIFY_ASSERT(i < _size, "input index is out of bounds", std::out_of_range);
			return data()[i];
		}
		constexpr const_reference at(size_type i) const {
			PLUGIFY_ASSERT(i < _size, "input index is out of bounds", std::out_of_range);
			return data()[i];
		}

		// [inplace.vector.data]

		constexpr T* data() noexcept { return this->base_data(); }
		constexpr const T* data() const noexcept { return this->base_data(); }
		constexpr size_type size() const noexcept { return _size; }
		static constexpr size_type max_size() noexcept { return N; }
		static constexpr size_type capacity() noexcept { return N; }
		[[nodiscard]] constexpr bool empty() const noexcept { return _size == 0; };

		// [inplace.vector.modifiers]

		template<class... Args>
		value_type& unchecked_emplace_back(Args&&... args) {
			// Precondition: (_size < N)
			value_type* p = data() + _size;
			p = std::construct_at(p, std::forward<Args>(args)...);
			set_size(_size + 1);
			return *p;
		}
		value_type& unchecked_push_back(const value_type& value) { return unchecked_emplace_back(value); }
		value_type& unchecked_push_back(value_type&& value) { return unchecked_emplace_back(static_cast<value_type&&>(value)); }

		template<class... Args>
		constexpr value_type* try_emplace_back(Args&&... args) {
			if (_size == N) {
				return nullptr;
			}
			return std::addressof(unchecked_emplace_back(static_cast<Args&&>(args)...));
		}
		constexpr value_type* try_push_back(const value_type& value) { return try_emplace_back(value); }
		constexpr value_type* try_push_back(value_type&& value) { return try_emplace_back(static_cast<value_type&&>(value)); }

		template<class... Args>
		value_type& emplace_back(Args&&... args) {
			PLUGIFY_ASSERT(_size != N, "resulted vector size would exceed capacity()", std::bad_alloc);
			return unchecked_emplace_back(static_cast<Args&&>(args)...);
		}
		value_type& push_back(const value_type& value) { return emplace_back(value); }
		value_type& push_back(value_type&& value) { return emplace_back(static_cast<value_type&&>(value)); }

	#if __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L
		template<std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
		constexpr void append_range(R&& rg) {
			for (auto&& e : rg) {
				emplace_back(static_cast<decltype(e)>(e));
			}
		}
	#endif // __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L

		void pop_back() {
			std::destroy_at(data() + _size - 1);
			set_size(_size - 1);
		}

		template<class... Args>
		iterator emplace(const_iterator pos, Args&&... args) {
			auto it = iterator(pos);
			emplace_back(static_cast<Args&&>(args)...);
			std::rotate(it, end() - 1, end());
			return it;
		}
		iterator insert(const_iterator pos, const value_type& value) { return emplace(pos, value); }
		iterator insert(const_iterator pos, value_type&& value) { return emplace(pos, static_cast<value_type&&>(value)); }

		iterator insert(const_iterator pos, size_type n, const value_type& value) {
			PLUGIFY_ASSERT(N - _size >= n, "resulted vector size would exceed capacity()", std::bad_alloc);
			auto it = iterator(pos);
			auto oldend = end();
	#if defined(__cpp_lib_trivially_relocatable)
			// Open a window and fill in-place; if filling fails, close the window again.
			if constexpr (std::is_trivially_relocatable_v<value_type>) {
				std::uninitialized_relocate_backward(it, oldend, oldend + n);
				try {
					std::uninitialized_fill_n(it, n, value);
					set_size(_size + n);
				} catch (...) {
					std::uninitialized_relocate(it + n, oldend + n, it);
					throw;
				}
				return it;
			}
	#endif
			// Fill at the end of the vector, then rotate into place.
			std::uninitialized_fill_n(oldend, n, value);
			set_size(_size + n);
			std::rotate(it, oldend, oldend + n);
			return it;
		}

		template<std::input_iterator InputIterator>
		iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
			auto it = iterator(pos);
			auto oldend = end();
			if constexpr (std::random_access_iterator<InputIterator>) {
				size_type n = static_cast<size_type>(std::distance(first, last));
				PLUGIFY_ASSERT(N - _size >= n, "resulted vector size would exceed capacity()", std::bad_alloc);
	#if defined(__cpp_lib_trivially_relocatable)
				// Open a window and fill in-place; if filling fails, close the window again.
				if constexpr (std::is_trivially_relocatable_v<value_type>) {
					std::uninitialized_relocate_backward(it, oldend, oldend + n);
					try {
						std::uninitialized_copy_n(first, n, it);
						set_size(_size + n);
					} catch (...) {
						std::uninitialized_relocate(it + n, oldend + n, it);
						throw;
					}
					return it;
				}
	#endif
				// Fill at the end of the vector, then rotate into place.
				std::uninitialized_copy_n(first, n, oldend);
				set_size(_size + n);
				std::rotate(it, oldend, oldend + n);
			} else {
				auto oldend = end();
				for (; first != last; ++first) {
					emplace_back(*first);
				}
				std::rotate(it, oldend, end());
			}
			return it;
		}

	#if __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L
		template<std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
		iterator insert_range(const_iterator pos, R&& rg) {
			auto it = iterator(pos);
			auto oldend = end();
			if constexpr (std::ranges::sized_range<R>) {
				size_type n = std::ranges::size(rg);
				PLUGIFY_ASSERT(N - _size >= n, "resulted vector size would exceed capacity()", std::bad_alloc);
	#if defined(__cpp_lib_trivially_relocatable)
				// Open a window and fill in-place; if filling fails, close the window again.
				if constexpr (std::is_trivially_relocatable_v<value_type>) {
					std::uninitialized_relocate_backward(it, oldend, oldend + n);
					try {
						std::ranges::uninitialized_copy_n(std::ranges::begin(rg), n, it, std::unreachable_sentinel);
						set_size(_size + n);
					} catch (...) {
						std::uninitialized_relocate(it + n, oldend + n, it);
						throw;
					}
					return it;
				}
	#endif
				// Fill at the end of the vector, then rotate into place.
				std::ranges::uninitialized_copy_n(std::ranges::begin(rg), n, oldend, std::unreachable_sentinel);
				set_size(_size + n);
				std::rotate(it, oldend, oldend + n);
			} else {
				auto [rgend, newend] = std::ranges::uninitialized_copy(rg, std::ranges::subrange(oldend, data() + N));
				if (rgend != std::ranges::end(rg)) {
					std::destroy(oldend, newend);
					PLUGIFY_ASSERT(false, "resulted vector size would exceed capacity()", std::bad_alloc);
				} else {
					set_size(newend - data());
					std::rotate(it, oldend, newend);
				}
			}
			return it;
		}
	#endif // __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L

		iterator insert(const_iterator pos, std::initializer_list<value_type> il) { return insert(pos, il.begin(), il.end()); }

		iterator erase(const_iterator pos) {
			auto it = iterator(pos);
			auto oldend = end();
	#if defined(__cpp_lib_trivially_relocatable)
			if constexpr (std::is_trivially_relocatable_v<value_type>) {
				std::destroy_at(it);
				std::uninitialized_relocate(it + 1, oldend, it);
				set_size(_size - 1);
				return it;
			}
	#endif
			std::move(it + 1, oldend, it);
			std::destroy_at(oldend - 1);
			set_size(_size - 1);
			return it;
		}

		iterator erase(const_iterator first, const_iterator last) {
			auto ifirst = iterator(first);
			auto ilast = iterator(last);
			auto n = static_cast<size_type>(std::distance(ifirst, ilast));
			if (n != 0) {
				auto oldend = end();
	#if defined(__cpp_lib_trivially_relocatable)
				if constexpr (std::is_trivially_relocatable_v<value_type>) {
					std::destroy(ifirst, ilast);
					std::uninitialized_relocate(ilast, oldend, ifirst);
					set_size(_size - n);
					return ifirst;
				}
	#endif // __cpp_lib_trivially_relocatable
				std::destroy(std::move(ilast, oldend, ifirst), oldend);
				set_size(_size - n);
			}
			return ifirst;
		}

		constexpr void clear() noexcept {
			std::destroy(data(), data() + _size);
			set_size(0);
		}

		constexpr void swap(inplace_vector& b)
			noexcept(N == 0 || (std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T>))
		{
			auto& a = *this;
			if (a._size < b._size) {
				b.swap(a);
			} else {
				std::swap_ranges(a.data(), a.data() + b._size, b.data());
	#if defined(__cpp_lib_trivially_relocatable)
				size_t n = a._size;
				a.set_size(b._size);
				std::uninitialized_relocate(a.data() + b._size, a.data() + n, b.data() + b._size);
				b.set_size(n);
	#else
				std::uninitialized_move(a.data() + b._size, a.data() + a._size, b.data() + b._size);
				std::destroy(a.data() + b._size, a.data() + a._size);
				if constexpr (N != 0) {
					std::swap(a._size, b._size);
				}
	#endif
			}
		}

		friend constexpr void swap(inplace_vector& a, inplace_vector& b) noexcept(noexcept(a.swap(b))) {
			a.swap(b);
		}

		constexpr friend bool operator==(const inplace_vector& lhs, const inplace_vector& rhs) {
			if (lhs.size() != rhs.size()) return false;
			return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
		}

	#if __cpp_impl_three_way_comparison >= 201907L
		constexpr friend auto operator<=>(const inplace_vector& lhs, const inplace_vector& rhs) {
			return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
		}
	#else
		constexpr friend bool operator<(const inplace_vector& a, const inplace_vector& b) {
			const T *adata = a.data();
			const T *bdata = b.data();
			size_t n = (a._size < b._size) ? a._size : b._size;
			for (size_t i = 0; i < n; ++i) {
				if (adata[i] < bdata[i]) {
					return true;
				} else if (bdata[i] < adata[i]) {
					return false;
				}
			}
			return (a._size < b._size);
		}
		constexpr friend bool operator>(const inplace_vector& a, const inplace_vector& b) { return (b < a); }
		constexpr friend bool operator<=(const inplace_vector& a, const inplace_vector& b) { return !(b < a); }
		constexpr friend bool operator>=(const inplace_vector& a, const inplace_vector& b) { return !(a < b); }
		constexpr friend bool operator!=(const inplace_vector& a, const inplace_vector& b) { return !(a == b); }
	#endif
	};
} // namespace plg

namespace std {
	template<class T, std::size_t N>
	using inplace_vector = plg::inplace_vector<T, N>;
} // namespace std

#endif