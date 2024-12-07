#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <memory>
#include <initializer_list>
#include <algorithm>
#include <span>
#include <limits>
#include <optional>

#include <cstdint>
#include <cstddef>
#include <cstring>

#if PLUGIFY_VECTOR_CONTAINERS_RANGES && (__cplusplus <= 202002L || !__has_include(<ranges>) || !defined(__cpp_lib_containers_ranges))
#  undef PLUGIFY_STRING_CONTAINERS_RANGES
#  define PLUGIFY_STRING_CONTAINERS_RANGES 0
#endif

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
#  include <ranges>
#endif

#include <plugify/macro.hpp>

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
		struct initialized_value_tag {};

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<typename Range, typename Type>
		concept vector_compatible_range = std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, Type>;
#endif
	} // namespace detail

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
		PLUGIFY_NO_UNIQUE_ADDRESS
		allocator_type _allocator;
		pointer _begin;
		pointer _end;
		pointer _capacity;

	private:
		constexpr static size_type growth_factor = 2; // When resizing, what number to scale by

		constexpr void copy_constructor(const vector& other) {
			const size_type capacity = other.capacity();
			_begin = allocator_traits::allocate(_allocator, capacity);
			std::uninitialized_copy(other.begin(), other.end(), begin());
			_end = _begin + other.size();
			_capacity = _begin + capacity;
		}

		template<std::input_iterator InputIterator>
		constexpr void range_constructor(InputIterator first, InputIterator last) {
			const size_type count = static_cast<size_type>(std::distance(first, last));
			_begin = allocator_traits::allocate(_allocator, count);
			std::uninitialized_copy(first, last, _begin);
			_capacity = _begin + count;
			_end = _begin + count;
		}

		[[nodiscard]] constexpr bool is_full() const {
			return _end == _capacity;
		}

		[[nodiscard]] constexpr size_type calculate_new_capacity() const {
			const size_type old_capacity = capacity();
			return old_capacity == 0 ? 1 : growth_factor * old_capacity;
		}

		[[nodiscard]] constexpr iterator const_iterator_cast(const_iterator iter) noexcept {
			return begin() + (iter - cbegin());
		}

		constexpr void reallocate(size_type new_capacity) {
			reallocate(new_capacity, [](pointer const) {});
		}

		template<typename F>
		constexpr void reallocate(size_type new_capacity, const F& construct) {
			const size_type old_size = size();
			const size_type old_capacity = capacity();
			PLUGIFY_ASSERT(new_capacity >= old_size, "plg::vector::reallocate(): resulted vector size would exceed size()", std::length_error);
			if (new_capacity == old_capacity)
				return;

			pointer const new_begin = allocator_traits::allocate(_allocator, new_capacity);
			construct(new_begin);
			std::uninitialized_move(_begin, _end, new_begin);
			std::destroy(_begin, _end);
			allocator_traits::deallocate(_allocator, _begin, capacity());
			_begin = new_begin;
			_end = _begin + old_size;
			_capacity = _begin + new_capacity;
		}

		template<typename F>
		constexpr void emplace_at_end(const F& construct) {
			if (is_full()) {
				reallocate(calculate_new_capacity(), construct);
			} else {
				construct(_begin);
			}
		}

		template<typename V>
		constexpr void resize_to(size_type count, const V& value) {
			if (count < size()) {
				std::destroy(_begin + count, _end);
				_end = _begin + count;
			} else if (count > size()) {
				const size_type old_size = size();
				auto construct = [&](pointer const data) {
					if constexpr (std::is_same_v<V, T>) {
						std::uninitialized_fill(data + old_size, data + count, value);
					} else {
						std::uninitialized_value_construct(data + old_size, data + count);
					}
				};
				if (count > capacity()) {
					reallocate(count, construct);
				} else {
					construct(_begin);
				}
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
			: _allocator(Allocator()), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
		}

		constexpr explicit vector(const Allocator& allocator) noexcept
			: _allocator(allocator), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
		}

		constexpr vector(size_type count, const T& value, const Allocator& allocator = Allocator())
			: _allocator(allocator), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
			PLUGIFY_ASSERT(count <= max_size(), "plg::vector::vector(): constructed vector size would exceed max_size()", std::length_error);
			_begin = allocator_traits::allocate(_allocator, count);
			std::uninitialized_fill_n(_begin, count, value);
			_capacity = _begin + count;
			_end = _begin + count;
		}

		constexpr explicit vector(size_type count, const Allocator& allocator = Allocator())
			: _allocator(allocator), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
			PLUGIFY_ASSERT(count <= max_size(), "plg::vector::vector(): constructed vector size would exceed max_size()", std::length_error);
			_begin = allocator_traits::allocate(_allocator, count);
			std::uninitialized_value_construct_n(_begin, count);
			_capacity = _begin + count;
			_end = _begin + count;
		}

		template<std::input_iterator InputIterator>
		constexpr vector(InputIterator first, InputIterator last, const Allocator& allocator = Allocator())
			: _allocator(allocator), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
			PLUGIFY_ASSERT(static_cast<size_type>(std::distance(first, last)) <= max_size(), "plg::vector::vector(): constructed vector size would exceed max_size()", std::length_error);
			range_constructor(first, last);
		}

		constexpr vector(const vector& other)
			: _allocator(other.get_allocator()), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
			copy_constructor(other);
		}

		constexpr vector(const vector& other, const Allocator& allocator)
			: _allocator(allocator), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
			copy_constructor(other);
		}

		constexpr vector(vector&& other) noexcept(std::is_nothrow_move_constructible<Allocator>::value)
			: _allocator(Allocator()), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
			swap(other);
		}

		constexpr vector(vector&& other, const Allocator& allocator)
			: _allocator(allocator), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
			if constexpr (allocator_traits::is_always_equal::value) {
				swap_without_allocator(std::move(other));
			} else {
				if (get_allocator() == other.get_allocator()) {
					swap_without_allocator(std::move(other));
				} else {
					const size_type capacity = other.capacity();
					_begin = allocator_traits::allocate(_allocator, capacity);
					std::uninitialized_move(other.begin(), other.end(), begin());
					_end = _begin + other.size();
					_capacity = _begin + capacity;
				}
			}
		}

		constexpr vector(std::initializer_list<T> list, const Allocator& allocator = Allocator())
			: _allocator(allocator), _begin{nullptr}, _end{nullptr}, _capacity{nullptr} {
			PLUGIFY_ASSERT(list.size() <= max_size(), "plg::vector::vector(): constructed vector size would exceed max_size()", std::length_error);
			range_constructor(list.begin(), list.end());
		}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<T> Range>
		constexpr vector(std::from_range_t, Range&& range, const Allocator& alloc = Allocator())
			: vector(std::ranges::begin(range), std::ranges::end(range), alloc) {}
#endif // PLUGIFY_VECTOR_CONTAINERS_RANGES

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

			clear();
			if constexpr (allocator_traits::propagate_on_container_copy_assignment::value) {
				if constexpr (!allocator_traits::is_always_equal::value) {
					if (get_allocator() != other.get_allocator()) {
						allocator_traits::deallocate(_allocator, _begin, capacity());
					}
				}
				_allocator = other.get_allocator();
			}

			assign(other.begin(), other.end());
			return *this;
		}

		constexpr vector& operator=(vector&& other) noexcept(
				std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
				std::allocator_traits<Allocator>::is_always_equal::value) {
			if (this == &other) [[unlikely]] {
				return *this;
			}

			clear();
			if constexpr (allocator_traits::propagate_on_container_move_assignment::value) {
				if constexpr (!allocator_traits::is_always_equal::value) {
					if (get_allocator() != other.get_allocator()) {
						allocator_traits::deallocate(_allocator, _begin, capacity());
					}
				}
				_allocator = other.get_allocator();
			}

			if constexpr (allocator_traits::propagate_on_container_move_assignment::value || allocator_traits::is_always_equal::value) {
				swap_without_allocator(std::move(other));
			} else {
				if (get_allocator() == other.get_allocator()) {
					swap_without_allocator(std::move(other));
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
			PLUGIFY_ASSERT(count <= max_size(), "plg::vector::assign(): resulted vector size would exceed max_size()", std::length_error);
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
			const size_type count = static_cast<size_type>(std::distance(first, last));
			PLUGIFY_ASSERT(count <= max_size(), "plg::vector::assign(): resulted vector size would exceed max_size()", std::length_error);
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

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<T> Range>
		constexpr void assign_range(Range&& range) {
			assign(std::ranges::begin(range), std::ranges::end(range));
		}
#endif // PLUGIFY_VECTOR_CONTAINERS_RANGES

		// get_allocator
		[[nodiscard]] constexpr allocator_type get_allocator() const {
			return _allocator;
		}

		// element access
		[[nodiscard]] constexpr reference at(size_type position) {
			PLUGIFY_ASSERT(position < size(), "plg::vector::at(): input index is out of bounds", std::out_of_range);
			return *(_begin + position);
		}

		[[nodiscard]] constexpr const_reference at(size_type position) const {
			PLUGIFY_ASSERT(position < size(), "plg::vector::at(): input index is out of bounds", std::out_of_range);
			return *(_begin + position);
		}

		[[nodiscard]] constexpr reference operator[](size_type position) noexcept {
			return *(_begin + position);
		}

		[[nodiscard]] constexpr const_reference operator[](size_type position) const noexcept {
			return *(_begin + position);
		}

		[[nodiscard]] constexpr reference front() {
			PLUGIFY_ASSERT(!empty(), "plg::vector::front(): vector is empty", std::length_error);
			return *_begin;
		}

		[[nodiscard]] constexpr const_reference front() const {
			PLUGIFY_ASSERT(!empty(), "plg::vector::front(): vector is empty", std::length_error);
			return *_begin;
		}

		[[nodiscard]] constexpr reference back() {
			PLUGIFY_ASSERT(!empty(), "plg::vector::back(): vector is empty", std::length_error);
			return *(_end - 1);
		}

		[[nodiscard]] constexpr const_reference back() const {
			PLUGIFY_ASSERT(!empty(), "plg::vector::back(): vector is empty", std::length_error);
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
			return const_iterator(_begin);
		}

		[[nodiscard]] constexpr iterator end() noexcept {
			return iterator(_end);
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept {
			return const_iterator(_end);
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept {
			return const_iterator(_end);
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(_end);
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(_end);
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(_end);
		}

		[[nodiscard]] constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(_begin);
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(_begin);
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(_begin);
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
			PLUGIFY_ASSERT(new_capacity <= max_size(), "plg::vector::reserve(): allocated memory size would exceed max_size()", std::length_error);
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
			const size_type sz = size();
			const size_type new_size = sz + count;
			PLUGIFY_ASSERT(new_size <= max_size(), "plg::vector::insert(): resulted vector size would exceed max_size()", std::length_error);
			const size_type position_distance = static_cast<size_type>(position - cbegin());
			PLUGIFY_ASSERT(position_distance <= sz, "plg::vector::insert(): pos out of range", std::out_of_range);
			if (count != 0) {
				if (new_size > capacity()) {
					pointer const new_begin = allocator_traits::allocate(_allocator, new_size);
					pointer const old_position = _begin + position_distance;
					std::uninitialized_move(_begin, old_position, new_begin);
					pointer const new_position = new_begin + position_distance;
					std::uninitialized_fill_n(new_position, count, value);
					std::uninitialized_move(old_position, _end, new_position + count);
					std::destroy(_begin, _end);
					allocator_traits::deallocate(_allocator, _begin, capacity());
					_begin = new_begin;
					_end = _begin + new_size;
					_capacity = _end;
				} else {
					pointer const pointer_position = _begin + position_distance;
					std::uninitialized_fill_n(_end, count, value);
					_end += count;
					std::rotate(pointer_position, _end - count, _end);
				}
			}
			return begin() + position_distance;
		}

		template<std::input_iterator InputIterator>
		constexpr iterator insert(const_iterator position, InputIterator first, InputIterator last) {
			const size_type sz = size();
			const size_type count = static_cast<size_type>(std::distance(first, last));
			const size_type new_size = sz + count;
			PLUGIFY_ASSERT(new_size <= max_size(), "plg::vector::insert(): resulted vector size would exceed max_size()", std::length_error);
			const size_type position_distance = static_cast<size_type>(position - cbegin());
			PLUGIFY_ASSERT(position_distance <= sz, "plg::vector::insert(): pos out of range", std::out_of_range);
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
					std::uninitialized_copy(first, last, _end);
					_end += count;
					std::rotate(pointer_position, _end - count, _end);
				}
			}
			return begin() + position_distance;
		}

		constexpr iterator insert(const_iterator position, std::initializer_list<T> list) {
			return insert(position, list.begin(), list.end());
		}

#if PLUGIFY_VECTOR_CONTAINERS_RANGES
		template<detail::vector_compatible_range<T> Range>
		constexpr iterator insert_range(const_iterator pos, Range&& range) {
			return insert(pos - begin(), std::ranges::begin(range), std::ranges::end(range));
		}
#endif // PLUGIFY_VECTOR_CONTAINERS_RANGES

		template<typename... Args>
		iterator emplace(const_iterator position, Args&&... args) {
			const size_type sz = size();
			const size_type new_size = sz + 1;
			PLUGIFY_ASSERT(new_size <= max_size(), "plg::vector::emplace(): resulted vector size would exceed max_size()", std::length_error);
			const size_type position_distance = static_cast<size_type>(position - cbegin());
			PLUGIFY_ASSERT(position_distance <= sz, "plg::vector::emplace(): pos out of range", std::out_of_range);
			if (position == cend()) {
				emplace_back(std::forward<Args>(args)...);
			} else {
				if (is_full()) {
					const size_type new_capacity = calculate_new_capacity();
					pointer const new_begin = allocator_traits::allocate(_allocator, new_capacity);
					pointer const old_position = _begin + position_distance;
					pointer const new_position = new_begin + position_distance;
					std::uninitialized_move(_begin, old_position, new_begin);
					std::construct_at(new_position, std::forward<Args>(args)...);
					std::uninitialized_move(old_position, _end, new_position + 1);
					std::destroy(_begin, _end);
					allocator_traits::deallocate(_allocator, _begin, capacity());
					_begin = new_begin;
					_end = _begin + new_size;
					_capacity = _begin + new_capacity;
				} else {
					pointer const pointer_position = _begin + position_distance;
					std::construct_at(_end, std::forward<Args>(args)...);
					++_end;
					std::rotate(pointer_position, _end - 1, _end);
				}
			}
			return begin() + position_distance;
		}

		constexpr iterator erase(const_iterator position) {
			iterator nonconst_position = const_iterator_cast(position);
			if (nonconst_position + 1 != end()) {
				std::rotate(nonconst_position, nonconst_position + 1, end());
			}
			--_end;
			std::destroy_at(_end);
			return nonconst_position;
		}

		constexpr iterator erase(const_iterator first, const_iterator last) {
			PLUGIFY_ASSERT(first <= last, "plg::vector::erase(): called with invalid range", std::out_of_range);
			iterator nonconst_first = const_iterator_cast(first);
			iterator nonconst_last = const_iterator_cast(last);
			if (nonconst_first != nonconst_last) {
				if (nonconst_last != end()) {
					std::rotate(nonconst_first, nonconst_last, end());
				}
				_end = nonconst_first.base() + static_cast<size_type>(end() - nonconst_last);
				std::destroy(_end, _end + static_cast<size_type>(std::distance(first, last)));
			}
			return nonconst_first;
		}

		constexpr void push_back(const T& value) {
			const size_type sz = size();
			PLUGIFY_ASSERT(sz + 1 <= max_size(), "plg::vector::push_back(): resulted vector size would exceed max_size()", std::length_error);
			emplace_at_end([&](pointer const data) {
				std::construct_at(data + sz, value);
			});
			++_end;
		}

		constexpr void push_back(T&& value) {
			const size_type sz = size();
			PLUGIFY_ASSERT(sz + 1 <= max_size(), "plg::vector::push_back(): resulted vector size would exceed max_size()", std::length_error);
			emplace_at_end([&](pointer const data) {
				std::construct_at(data + sz, std::move(value));
			});
			++_end;
		}

		template<typename... Args>
		constexpr reference emplace_back(Args&&... args) {
			const size_type sz = size();
			PLUGIFY_ASSERT(sz + 1 <= max_size(), "plg::vector::emplace_back(): resulted vector size would exceed max_size()", std::length_error);
			emplace_at_end([&](pointer const data) {
				std::construct_at(data + sz, std::forward<Args>(args)...);
			});
			++_end;
			return back();
		}

		constexpr void pop_back() {
			PLUGIFY_ASSERT(!empty(), "plg::vector::pop_back(): vector is empty", std::length_error);
			--_end;
			std::destroy_at(_end);
		}

		constexpr void resize(size_type count) {
			PLUGIFY_ASSERT(count <= max_size(), "plg::vector::resize(): allocated memory size would exceed max_size()", std::length_error);
			resize_to(count, detail::initialized_value_tag{});
		}

		constexpr void resize(size_type count, const T& value) {
			PLUGIFY_ASSERT(count <= max_size(), "plg::vector::resize(): allocated memory size would exceed max_size()", std::length_error);
			resize_to(count, value);
		}

		constexpr vector& operator+=(const T& value) {
			push_back(value);
			return *this;
		}

		constexpr vector& operator+=(T&& value) {
			push_back(std::move(value));
			return *this;
		}

		constexpr vector& operator+=(const vector& other) {
			insert(end(), other.begin(), other.end());
			return *this;
		}

		constexpr vector& operator+=(vector&& other) {
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
			PLUGIFY_ASSERT(size() == Size, "plg::vector::span_size(): const_span_size argument does not match size of vector", std::length_error);
			return std::span<T, Size>(data(), size());
		}

		template<size_type Size>
		[[nodiscard]] constexpr std::span<const T, Size> const_span_size() const noexcept {
			PLUGIFY_ASSERT(size() == Size, "plg::vector::const_span_size(): const_span_size argument does not match size of vector", std::length_error);
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

	namespace pmr {
		template<typename T>
		using vector = ::plg::vector<T, std::pmr::polymorphic_allocator<T>>;
	} // namespace pmr

} // namespace plg
