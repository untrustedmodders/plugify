#pragma once

#include "plg/config.hpp"

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
	namespace detail {

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
			size_t size_ = 0;
			union {
				[[maybe_unused]] char dummy_;
				T data_[N];
			};

			constexpr T *base_data() { return data_; }
			constexpr const T *base_data() const { return data_; }
			constexpr void set_size(size_t n) { size_ = n; }

			constexpr explicit ipvbase() noexcept {}
			ipvbase(const ipvbase& rhs)
				noexcept(std::is_nothrow_copy_constructible_v<T>)
			{
				if constexpr (std::is_trivially_copy_constructible_v<T>) {
					std::memmove((void*)this, (const void*)std::addressof(rhs), sizeof(ipvbase));
				} else {
					std::uninitialized_copy_n(rhs.data_, rhs.size_, data_);
					size_ = rhs.size_;
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
					std::uninitialized_relocate_n(rhs.data_, rhs.size_, data_);
					size_ = rhs.size_;
					rhs.size_ = 0;
#endif // __cpp_lib_trivially_relocatable
				} else {
					std::uninitialized_move_n(rhs.data_, rhs.size_, data_);
					size_ = rhs.size_;
				}
			}
			void operator=(const ipvbase& rhs)
				noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
			{
				if constexpr (std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> && std::is_trivially_destructible_v<T>) {
					std::memmove((void*)this, (const void*)std::addressof(rhs), sizeof(ipvbase));
				} else if (this == std::addressof(rhs)) {
					// do nothing
				} else if (rhs.size_ <= size_) {
					std::copy(rhs.data_, rhs.data_ + rhs.size_, data_);
					std::destroy(data_ + rhs.size_, data_ + size_);
					size_ = rhs.size_;
				} else {
					std::copy(rhs.data_, rhs.data_ + size_, data_);
					std::uninitialized_copy(rhs.data_ + size_, rhs.data_ + rhs.size_, data_ + size_);
					size_ = rhs.size_;
				}
			}
			void operator=(ipvbase&& rhs)
				noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
			{
				if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> && std::is_trivially_destructible_v<T>) {
					std::memmove((void*)this, (const void*)std::addressof(rhs), sizeof(ipvbase));
				} else if (this == std::addressof(rhs)) {
					// do nothing
				} else if (rhs.size_ <= size_) {
					std::move(rhs.data_, rhs.data_ + rhs.size_, data_);
					std::destroy(data_ + rhs.size_, data_ + size_);
					size_ = rhs.size_;
				} else {
					std::move(rhs.data_, rhs.data_ + size_, data_);
#if defined(__cpp_lib_trivially_relocatable)
					if constexpr (std::is_trivially_relocatable_v<T>) {
						std::uninitialized_relocate(rhs.data_ + size_, rhs.data_ + rhs.size_, data_ + size_);
						std::swap(rhs.size_, size_);
						return;
					}
#endif // __cpp_lib_trivially_relocatable
					std::uninitialized_move(rhs.data_ + size_, rhs.data_ + rhs.size_, data_ + size_);
					size_ = rhs.size_;
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
				std::destroy(data_, data_ + size_);
			}
		};

		template<class T>
		struct ipvbase_zero {
		static constexpr size_t size_ = 0;
			constexpr T *base_data() { return nullptr; }
		constexpr const T *base_data() const { return nullptr; }
			constexpr void set_size(size_t) { }
		};

		template<class T, size_t N>
		struct ipvbase_trivial {
			size_t size_ = 0;
			union {
				[[maybe_unused]] char dummy_;
				T data_[N];
			};
			constexpr explicit ipvbase_trivial() {}
			constexpr T *base_data() { return data_; }
			constexpr const T *base_data() const { return data_; }
			constexpr void set_size(size_t n) { size_ = n; }
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
	} // namespace detail

	template<class T, size_t N>
	class inplace_vector : detail::ipvbase_assignable<T>, detail::ipvbase_t<T, N> {
		using detail::ipvbase_t<T, N>::size_;
		using detail::ipvbase_t<T, N>::set_size;
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
		inplace_vector& operator=(std::initializer_list<value_type> il)
			requires std::is_copy_constructible_v<T>
		{
			assign(il.begin(), il.end());
			return *this;
		}

		constexpr inplace_vector(std::initializer_list<value_type> il)
			requires std::copy_constructible<T> : inplace_vector(il.begin(), il.end()) { }
		constexpr explicit inplace_vector(size_t n)
			requires std::default_initializable<T>
		{
			if (n > N) {
				throw_bad_alloc();
			}
			std::uninitialized_value_construct_n(data(), n);
			set_size(n);
		}
		constexpr explicit inplace_vector(size_t n, const value_type& value)
			requires std::copy_constructible<T>
		{
			assign(n, value);
		}

		template<std::input_iterator InputIterator>
			requires std::constructible_from<T, typename std::iterator_traits<InputIterator>::value_type>
		constexpr explicit inplace_vector(InputIterator first, InputIterator last)
		{
			if constexpr (std::random_access_iterator<InputIterator>) {
				size_t n = static_cast<size_type>(std::distance(first, last));
				if (n > N) {
					throw_bad_alloc();
				}
				std::uninitialized_copy_n(first, n, data());
				set_size(n);
			} else {
				for (; first != last; ++first) {
					emplace_back(*first);
				}
			}
		}

		constexpr void assign(std::initializer_list<value_type> il)
			requires std::is_copy_constructible_v<T>
		{
			assign(il.begin(), il.end());
		}

		constexpr void assign(size_t n, const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			if (n > N) {
				throw_bad_alloc();
			} else if (size_ >= n) {
				std::fill_n(data(), n, value);
				std::destroy(data() + n, data() + size_);
			} else {
				std::fill_n(data(), size_, value);
				std::uninitialized_fill_n(data() + size_, n - size_, value);
			}
			set_size(n);
		}

		template<std::input_iterator InputIterator>
			requires std::is_constructible_v<T, typename std::iterator_traits<InputIterator>::value_type>
		constexpr void assign(InputIterator first, InputIterator last) {
			const size_type n = static_cast<size_type>(std::distance(first, last));
			if (n > N) {
				throw_bad_alloc();
			} else if (size_ >= n) {
				std::copy(first, last, data());
				std::destroy(data() + n, data() + size_);
			} else {
				std::copy(first, first + size_, data());
				std::uninitialized_copy(first + size_, last, data() + size_);
			}
			set_size(n);
		}

	#if __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L
		template<std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
		constexpr explicit inplace_vector(std::from_range_t, R&& rg) {
			if constexpr (std::ranges::sized_range<R>) {
				size_t n = std::ranges::size(rg);
				if (n > N) {
					throw_bad_alloc();
				}
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
			size_t n = std::ranges::size(rg);
			if (n > N) {
				throw_bad_alloc();
			} else if (size_ >= n) {
				std::ranges::copy(first, last, data());
				std::destroy(data() + n, data() + size_);
			} else {
				auto mid = std::ranges::next(first, size_, last);
				std::ranges::copy(first, mid, data());
				std::ranges::uninitialized_copy(mid, last, data() + size_);
			}
			set_size(n);
		}
	#endif // __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L

		// iterators

		constexpr iterator begin() noexcept { return data(); }
		constexpr iterator end() noexcept { return data() + size_; }
		constexpr const_iterator begin() const noexcept { return data(); }
		constexpr const_iterator end() const noexcept { return data() + size_; }
		constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
		constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
		constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
		constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
		constexpr const_iterator cbegin() const noexcept { return data(); }
		constexpr const_iterator cend() const noexcept { return data() + size_; }
		constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
		constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

		constexpr void resize(size_type n)
			requires std::is_default_constructible_v<T>
		{
			if (n > N) {
				throw_bad_alloc();
			} else if (n < size_) {
				std::destroy(data() + n, data() + size_);
				set_size(n);
			} else {
				std::uninitialized_value_construct(data() + size_, data() + n);
				set_size(size_ + n);
			}
		}

		constexpr void resize(size_type n, const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			if (n > N) {
				throw_bad_alloc();
			} else if (n < size_) {
				std::destroy(data() + n, data() + size_);
				set_size(n);
			} else {
				std::uninitialized_fill(data() + size_, data() + n, value);
				set_size(size_ + n);
			}
		}

		static constexpr void reserve(size_type n) {
			if (n > N) {
				throw_bad_alloc();
			}
		}
		static constexpr void shrink_to_fit() noexcept {}

		// element access

		constexpr reference operator[](size_type pos) {
			PLUGIFY_ASSERT(pos < size(), "index out of bounds");
			return data()[pos];
		}
		constexpr reference front() {
			PLUGIFY_ASSERT(!empty(), "called on an empty vector");
			return data()[0];
		}
		constexpr reference back() {
			PLUGIFY_ASSERT(!empty(), "called on an empty vector");
			return data()[size_ - 1];
		}

		constexpr const_reference operator[](size_type pos) const {
			PLUGIFY_ASSERT(pos < size(), "index out of bounds");
			return data()[pos];
		}
		constexpr const_reference front() const {
			PLUGIFY_ASSERT(!empty(), "called on an empty vector");
			return data()[0];
		}
		constexpr const_reference back() const {
			PLUGIFY_ASSERT(!empty(), "called on an empty vector");
			return data()[size_ - 1];
		}

		constexpr reference at(size_type i) {
			if (i >= size_) {
				throw_out_of_range();
			}
			return data()[i];
		}
		constexpr const_reference at(size_type i) const {
			if (i >= size_) {
				throw_out_of_range();
			}
			return data()[i];
		}

		// [inplace.vector.data]

		constexpr T* data() noexcept { return this->base_data(); }
		constexpr const T* data() const noexcept { return this->base_data(); }
		constexpr size_type size() const noexcept { return size_; }
		static constexpr size_type max_size() noexcept { return N; }
		static constexpr size_type capacity() noexcept { return N; }
		[[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; };

		// [inplace.vector.modifiers]

		template<class... Args>
			requires std::is_constructible_v<T, Args...>
		value_type& unchecked_emplace_back(Args&&... args) {
			// Precondition: (size_ < N)
			value_type* p = data() + size_;
			p = std::construct_at(p, std::forward<Args>(args)...);
			set_size(size_ + 1);
			return *p;
		}
		value_type& unchecked_push_back(const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			return unchecked_emplace_back(value);
		}
		value_type& unchecked_push_back(value_type&& value)
			requires std::is_move_constructible_v<T>
		{
			return unchecked_emplace_back(static_cast<value_type&&>(value));
		}

		template<class... Args>
			requires std::is_constructible_v<T, Args...>
		constexpr value_type* try_emplace_back(Args&&... args) {
			if (size_ == N) {
				return nullptr;
			}
			return std::addressof(unchecked_emplace_back(static_cast<Args&&>(args)...));
		}
		constexpr value_type* try_push_back(const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			return try_emplace_back(value);
		}
		constexpr value_type* try_push_back(value_type&& value)
			requires std::is_move_constructible_v<T>
		{
			return try_emplace_back(static_cast<value_type&&>(value));
		}

		template<class... Args>
			requires std::is_constructible_v<T, Args...>
		value_type& emplace_back(Args&&... args) {
			if (size_ == N) {
				throw_bad_alloc();
			}
			return unchecked_emplace_back(static_cast<Args&&>(args)...);
		}
		value_type& push_back(const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			return emplace_back(value);
		}
		value_type& push_back(value_type&& value)
			requires std::is_move_constructible_v<T>
		{
			return emplace_back(static_cast<value_type&&>(value));
		}

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
			std::destroy_at(data() + size_ - 1);
			set_size(size_ - 1);
		}

		template<class... Args>
			requires std::is_constructible_v<T, Args...>
		iterator emplace(const_iterator pos, Args&&... args) {
			auto it = iterator(pos);
			emplace_back(static_cast<Args&&>(args)...);
			std::rotate(it, end() - 1, end());
			return it;
		}
		iterator insert(const_iterator pos, const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			return emplace(pos, value);
		}
		iterator insert(const_iterator pos, value_type&& value)
			requires std::is_move_constructible_v<T>
		{
			return emplace(pos, static_cast<value_type&&>(value));
		}

		iterator insert(const_iterator pos, size_type n, const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			if (N - size_ < n) {
				throw_bad_alloc();
			}
			auto it = iterator(pos);
			auto oldend = end();
	#if defined(__cpp_lib_trivially_relocatable)
			// Open a window and fill in-place; if filling fails, close the window again.
			if constexpr (std::is_trivially_relocatable_v<value_type>) {
				std::uninitialized_relocate_backward(it, oldend, oldend + n);
				try {
					std::uninitialized_fill_n(it, n, value);
					set_size(size_ + n);
				} catch (...) {
					std::uninitialized_relocate(it + n, oldend + n, it);
					throw;
				}
				return it;
			}
	#endif
			// Fill at the end of the vector, then rotate into place.
			std::uninitialized_fill_n(oldend, n, value);
			set_size(size_ + n);
			std::rotate(it, oldend, oldend + n);
			return it;
		}

		template<std::input_iterator InputIterator>
			requires (std::is_constructible_v<T, typename std::iterator_traits<InputIterator>::value_type> && !std::is_const_v<T>)
		iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
			auto it = iterator(pos);
			auto oldend = end();
			if constexpr (std::random_access_iterator<InputIterator>) {
				size_type n = static_cast<size_type>(std::distance(first, last));
				if (N - size_ < n) {
					throw_bad_alloc();
				}
	#if defined(__cpp_lib_trivially_relocatable)
				// Open a window and fill in-place; if filling fails, close the window again.
				if constexpr (std::is_trivially_relocatable_v<value_type>) {
					std::uninitialized_relocate_backward(it, oldend, oldend + n);
					try {
						std::uninitialized_copy_n(first, n, it);
						set_size(size_ + n);
					} catch (...) {
						std::uninitialized_relocate(it + n, oldend + n, it);
						throw;
					}
					return it;
				}
	#endif
				// Fill at the end of the vector, then rotate into place.
				std::uninitialized_copy_n(first, n, oldend);
				set_size(size_ + n);
				std::rotate(it, oldend, oldend + n);
			} else {
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
				if (N - size_ < n) {
					throw_bad_alloc();
				}
	#if defined(__cpp_lib_trivially_relocatable)
				// Open a window and fill in-place; if filling fails, close the window again.
				if constexpr (std::is_trivially_relocatable_v<value_type>) {
					std::uninitialized_relocate_backward(it, oldend, oldend + n);
					try {
						std::ranges::uninitialized_copy_n(std::ranges::begin(rg), n, it, std::unreachable_sentinel);
						set_size(size_ + n);
					} catch (...) {
						std::uninitialized_relocate(it + n, oldend + n, it);
						throw;
					}
					return it;
				}
	#endif
				// Fill at the end of the vector, then rotate into place.
				std::ranges::uninitialized_copy_n(std::ranges::begin(rg), n, oldend, std::unreachable_sentinel);
				set_size(size_ + n);
				std::rotate(it, oldend, oldend + n);
			} else {
				auto [rgend, newend] = std::ranges::uninitialized_copy(rg, std::ranges::subrange(oldend, data() + N));
				if (rgend != std::ranges::end(rg)) {
					std::destroy(oldend, newend);
					throw_bad_alloc();
				} else {
					set_size(newend - data());
					std::rotate(it, oldend, newend);
				}
			}
			return it;
		}
	#endif // __cpp_lib_ranges >= 201911L && __cpp_lib_ranges_to_container >= 202202L

		iterator insert(const_iterator pos, std::initializer_list<value_type> il)
			requires (std::is_copy_constructible_v<T> && !std::is_const_v<T>)
		{
			return insert(pos, il.begin(), il.end());
		}

		iterator erase(const_iterator pos)
			requires (!std::is_const_v<T>)
		{
			auto it = iterator(pos);
			auto oldend = end();
	#if defined(__cpp_lib_trivially_relocatable)
			if constexpr (std::is_trivially_relocatable_v<value_type>) {
				std::destroy_at(it);
				std::uninitialized_relocate(it + 1, oldend, it);
				set_size(size_ - 1);
				return it;
			}
	#endif
			std::move(it + 1, oldend, it);
			std::destroy_at(oldend - 1);
			set_size(size_ - 1);
			return it;
		}

		iterator erase(const_iterator first, const_iterator last)
			requires (!std::is_const_v<T>)
		{
			auto ifirst = iterator(first);
			auto ilast = iterator(last);
			auto n = static_cast<size_type>(std::distance(ifirst, ilast));
			if (n != 0) {
				auto oldend = end();
	#if defined(__cpp_lib_trivially_relocatable)
				if constexpr (std::is_trivially_relocatable_v<value_type>) {
					std::destroy(ifirst, ilast);
					std::uninitialized_relocate(ilast, oldend, ifirst);
					set_size(size_ - n);
					return ifirst;
				}
	#endif // __cpp_lib_trivially_relocatable
				std::destroy(std::move(ilast, oldend, ifirst), oldend);
				set_size(size_ - n);
			}
			return ifirst;
		}

		constexpr void clear() noexcept {
			std::destroy(data(), data() + size_);
			set_size(0);
		}

		constexpr void swap(inplace_vector& b)
			noexcept(N == 0 || (std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T>))
			requires (!std::is_const_v<T>)
		{
			auto& a = *this;
			if (a.size_ < b.size_) {
				b.swap(a);
			} else {
				std::swap_ranges(a.data(), a.data() + b.size_, b.data());
	#if defined(__cpp_lib_trivially_relocatable)
				size_t n = a.size_;
				a.set_size(b.size_);
				std::uninitialized_relocate(a.data() + b.size_, a.data() + n, b.data() + b.size_);
				b.set_size(n);
	#else
				std::uninitialized_move(a.data() + b.size_, a.data() + a.size_, b.data() + b.size_);
				std::destroy(a.data() + b.size_, a.data() + a.size_);
				if constexpr (N != 0) {
					std::swap(a.size_, b.size_);
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
			const T* adata = a.data();
			const T* bdata = b.data();
			size_t n = (a.size_ < b.size_) ? a.size_ : b.size_;
			for (size_t i = 0; i < n; ++i) {
				if (adata[i] < bdata[i]) {
					return true;
				} else if (bdata[i] < adata[i]) {
					return false;
				}
			}
			return (a.size_ < b.size_);
		}
		constexpr friend bool operator>(const inplace_vector& a, const inplace_vector& b) { return (b < a); }
		constexpr friend bool operator<=(const inplace_vector& a, const inplace_vector& b) { return !(b < a); }
		constexpr friend bool operator>=(const inplace_vector& a, const inplace_vector& b) { return !(a < b); }
		constexpr friend bool operator!=(const inplace_vector& a, const inplace_vector& b) { return !(a == b); }
	#endif

	private:
		[[noreturn]] static void throw_bad_alloc() {
			PLUGIFY_THROW("memory size would exceed capacity()", std::bad_alloc);
		}

		[[noreturn]] static void throw_out_of_range() {
			PLUGIFY_THROW("input index is out of bounds", std::out_of_range);
		}
	};
} // namespace plg

namespace std {
	template<class T, std::size_t N>
	using inplace_vector = plg::inplace_vector<T, N>;
} // namespace std

#endif