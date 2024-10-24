#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <memory>
#include <initializer_list>
#include <algorithm>
#include <numeric>
#include <execution>
#include <limits>
#include <iostream>
#include <ranges>

#ifndef PLUGIFY_VECTOR_EXCEPTIONS
#  if __cpp_exceptions
#    define PLUGIFY_VECTOR_EXCEPTIONS 1
#  else
#    define PLUGIFY_VECTOR_EXCEPTIONS 0
#  endif
#endif

#if PLUGIFY_VECTOR_EXCEPTIONS && (!__cpp_exceptions || !__has_include(<stdexcept>))
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
#  define _PLUGIFY_VECTOR_ASSERT(x, str, e) do { if (!(x)) throw e(str); } while (0)
#elif PLUGIFY_VECTOR_FALLBACK_ASSERT
#  include <cassert>
#  define _PLUGIFY_VECTOR_ASSERT(x, str, ...) assert(x && str)
#elif PLUGIFY_VECTOR_FALLBACK_ABORT
#  if !PLUGIFY_VECTOR_NUMERIC_CONVERSIONS
#    include <cstdlib>
#  endif
#  define _PLUGIFY_VECTOR_ASSERT(x, ...) do { if (!(x)) { std::abort(); } } while (0)
#else
#  define _PLUGIFY_VECTOR_ASSERT(x, str, ...) do { if (!(x)) { PLUGIFY_VECTOR_FALLBACK_ABORT_FUNCTION (str); { while (true) { [] { } (); } } } } while (0)
#endif

#if defined(_MSC_VER)
#define _PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define _PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

namespace plg {
	template<typename T, typename Allocator = std::allocator<T>>
	class vector {
		// Purely to make notation easier
		using alloc_traits = std::allocator_traits<Allocator>;
	public:
		using value_type = T;
		using allocator_type = Allocator;
		using size_type = typename alloc_traits::size_type;
		using difference_type = typename alloc_traits::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = typename alloc_traits::pointer;
		using const_pointer = typename alloc_traits::const_pointer;
		using iterator = T*;
		using const_iterator = const T*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	public:
		// constructors
		constexpr vector() noexcept(std::is_nothrow_default_constructible<allocator_type>::value)
			: _alloc(), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
		}
		constexpr explicit vector(const Allocator& a) noexcept
			: _alloc(a), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
		}
		constexpr vector(size_type count, const T& value, const allocator_type& a = Allocator())
			: _alloc(a), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
			_start = _alloc.allocate(count * sizeof(T));
			_finish = _end_of_storage = _start + count;
			std::uninitialized_fill_n(_start, count, value);
		}
		constexpr explicit vector(size_type count, const allocator_type& a = Allocator())
			: _alloc(a), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
			_start = _alloc.allocate(count * sizeof(T));
			_finish = _end_of_storage = _start + count;
			std::uninitialized_fill_n(_start, count, T());
		}
		// In order to avoid ambiguity with overload 3, this overload participates in overload resolution only if InputIterator satisfies LegacyInputIterator.
		// shall use SFINAE
		template<typename InputIterator> requires(std::is_base_of_v<typename std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>)
		constexpr vector(InputIterator first, InputIterator last, const allocator_type& a = Allocator())
			: _alloc(a), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
			for (auto iter = first; iter != last; ++iter) {
				push_back(*iter);
			}
		}
		constexpr vector(const vector& other)
			: _alloc(other._alloc), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
			if (other.size() > 0) {
				_start = _alloc.allocate(other.size() * sizeof(T));
				_finish = _end_of_storage = _start + other.size();
				std::uninitialized_copy(other.begin(), other.end(), _start);
			}
		}
		constexpr vector(const vector& other, const allocator_type& a)
			: _alloc(a), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
			if (other.size() > 0) {
				_start = _alloc.allocate(other.size() * sizeof(T));
				_finish = _end_of_storage = _start + other.size();
				std::uninitialized_copy(other.begin(), other.end(), _start);
			}
		}
		constexpr vector(vector&& other) noexcept
			: _alloc(std::move(other._alloc)), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
			_start = other._start;
			_finish = other._finish;
			_end_of_storage = other._end_of_storage;
			other._start = other._finish = other._end_of_storage = nullptr;
		}
		constexpr vector(vector&& other, const allocator_type& a)
			: _alloc(a), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
			if (_alloc == other._alloc) {
				_start = other._start;
				_finish = other._finish;
				_end_of_storage = other._end_of_storage;
				other._start = other._finish = other._end_of_storage = nullptr;
			} else {
				if (other.size() > 0) {
					_start = _alloc.allocate(other.size() * sizeof(T));
					std::uninitialized_move(other.begin(), other.end(), _start);
				}
			}
		}
		constexpr vector(std::initializer_list<T> il, const allocator_type& a = Allocator())
			: _alloc(a), _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
			if (il.size() > 0) {
				_start = _alloc.allocate(il.size() * sizeof(T));
				_finish = _end_of_storage = _start + il.size();
				std::uninitialized_copy(il.begin(), il.end(), _start);
			}
		}
		// destructor
		constexpr ~vector() {
			clear();
			free_all_spaces();
		}

		// operator =
		constexpr vector& operator=(const vector& other) {
			if (this == &other)
				return *this;
			clear();
			if (other.size() <= capacity()) {
				std::uninitialized_copy(other.begin(), other.end(), _start);
				_finish = _start + other.size();
			} else {
				free_all_spaces();
				_start = _alloc.allocate(other.size() * sizeof(T));
				std::uninitialized_copy(other.begin(), other.end(), _start);
				_finish = _end_of_storage = _start + other.size();
			}
			return *this;
		}
		constexpr vector& operator=(vector&& other) noexcept(
				alloc_traits::propagate_on_container_move_assignment::value ||
				alloc_traits::is_always_equal::value) {
			if (this == &other)
				return *this;
			clear();
			free_all_spaces();
			if (_alloc == other._alloc) {
				_start = other._start;
				_finish = other._finish;
				_end_of_storage = other._end_of_storage;
				other._start = other._finish = other._end_of_storage = nullptr;
			} else {
				_alloc = other._alloc;
				if (other.size() > 0) {
					_start = _alloc.allocate(other.size() * sizeof(T));
					std::uninitialized_move(other.begin(), other.end(), _start);
				}
			}
			return *this;
		}
		constexpr vector& operator=(std::initializer_list<T> il) {
			assign(il);
			return *this;
		}
		// assign
		constexpr void assign(size_type count, const T& value) {
			clear();
			if (count <= capacity()) {
				std::uninitialized_fill_n(_start, count, value);
				_finish = _start + count;
			} else {
				free_all_spaces();
				_start = _alloc.allocate(count * sizeof(T));
				std::uninitialized_fill_n(_start, count, value);
				_finish = _end_of_storage = _start + count;
			}
		}
		template<typename InputIterator> requires(std::is_base_of_v<typename std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>)
		constexpr void assign(InputIterator first, InputIterator last) {
			clear();
			free_all_spaces();
			for (; first != last; ++first) {
				push_back(*first);
			}
		}
		constexpr void assign(std::initializer_list<T> il) {
			clear();
			if (il.size() <= capacity()) {
				std::uninitialized_copy(il.begin(), il.end(), _start);
				_finish = _start + il.size();
			} else {
				free_all_spaces();
				_start = _alloc.allocate(il.size() * sizeof(T));
				std::uninitialized_copy(il.begin(), il.end(), _start);
				_finish = _end_of_storage = _start + il.size();
			}
		}
		// allocator
		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept {
			return _alloc;
		}

		// element access
		[[nodiscard]] constexpr reference at(size_type pos) {
			_PLUGIFY_VECTOR_ASSERT(pos < size(), "plg::vector::at(): input index is out of bounds", std::out_of_range);
			return *(begin() + pos);
		}
		[[nodiscard]] constexpr const_reference at(size_type pos) const {
			_PLUGIFY_VECTOR_ASSERT(pos < size(), "plg::vector::at(): input index is out of bounds", std::out_of_range);
			return *(begin() + pos);
		}
		[[nodiscard]] constexpr reference operator[](size_type pos) {
			_PLUGIFY_VECTOR_ASSERT(pos < size(), "plg::vector::operator[]: input index is out of bounds", std::out_of_range);
			return *(begin() + pos);
		}
		[[nodiscard]] constexpr const_reference operator[](size_type pos) const {
			_PLUGIFY_VECTOR_ASSERT(pos < size(), "plg::vector::operator[]: input index is out of bounds", std::out_of_range);
			return *(begin() + pos);
		}
		[[nodiscard]] constexpr reference front() {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::front(): array is empty", std::length_error);
			return *begin();
		}
		[[nodiscard]] constexpr const_reference front() const {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::front(): array is empty", std::length_error);
			return *begin();
		}
		[[nodiscard]] constexpr reference back() {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::back(): array is empty", std::length_error);
			return *(end() - 1);
		}
		[[nodiscard]] constexpr const_reference back() const {
			_PLUGIFY_VECTOR_ASSERT(!empty(), "plg::vector::back(): array is empty", std::length_error);
			return *(end() - 1);
		}
		[[nodiscard]] constexpr pointer data() noexcept {
			return _start;
		}
		[[nodiscard]] constexpr const_pointer data() const noexcept {
			return _start;
		}

		// iterators
		[[nodiscard]] constexpr iterator begin() noexcept {
			return _start;
		}
		[[nodiscard]] constexpr const_iterator begin() const noexcept {
			return _start;
		}
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept {
			return _start;
		}
		[[nodiscard]] constexpr iterator end() noexcept {
			return _finish;
		}
		[[nodiscard]] constexpr const_iterator end() const noexcept {
			return _finish;
		}
		[[nodiscard]] constexpr const_iterator cend() const noexcept {
			return _finish;
		}
		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(end());
		}
		[[nodiscard]] constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(begin());
		}

		// size and capacity
		[[nodiscard]] constexpr bool empty() const noexcept {
			return begin() == end();
		}
		[[nodiscard]] constexpr size_type size() const noexcept {
			return size_type(end() - begin());
		}
		[[nodiscard]] constexpr size_type max_size() const noexcept {
			return std::numeric_limits<difference_type>::max();
		}
		constexpr void reserve(size_type new_cap) {
			if (new_cap > capacity()) {
				if (new_cap < 2 * capacity()) {
					new_cap = 2 * capacity();
				}
				adjust_capacity(new_cap);
			}
		}
		[[nodiscard]] constexpr size_type capacity() const noexcept {
			return _end_of_storage - _start;
		}
		constexpr void shrink_to_fit() {
			// do shrink only when capacity > 2*size(), just a choice of implementation, no sepcial meaning
			if (capacity() > 2 * size()) {
				adjust_capacity(size());
			}
		}

		// modifiers
		constexpr void clear() noexcept {
			erase_range(_start, _finish);
			_finish = _start;
		}
		constexpr iterator insert(const_iterator pos, const T& value) {
			size_type idx = (size_type) (pos - _start);
			move_backward(idx, 1);
			*(_start + idx) = value;
			return _start + idx;
		}
		constexpr iterator insert(const_iterator pos, T&& value) {
			size_type idx = (size_type) (pos - _start);
			move_backward(idx, 1);
			*(_start + idx) = std::move(value);
			return _start + idx;
		}
		constexpr iterator insert(const_iterator pos, size_type count, const T& value) {
			size_type idx = (size_type) (pos - _start);
			move_backward(idx, count);
			fill_range(_start + idx, _start + idx + count, value);
			return _start + idx;
		}
		template<typename InputIterator> requires(std::is_base_of_v<typename std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>)
		constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
			size_type idx = (size_type) (pos - _start);
			move_backward(idx, last - first);
			copy_range(first, last, _start + idx);
			return _start + idx;
		}
		constexpr iterator insert(const_iterator pos, std::initializer_list<T> il) {
			size_type idx = (size_type) (pos - _start);
			if (il.size() > 0) {
				move_backward(idx, il.size());
				copy_range(il.begin(), il.end(), _start + idx);
			}
			return _start + idx;
		}
		template<typename... Args>
		constexpr iterator emplace(const_iterator pos, Args&&... args) {
			size_type idx = (size_type) (pos - _start);
			move_backward(idx, 1);
			alloc_traits::construct(_alloc, _start + idx, std::forward<Args>(args)...);
			return _start + idx;
		}
		constexpr iterator erase(const_iterator pos) {
			move_forward(pos + 1, 1);
			return (iterator) pos;
		}
		constexpr iterator erase(const_iterator first, const_iterator last) {
			move_forward(last, last - first);
			return (iterator) first;
		}
		constexpr void push_back(const T& value) {
			if (size() == capacity()) {
				adjust_capacity();// default ajust to double of size()
			}
			alloc_traits::construct(_alloc, _finish, value);
			++_finish;
		}
		constexpr void push_back(T&& value) {
			if (size() == capacity()) {
				adjust_capacity();
			}
			alloc_traits::construct(_alloc, _finish, std::forward<T>(value));
			++_finish;
		}
		template<typename... Args>
		constexpr reference emplace_back(Args&&... args) {
			if (size() == capacity()) {
				adjust_capacity();
			}
			alloc_traits::construct(_alloc, _finish, std::forward<Args>(args)...);
			++_finish;
			return *_finish;
		}
		constexpr void pop_back() {
			assert(!empty());
			_alloc.destroy(_finish - 1);
			--_finish;
		}
		constexpr void resize(size_type count) {
			if (count == size()) {
				return;
			} else if (count < size()) {
				erase_range(_start + count, _finish);
				_finish = _start + count;
			} else if (count <= capacity()) {
				std::uninitialized_fill(_finish, _start + count, T());
				_finish = _start + count;
			} else {
				adjust_capacity(count);
				std::uninitialized_fill(_finish, _start + count, T());
				_finish = _start + count;
			}
		}
		constexpr void resize(size_type count, const value_type& value) {
			if (count == size()) {
				return;
			} else if (count < size()) {
				erase_range(_start + count, _finish);
				_finish = _start + count;
			} else if (count <= capacity()) {
				std::uninitialized_fill(_finish, _start + count, value);
				_finish = _start + count;
			} else {
				adjust_capacity(count);
				std::uninitialized_fill(_finish, _start + count, value);
				_finish = _start + count;
			}
		}
		constexpr void swap(vector& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value || std::allocator_traits<Allocator>::is_always_equal::value) {
			if (this == &other)
				return;
			using std::swap;
			swap(_alloc, other._alloc);
			swap(_start, other._start);
			swap(_finish, other._finish);
			swap(_end_of_storage, other._end_of_storage);
		}

	private:
		// auxiliary functions
		template<typename InputIterator>
		void copy_range(InputIterator first, InputIterator last, iterator dest) {
			for (; first != last; ++first, ++dest) {
				*dest = *first;
			}
		}
		// move from front to back
		void move_range(const_iterator first, const_iterator last, iterator dest) {
			for (; first != last; ++first, ++dest) {
				*dest = std::move(*first);
			}
		}
		// move from back to front
		void move_range_from_back_to_front(const_iterator first, const_iterator last, iterator dest) {
			auto count = last - first;
			auto rfirst = std::make_reverse_iterator(first);
			auto iter = std::make_reverse_iterator(last);
			for (auto rdest = std::make_reverse_iterator(dest + count); iter != rfirst; ++iter, ++rdest) {
				*rdest = std::move(*iter);
			}
		}
		void fill_range(iterator first, iterator last, const T& value) {
			for (auto dest = first; dest != last; ++dest) {
				*dest = value;
			}
		}
		// destroy a range of elements in reverse order.
		void erase_range(iterator first, iterator last) {
			for (auto iter = last; iter != first;) {
				alloc_traits::destroy(_alloc, &*(--iter));
			}
		}
		// free all spaces for another allocation
		void free_all_spaces() {
			if (_start) {
				_alloc.deallocate(_start, capacity());
			}
			_start = _finish = _end_of_storage = nullptr;
		}
		// adjust capacity: less or more, reserve, shrink_to_fit, etc
		// new_cap must greater than or equal to size(), default adjust to double of current size()
		void adjust_capacity(size_type new_cap = 0) {
			if (new_cap <= size()) {
				new_cap = 2 * size();
				new_cap = new_cap > 0 ? new_cap : 1;// make sure at least for 1 elements
			}
			pointer new_start = _alloc.allocate(new_cap * sizeof(T));
			pointer new_finish = new_start + size();
			pointer new_end_of_storage = new_start + new_cap;
			std::uninitialized_move(_start, _finish, new_start);
			clear();
			free_all_spaces();
			_start = new_start;
			_finish = new_finish;
			_end_of_storage = new_end_of_storage;
		}
		// move elements after(include) idx backward specific location.
		// do the copy(move if possible), and ajust capacity if necessary
		// for insert
		void move_backward(size_type idx, size_type count) {
			while (size() + count > capacity()) {
				adjust_capacity(max(2 * size(), size() + count));
			}
			if (idx + count >= size()) {
				std::uninitialized_copy(_start + idx, _finish, _start + idx + count);
			} else {
				std::uninitialized_copy(_finish - count, _finish, _finish);
				// move elements from back to front
				move_range_from_back_to_front(_start + idx, _finish - count, _start + idx + count);
			}
			_finish += count;
		}
		// move elements forward
		// called after erasing
		void move_forward(const_iterator first, size_type count) {
			move_range(first, _finish, (iterator) (first - count));
			erase_range(_finish - count, _finish);
			_finish -= count;
		}

	private:
		pointer _start;
		pointer _finish; // off the end
		pointer _end_of_storage; // off the capacity
		_PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS
		allocator_type _alloc;
	};

	namespace details {
		// non-member operations
		// a non-standard compare function for vector
		// equal 0 less -1 greater 1
		template<typename T, typename Allocator>
		constexpr int cmp_vector(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
			auto iter1 = lhs.begin();
			auto iter2 = rhs.begin();
			for (; iter1 != lhs.end() && iter2 != rhs.end(); ++iter1, ++iter2)
			{
				if (*iter1 == *iter2)
				{
					continue;
				}
				return *iter1 > *iter2 ? 1 : -1;
			}
			if (iter1 != lhs.end())
			{
				return 1;
			}
			if (iter2 != rhs.end())
			{
				return -1;
			}
			return 0;
		}
	} // namespace details

	// comparisons
	template<typename T, typename Allocator>
	constexpr bool operator==(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return details::cmp_vector(lhs, rhs) == 0;
	}
	template<typename T, typename Allocator>
	constexpr bool operator!=(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return details::cmp_vector(lhs, rhs) != 0;
	}
	template<typename T, typename Allocator>
	constexpr bool operator<(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return details::cmp_vector(lhs, rhs) < 0;
	}
	template<typename T, typename Allocator>
	constexpr bool operator<=(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return details::cmp_vector(lhs, rhs) <= 0;
	}
	template<typename T, typename Allocator>
	constexpr bool operator>(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return details::cmp_vector(lhs, rhs) > 0;
	}
	template<typename T, typename Allocator>
	constexpr bool operator>=(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return details::cmp_vector(lhs, rhs) >= 0;
	}

	// global swap for vector
	template<typename T, typename Allocator>
	constexpr void swap(vector<T, Allocator>& lhs, vector<T, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
		lhs.swap(rhs);
	}

	namespace pmr {
		template<typename T>
		using vector = ::plg::vector<T, std::pmr::polymorphic_allocator<T>>;
	} // namespace pmr

} // namespace plg