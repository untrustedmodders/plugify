#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <memory_resource>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>

#include "plg/allocator.hpp"
#include "plg/guards.hpp"
#include "plg/split_buffer.hpp"
#include "plg/uninitialized.hpp"

// Just in case, because we can't ignore some warnings from `-Wpedantic` (about zero size arrays and anonymous structs when gnu extensions are disabled) on gcc
#if PLUGIFY_COMPILER_CLANG
#  pragma clang system_header
#elif PLUGIFY_COMPILER_GCC
#  pragma GCC system_header
#endif

// from https://github.com/llvm/llvm-project/blob/main/libcxx/include/vector
namespace plg {
	namespace detail {
		template <class T, class Alloc>
		struct temp_value {
			using allocator_traits = std::allocator_traits<Alloc>;

			union {
				T v;
			};
			PLUGIFY_NO_UNIQUE_ADDRESS Alloc& a;

			constexpr T* addr() {
				return std::addressof(v);
			}

			constexpr T& get() {
				return *addr();
			}

			template <class... Args>
			PLUGIFY_NO_CFI constexpr
			explicit temp_value(Alloc& alloc, Args&&... args)
				: a(alloc) {
				allocator_traits::construct(a, addr(), std::forward<Args>(args)...);
			}

			constexpr ~temp_value() {
				allocator_traits::destroy(a, addr());
			}
		};
	}

	template <class T, class Allocator = allocator<T>>
	class vector {
		template <class U, class Alloc>
		using split_buffer = split_buffer<U, Alloc, split_buffer_pointer_layout>;
	public:
		//
		// Types
		//
		using value_type = T;
		using allocator_type = Allocator;
		using alloc_traits = std::allocator_traits<allocator_type>;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = typename alloc_traits::size_type;
		using difference_type = typename alloc_traits::difference_type;
		using pointer = typename alloc_traits::pointer;
		using const_pointer = typename alloc_traits::const_pointer;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		//static_assert(std::check_valid_allocator<allocator_type>::value, "");
		static_assert(
			std::is_same_v<typename allocator_type::value_type, value_type>,
			"Allocator::value_type must be same type as value_type"
		);

		//
		// [vector.cons], construct/copy/destroy
		//
		constexpr vector() noexcept(std::is_nothrow_default_constructible_v<allocator_type>) = default;

		constexpr explicit vector(const allocator_type& a) noexcept
			: alloc_(a) {
		}

		constexpr explicit vector(size_type n) {
			auto guard = make_exception_guard(destroy_vector(*this));
			if (n > 0) {
				vallocate(n);
				construct_at_end(n);
			}
			guard.complete();
		}

		constexpr
			explicit vector(size_type n, const allocator_type& a)
			: alloc_(a) {
			auto guard = make_exception_guard(destroy_vector(*this));
			if (n > 0) {
				vallocate(n);
				construct_at_end(n);
			}
			guard.complete();
		}

		constexpr vector(size_type n, const value_type& x) {
			auto guard = make_exception_guard(destroy_vector(*this));
			if (n > 0) {
				vallocate(n);
				construct_at_end(n, x);
			}
			guard.complete();
		}

		constexpr
		vector(size_type n, const value_type& x, const allocator_type& a)
			: alloc_(a) {
			auto guard = make_exception_guard(destroy_vector(*this));
			if (n > 0) {
				vallocate(n);
				construct_at_end(n, x);
			}
			guard.complete();
		}

		template<std::input_iterator InputIterator>
		constexpr
		vector(InputIterator first, InputIterator last) {
			init_with_sentinel(first, last);
		}

		template<std::input_iterator InputIterator>
		constexpr
		vector(InputIterator first, InputIterator last, const allocator_type& a)
			: alloc_(a) {
			init_with_sentinel(first, last);
		}

		template <std::forward_iterator ForwardIterator>
		constexpr
		vector(ForwardIterator first, ForwardIterator last) {
			size_type n = static_cast<size_type>(std::distance(first, last));
			init_with_size(first, last, n);
		}

		template <std::forward_iterator ForwardIterator>
		constexpr
		vector(ForwardIterator first, ForwardIterator last, const allocator_type& a)
			: alloc_(a) {
			size_type n = static_cast<size_type>(std::distance(first, last));
			init_with_size(first, last, n);
		}

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<T> Range>
		constexpr vector(
			std::from_range_t,
			Range&& range,
			const allocator_type& alloc = allocator_type()
		) : alloc_(alloc) {
			if constexpr (std::ranges::forward_range<Range> || std::ranges::sized_range<Range>) {
				auto n = static_cast<size_type>(std::ranges::distance(range));
				init_with_size(std::ranges::begin(range), std::ranges::end(range), n);

			} else {
				init_with_sentinel(std::ranges::begin(range), std::ranges::end(range));
			}
		}
#endif

	private:
		class destroy_vector {
		public:
			constexpr explicit destroy_vector(vector& vec)
				: vec_(vec) {
			}

			constexpr void operator()() {
				if (vec_.begin_ != nullptr) {
					vec_.clear();
					vec_.annotate_delete();
					alloc_traits::deallocate(vec_.alloc_, vec_.begin_, vec_.capacity());
				}
			}

		private:
			vector& vec_;
		};

	public:
		constexpr ~vector() {
			destroy_vector (*this)();
		}

		constexpr vector(const vector& x)
			: alloc_(alloc_traits::select_on_container_copy_construction(x.alloc_)) {
			init_with_size(x.begin_, x.end_, x.size());
		}

		constexpr
		vector(const vector& x, const std::type_identity_t<allocator_type>& a)
			: alloc_(a) {
			init_with_size(x.begin_, x.end_, x.size());
		}

		constexpr vector& operator=(const vector& x);

		constexpr vector(std::initializer_list<value_type> il) {
			init_with_size(il.begin(), il.end(), il.size());
		}

		constexpr
		vector(std::initializer_list<value_type> il, const allocator_type& a)
			: alloc_(a) {
			init_with_size(il.begin(), il.end(), il.size());
		}

		constexpr vector&
		operator=(std::initializer_list<value_type> il) {
			assign(il.begin(), il.end());
			return *this;
		}

		constexpr vector(vector&& x) noexcept;

		constexpr
		vector(vector&& x, const std::type_identity_t<allocator_type>& a);

		constexpr vector& operator=(vector&& x) noexcept(
			std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
			std::allocator_traits<Allocator>::is_always_equal::value)
		{
			move_assign(
				x,
				std::integral_constant<bool, alloc_traits::propagate_on_container_move_assignment::value>()
			);
			return *this;
		}

		template<std::input_iterator InputIterator>
		constexpr void
		assign(InputIterator first, InputIterator last) {
			assign_with_sentinel(first, last);
		}

		template <std::forward_iterator ForwardIterator>
		constexpr void
		assign(ForwardIterator first, ForwardIterator last) {
			assign_with_size(first, last, std::distance(first, last));
		}

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<T> Range>
		constexpr void assign_range(Range&& range) {
			if constexpr (std::ranges::forward_range<Range> || std::ranges::sized_range<Range>) {
				auto n = static_cast<size_type>(std::ranges::distance(range));
				assign_with_size(std::ranges::begin(range), std::ranges::end(range), n);

			} else {
				assign_with_sentinel(std::ranges::begin(range), std::ranges::end(range));
			}
		}
#endif

		constexpr void
		assign(size_type n, const_reference u);

		constexpr void
		assign(std::initializer_list<value_type> il) {
			assign(il.begin(), il.end());
		}

		[[nodiscard]] constexpr allocator_type
		get_allocator() const noexcept {
			return this->alloc_;
		}

		//
		// Iterators
		//
		[[nodiscard]] constexpr iterator begin() noexcept {
			return make_iter(add_alignment_assumption(this->begin_));
		}

		[[nodiscard]] constexpr const_iterator
		begin() const noexcept {
			return make_iter(add_alignment_assumption(this->begin_));
		}

		[[nodiscard]] constexpr iterator end() noexcept {
			return make_iter(add_alignment_assumption(this->end_));
		}

		[[nodiscard]] constexpr const_iterator
		end() const noexcept {
			return make_iter(add_alignment_assumption(this->end_));
		}

		[[nodiscard]] constexpr reverse_iterator
		rbegin() noexcept {
			return reverse_iterator(end());
		}

		[[nodiscard]] constexpr const_reverse_iterator
		rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		[[nodiscard]] constexpr reverse_iterator
		rend() noexcept {
			return reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_reverse_iterator
		rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_iterator
		cbegin() const noexcept {
			return begin();
		}

		[[nodiscard]] constexpr const_iterator
		cend() const noexcept {
			return end();
		}

		[[nodiscard]] constexpr const_reverse_iterator
		crbegin() const noexcept {
			return rbegin();
		}

		[[nodiscard]] constexpr const_reverse_iterator
		crend() const noexcept {
			return rend();
		}

		//
		// [vector.capacity], capacity
		//
		[[nodiscard]] constexpr size_type size() const noexcept {
			return static_cast<size_type>(this->end_ - this->begin_);
		}

		[[nodiscard]] constexpr size_type
		capacity() const noexcept {
			return static_cast<size_type>(this->cap_ - this->begin_);
		}

		[[nodiscard]] constexpr bool
		empty() const noexcept {
			return this->begin_ == this->end_;
		}

		[[nodiscard]] constexpr size_type
		max_size() const noexcept {
			return std::min<size_type>(
				alloc_traits::max_size(this->alloc_),
				std::numeric_limits<difference_type>::max()
			);
		}

		constexpr void reserve(size_type n);
		constexpr void shrink_to_fit() noexcept;

		//
		// element access
		//
		[[nodiscard]] constexpr reference
		operator[](size_type n) noexcept {
			PLUGIFY_ASSERT(n < size(), "vector[] index out of bounds");
			return this->begin_[n];
		}

		[[nodiscard]] constexpr const_reference
		operator[](size_type n) const noexcept {
			PLUGIFY_ASSERT(n < size(), "vector[] index out of bounds");
			return this->begin_[n];
		}

		[[nodiscard]] constexpr reference at(size_type n) {
			if (n >= size()) {
				this->throw_out_of_range();
			}
			return this->begin_[n];
		}

		[[nodiscard]] constexpr const_reference
		at(size_type n) const {
			if (n >= size()) {
				this->throw_out_of_range();
			}
			return this->begin_[n];
		}

		[[nodiscard]] constexpr reference front() noexcept {
			PLUGIFY_ASSERT(!empty(), "front() called on an empty vector");
			return *this->begin_;
		}

		[[nodiscard]] constexpr const_reference
		front() const noexcept {
			PLUGIFY_ASSERT(!empty(), "front() called on an empty vector");
			return *this->begin_;
		}

		[[nodiscard]] constexpr reference back() noexcept {
			PLUGIFY_ASSERT(!empty(), "back() called on an empty vector");
			return *(this->end_ - 1);
		}

		[[nodiscard]] constexpr const_reference
		back() const noexcept {
			PLUGIFY_ASSERT(!empty(), "back() called on an empty vector");
			return *(this->end_ - 1);
		}

		//
		// [vector.data], data access
		//
		[[nodiscard]] constexpr value_type*
		data() noexcept {
			return std::to_address(this->begin_);
		}

		[[nodiscard]] constexpr const value_type*
		data() const noexcept {
			return std::to_address(this->begin_);
		}

		//
		// [vector.modifiers], modifiers
		//
		constexpr void push_back(const_reference x) {
			emplace_back(x);
		}

		constexpr void push_back(value_type&& x) {
			emplace_back(std::move(x));
		}

		template <class... Args>
		constexpr
			reference
			emplace_back(Args&&... args);

		template <class... Args>
		constexpr void
		emplace_back_assume_capacity(Args&&... args) {
			PLUGIFY_ASSERT(
				size() < capacity(),
				"We assume that we have enough space to insert an element at the end of the vector"
			);
			ConstructTransaction tx(*this, 1);
			alloc_traits::construct(this->alloc_, std::to_address(tx.pos_), std::forward<Args>(args)...);
			++tx.pos_;
		}

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<T> Range>
		constexpr void append_range(Range&& range) {
			insert_range(end(), std::forward<Range>(range));
		}
#endif

		constexpr void pop_back() {
			PLUGIFY_ASSERT(!empty(), "vector::pop_back called on an empty vector");
			this->destruct_at_end(this->end_ - 1);
		}

		constexpr iterator
		insert(const_iterator position, const_reference x);

		constexpr iterator
		insert(const_iterator position, value_type&& x);
		template <class... Args>
		constexpr iterator
		emplace(const_iterator position, Args&&... args);

		constexpr iterator
		insert(const_iterator position, size_type n, const_reference x);

		template<std::input_iterator InputIterator>
		constexpr iterator
		insert(const_iterator position, InputIterator first, InputIterator last) {
			return insert_with_sentinel(position, first, last);
		}

		template <std::forward_iterator ForwardIterator>
		constexpr iterator
		insert(const_iterator position, ForwardIterator first, ForwardIterator last) {
			return insert_with_size(position, first, last, std::distance(first, last));
		}

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<T> Range>
		constexpr iterator
		insert_range(const_iterator position, Range&& range) {
			if constexpr (std::ranges::forward_range<Range> || std::ranges::sized_range<Range>) {
				auto n = static_cast<size_type>(std::ranges::distance(range));
				return insert_with_size(position, std::ranges::begin(range), std::ranges::end(range), n);

			} else {
				return insert_with_sentinel(position, std::ranges::begin(range), std::ranges::end(range));
			}
		}
#endif

		constexpr iterator
		insert(const_iterator position, std::initializer_list<value_type> il) {
			return insert(position, il.begin(), il.end());
		}

		constexpr iterator erase(const_iterator position);
		constexpr iterator
		erase(const_iterator first, const_iterator last);

		constexpr void clear() noexcept {
			size_type old_size = size();
			base_destruct_at_end(this->begin_);
			annotate_shrink(old_size);
		}

		constexpr void resize(size_type sz);
		constexpr void
		resize(size_type sz, const_reference x);

		constexpr void swap(vector&) noexcept;

		constexpr bool invariants() const;

	private:
		pointer begin_ = nullptr;
		pointer end_ = nullptr;
		pointer cap_ = nullptr;
		PLUGIFY_NO_UNIQUE_ADDRESS
		allocator_type alloc_;

		//  Allocate space for n objects
		//  throws length_error if n > max_size()
		//  throws (probably bad_alloc) if memory run out
		//  Precondition:  begin_ == end_ == cap_ == nullptr
		//  Precondition:  n > 0
		//  Postcondition:  capacity() >= n
		//  Postcondition:  size() == 0
		constexpr void vallocate(size_type n) {
			if (n > max_size()) {
				this->throw_length_error();
			}
			auto allocation = allocate_at_least(this->alloc_, n);
			begin_ = allocation.ptr;
			end_ = allocation.ptr;
			cap_ = begin_ + allocation.count;
			annotate_new(0);
		}

		constexpr void vdeallocate() noexcept;
		constexpr size_type recommend(size_type new_size) const;
		constexpr void construct_at_end(size_type n);
		constexpr void
		construct_at_end(size_type n, const_reference x);

		template <class InputIterator, class Sentinel>
		constexpr void
		init_with_size(InputIterator first, Sentinel last, size_type n) {
			auto guard = make_exception_guard(destroy_vector(*this));

			if (n > 0) {
				vallocate(n);
				construct_at_end(std::move(first), std::move(last), n);
			}

			guard.complete();
		}

		template <class InputIterator, class Sentinel>
		constexpr void
		init_with_sentinel(InputIterator first, Sentinel last) {
			auto guard = make_exception_guard(destroy_vector(*this));

			for (; first != last; ++first) {
				emplace_back(*first);
			}

			guard.complete();
		}

		template <class Iterator, class Sentinel>
		constexpr void
		assign_with_sentinel(Iterator first, Sentinel last);

		// The `Iterator` in `*_with_size` functions can be input-only only if called from
		// `*_range` (since C++23). Otherwise, `Iterator` is a forward iterator.

		template <class Iterator, class Sentinel>
		constexpr void
		assign_with_size(Iterator first, Sentinel last, difference_type n);

		template <class Iterator>
			requires (!std::same_as<decltype(*std::declval<Iterator&>())&&, value_type&&>)
		constexpr void
		insert_assign_n_unchecked(Iterator first, difference_type n, pointer position) {
			for (pointer end_position = position + n; position != end_position;
				 ++position, (void) ++first) {
				detail::temp_value<value_type, Allocator> tmp(this->alloc_, *first);
				*position = std::move(tmp.get());
			}
		}

		template <class Iterator>
			requires (std::same_as<decltype(*std::declval<Iterator&>())&&, value_type&&>)
		constexpr void
		insert_assign_n_unchecked(Iterator first, difference_type n, pointer position) {
#if PLUGIFY_HAS_CXX23
			if constexpr (!std::forward_iterator<Iterator>) {
				// Handles input-only sized ranges for insert_range
				std::ranges::copy_n(std::move(first), n, position);
			} else
#endif
			{
				std::copy_n(first, n, position);
			}
		}

		template <class InputIterator, class Sentinel>
		constexpr iterator
		insert_with_sentinel(const_iterator position, InputIterator first, Sentinel last);

		template <class Iterator, class Sentinel>
		constexpr iterator
		insert_with_size(const_iterator position, Iterator first, Sentinel last, difference_type n);

		template <class InputIterator, class Sentinel>
		constexpr void
		construct_at_end(InputIterator first, Sentinel last, size_type n);

		constexpr void append(size_type n);
		constexpr void
		append(size_type n, const_reference x);

		constexpr iterator make_iter(pointer p) noexcept {
			return iterator(p);
		}

		constexpr const_iterator make_iter(const_pointer p) const noexcept {
			return const_iterator(p);
		}

		constexpr void
		swap_out_circular_buffer(split_buffer<value_type, allocator_type&>& v);
		constexpr pointer
		swap_out_circular_buffer(split_buffer<value_type, allocator_type&>& v, pointer p);
		constexpr void
		move_range(pointer from_s, pointer from_e, pointer to);
		constexpr void
		move_assign(vector& c, std::true_type) noexcept(std::is_nothrow_move_assignable<allocator_type>::value);
		constexpr void
		move_assign(vector& c, std::false_type) noexcept(alloc_traits::is_always_equal::value);

		constexpr void
		destruct_at_end(pointer new_last) noexcept {
			size_type old_size = size();
			base_destruct_at_end(new_last);
			annotate_shrink(old_size);
		}

		template <class... Args>
		constexpr inline pointer
		emplace_back_slow_path(Args&&... args);

		// The following functions are no-ops outside of AddressSanitizer mode.
		// We call annotations for every allocator, unless explicitly disabled.
		//
		// To disable annotations for a particular allocator, change value of
		// asan_annotate_container_with_allocator to false.
		// For more details, see the "Using libc++" documentation page or
		// the documentation for sanitizer_annotate_contiguous_container.

		constexpr void
		annotate_contiguous_container(
			[[maybe_unused]] const void* old_mid,
			[[maybe_unused]] const void* new_mid
		) const {
			plg::annotate_contiguous_container<Allocator>(data(), data() + capacity(), old_mid, new_mid);
		}

		constexpr void
		annotate_new(size_type current_size) const noexcept {
			annotate_contiguous_container(data() + capacity(), data() + current_size);
		}

		constexpr void annotate_delete() const noexcept {
			annotate_contiguous_container(data() + size(), data() + capacity());
		}

		constexpr void
		annotate_increase(size_type n) const noexcept {
			annotate_contiguous_container(data() + size(), data() + size() + n);
		}

		constexpr void
		annotate_shrink(size_type old_size) const noexcept {
			annotate_contiguous_container(data() + old_size, data() + size());
		}

		struct ConstructTransaction {
			constexpr
				explicit ConstructTransaction(vector& v, size_type n)
				: v_(v)
				, pos_(v.end_)
				, new_end_(v.end_ + n) {
				v.annotate_increase(n);
			}

			constexpr ~ConstructTransaction() {
				v_.end_ = pos_;
				if (pos_ != new_end_) {
					v_.annotate_shrink(new_end_ - v_.begin_);
				}
			}

			vector& v_;
			pointer pos_;
			const const_pointer new_end_;

			ConstructTransaction(const ConstructTransaction&) = delete;
			ConstructTransaction& operator=(const ConstructTransaction&) = delete;
		};

		constexpr void
		base_destruct_at_end(pointer new_last) noexcept {
			pointer soon_to_be_end = this->end_;
			while (new_last != soon_to_be_end) {
				alloc_traits::destroy(this->alloc_, std::to_address(--soon_to_be_end));
			}
			this->end_ = new_last;
		}

		constexpr void copy_assign_alloc(const vector& c) {
			copy_assign_alloc(
				c,
				std::integral_constant<bool, alloc_traits::propagate_on_container_copy_assignment::value>()
			);
		}

		constexpr void
		move_assign_alloc(vector& c) noexcept(
			!alloc_traits::propagate_on_container_move_assignment::value
			|| std::is_nothrow_move_assignable<allocator_type>::value
		) {
			move_assign_alloc(
				c,
				std::integral_constant<bool, alloc_traits::propagate_on_container_move_assignment::value>()
			);
		}

		[[noreturn]] static void throw_length_error() {
			PLUGIFY_THROW("allocated memory size would exceed max_size()", std::length_error);
		}

		[[noreturn]] static void throw_out_of_range() {
			PLUGIFY_THROW("input index is out of bounds", std::out_of_range);
		}

		constexpr void
		copy_assign_alloc(const vector& c, std::true_type) {
			if (this->alloc_ != c.alloc_) {
				clear();
				annotate_delete();
				alloc_traits::deallocate(this->alloc_, this->begin_, capacity());
				this->begin_ = this->end_ = this->cap_ = nullptr;
			}
			this->alloc_ = c.alloc_;
		}

		constexpr void
		copy_assign_alloc(const vector&, std::false_type) {
		}

		constexpr void
		move_assign_alloc(vector& c, std::true_type) noexcept(
			std::is_nothrow_move_assignable_v<allocator_type>
		) {
			this->alloc_ = std::move(c.alloc_);
		}

		constexpr void
		move_assign_alloc(vector&, std::false_type) noexcept {
		}

		template <class Ptr = pointer>
			requires(std::is_pointer_v<Ptr>)
		static constexpr PLUGIFY_NO_CFI pointer add_alignment_assumption(Ptr p) noexcept {
			if (!std::is_constant_evaluated()) {
				return static_cast<pointer>(std::assume_aligned<alignof(decltype(*p))>(p));
			}
			return p;
		}

		template <class Ptr = pointer>
			requires(!std::is_pointer_v<Ptr>)
		static constexpr PLUGIFY_NO_CFI pointer add_alignment_assumption(Ptr p) noexcept {
			return p;
		}

		constexpr void
		swap_layouts(split_buffer<T, allocator_type&>& sb) {
			auto vector_begin = begin_;
			auto vector_sentinel = end_;
			auto vector_cap = cap_;

			auto sb_begin = sb.begin();
			auto sb_sentinel = sb.raw_sentinel();
			auto sb_cap = sb.raw_capacity();

			// TODO: replace with set_valid_range and set_capacity when vector supports it.
			begin_ = sb_begin;
			end_ = sb_sentinel;
			cap_ = sb_cap;

			sb.set_valid_range(vector_begin, vector_sentinel);
			sb.set_capacity(vector_cap);
		}
	};

	template <
		std::input_iterator InputIterator,
		is_allocator Alloc>
	vector(InputIterator, InputIterator, Alloc = Alloc())
		-> vector<std::iterator_traits<InputIterator>, Alloc>;

#if PLUGIFY_HAS_CXX23
	template <
		std::ranges::input_range Range,
		is_allocator Alloc>
	vector(std::from_range_t, Range&&, Alloc = Alloc())
		-> vector<std::ranges::range_value_t<Range>, Alloc>;
#endif

	// swap_out_circular_buffer relocates the objects in [begin_, end_) into the front of v and
	// swaps the buffers of *this and v. It is assumed that v provides space for exactly (end_ -
	// begin_) objects in the front. This function has a strong exception guarantee.
	template <class T, class Allocator>
	constexpr void
	vector<T, Allocator>::swap_out_circular_buffer(split_buffer<value_type, allocator_type&>& v) {
		annotate_delete();
		auto new_begin = v.begin() - size();
		uninitialized_allocator_relocate(
			this->alloc_,
			std::to_address(begin_),
			std::to_address(end_),
			std::to_address(new_begin)
		);
		v.set_valid_range(new_begin, v.end());
		end_ = begin_;	// All the objects have been destroyed by relocating them.

		swap_layouts(v);
		v.set_data(v.begin());
		annotate_new(size());
	}

	// swap_out_circular_buffer relocates the objects in [begin_, p) into the front of v, the
	// objects in [p, end_) into the back of v and swaps the buffers of *this and v. It is assumed
	// that v provides space for exactly (p - begin_) objects in the front and space for at least
	// (end_ - p) objects in the back. This function has a strong exception guarantee if begin_ == p
	// || end_ == p.
	template <class T, class Allocator>
	constexpr typename vector<T, Allocator>::pointer
	vector<T, Allocator>::swap_out_circular_buffer(
		split_buffer<value_type, allocator_type&>& v,
		pointer p
	) {
		annotate_delete();
		pointer ret = v.begin();

		// Relocate [p, end_) first to avoid having a hole in [begin_, end_)
		// in case something in [begin_, p) throws.
		uninitialized_allocator_relocate(
			this->alloc_,
			std::to_address(p),
			std::to_address(end_),
			std::to_address(v.end())
		);
		auto relocated_so_far = end_ - p;
		v.set_sentinel(v.end() + relocated_so_far);
		end_ = p;  // The objects in [p, end_) have been destroyed by relocating them.
		auto new_begin = v.begin() - (p - begin_);

		uninitialized_allocator_relocate(
			this->alloc_,
			std::to_address(begin_),
			std::to_address(p),
			std::to_address(new_begin)
		);
		v.set_valid_range(new_begin, v.end());
		end_ = begin_;	// All the objects have been destroyed by relocating them.
		swap_layouts(v);
		v.set_data(v.begin());
		annotate_new(size());
		return ret;
	}

	template <class T, class Allocator>
	constexpr void vector<T, Allocator>::vdeallocate() noexcept {
		if (this->begin_ != nullptr) {
			clear();
			annotate_delete();
			alloc_traits::deallocate(this->alloc_, this->begin_, capacity());
			this->begin_ = this->end_ = this->cap_ = nullptr;
		}
	}

	//  Precondition:  new_size > capacity()
	template <class T, class Allocator>
	constexpr inline
		typename vector<T, Allocator>::size_type
		vector<T, Allocator>::recommend(size_type new_size) const {
		const size_type ms = max_size();
		if (new_size > ms) {
			this->throw_length_error();
		}
		const size_type cap = capacity();
		if (cap >= ms / 2) {
			return ms;
		}
		return std::max<size_type>(2 * cap, new_size);
	}

	//  Default constructs n objects starting at end_
	//  throws if construction throws
	//  Precondition:  n > 0
	//  Precondition:  size() + n <= capacity()
	//  Postcondition:  size() == size() + n
	template <class T, class Allocator>
	constexpr void vector<T, Allocator>::construct_at_end(size_type n) {
		ConstructTransaction tx(*this, n);
		const_pointer new_end = tx.new_end_;
		for (pointer pos = tx.pos_; pos != new_end; tx.pos_ = ++pos) {
			alloc_traits::construct(this->alloc_, std::to_address(pos));
		}
	}

	//  Copy constructs n objects starting at end_ from x
	//  throws if construction throws
	//  Precondition:  n > 0
	//  Precondition:  size() + n <= capacity()
	//  Postcondition:  size() == old size() + n
	//  Postcondition:  [i] == x for all i in [size() - n, n)
	template <class T, class Allocator>
	constexpr inline void
	vector<T, Allocator>::construct_at_end(size_type n, const_reference x) {
		ConstructTransaction tx(*this, n);
		const_pointer new_end = tx.new_end_;
		for (pointer pos = tx.pos_; pos != new_end; tx.pos_ = ++pos) {
			alloc_traits::construct(this->alloc_, std::to_address(pos), x);
		}
	}

	template <class T, class Allocator>
	template <class InputIterator, class Sentinel>
	constexpr void
	vector<T, Allocator>::construct_at_end(InputIterator first, Sentinel last, size_type n) {
		ConstructTransaction tx(*this, n);
		tx.pos_ = uninitialized_allocator_copy(
			this->alloc_,
			std::move(first),
			std::move(last),
			tx.pos_
		);
	}

	//  Default constructs n objects starting at end_
	//  throws if construction throws
	//  Postcondition:  size() == size() + n
	//  Exception safety: strong.
	template <class T, class Allocator>
	constexpr void vector<T, Allocator>::append(size_type n) {
		if (static_cast<size_type>(this->cap_ - this->end_) >= n) {
			this->construct_at_end(n);
		} else {
			split_buffer<value_type, allocator_type&> v(recommend(size() + n), size(), this->alloc_);
			v.construct_at_end(n);
			swap_out_circular_buffer(v);
		}
	}

	//  Default constructs n objects starting at end_
	//  throws if construction throws
	//  Postcondition:  size() == size() + n
	//  Exception safety: strong.
	template <class T, class Allocator>
	constexpr void
	vector<T, Allocator>::append(size_type n, const_reference x) {
		if (static_cast<size_type>(this->cap_ - this->end_) >= n) {
			this->construct_at_end(n, x);
		} else {
			split_buffer<value_type, allocator_type&> v(recommend(size() + n), size(), this->alloc_);
			v.construct_at_end(n, x);
			swap_out_circular_buffer(v);
		}
	}

	template <class T, class Allocator>
	constexpr inline
	vector<T, Allocator>::vector(vector&& x) noexcept
		: alloc_(std::move(x.alloc_)) {
		this->begin_ = x.begin_;
		this->end_ = x.end_;
		this->cap_ = x.cap_;
		x.begin_ = x.end_ = x.cap_ = nullptr;
	}

	template <class T, class Allocator>
	constexpr inline
	vector<T, Allocator>::vector(vector&& x, const std::type_identity_t<allocator_type>& a)
		: alloc_(a) {
		if (a == x.alloc_) {
			this->begin_ = x.begin_;
			this->end_ = x.end_;
			this->cap_ = x.cap_;
			x.begin_ = x.end_ = x.cap_ = nullptr;
		} else {
			using Ip = std::move_iterator<iterator>;
			init_with_size(Ip(x.begin()), Ip(x.end()), x.size());
		}
	}

	template <class T, class Allocator>
	constexpr void
	vector<T, Allocator>::move_assign(vector& c, std::false_type) noexcept(
		alloc_traits::is_always_equal::value
	) {
		if (this->alloc_ != c.alloc_) {
			using Ip = std::move_iterator<iterator>;
			assign(Ip(c.begin()), Ip(c.end()));
		} else {
			move_assign(c, std::true_type());
		}
	}

	template <class T, class Allocator>
	constexpr void
	vector<T, Allocator>::move_assign(vector& c, std::true_type) noexcept(
		std::is_nothrow_move_assignable<allocator_type>::value
	) {
		vdeallocate();
		move_assign_alloc(c);  // this can throw
		this->begin_ = c.begin_;
		this->end_ = c.end_;
		this->cap_ = c.cap_;
		c.begin_ = c.end_ = c.cap_ = nullptr;
	}

	template <class T, class Allocator>
	constexpr inline vector<T, Allocator>&
	vector<T, Allocator>::operator=(const vector& x) {
		if (this != std::addressof(x)) {
			copy_assign_alloc(x);
			assign(x.begin_, x.end_);
		}
		return *this;
	}

	template <class T, class Allocator>
	template <class Iterator, class Sentinel>
	constexpr void
	vector<T, Allocator>::assign_with_sentinel(Iterator first, Sentinel last) {
		pointer cur = begin_;
		for (; first != last && cur != end_; ++first, (void) ++cur) {
			*cur = *first;
		}
		if (cur != end_) {
			destruct_at_end(cur);
		} else {
			for (; first != last; ++first) {
				emplace_back(*first);
			}
		}
	}

	template <class T, class Allocator>
	template <class Iterator, class Sentinel>
	constexpr void
	vector<T, Allocator>::assign_with_size(Iterator first, Sentinel last, difference_type n) {
		size_type new_size = static_cast<size_type>(n);
		if (new_size <= capacity()) {
			if (new_size > size()) {
#if PLUGIFY_HAS_CXX23
				auto mid = std::ranges::copy_n(std::move(first), size(), this->begin_).in;
				construct_at_end(std::move(mid), std::move(last), new_size - size());
#else
				Iterator mid = std::next(first, size());
				std::copy(first, mid, this->begin_);
				construct_at_end(mid, last, new_size - size());
#endif
			} else {
				pointer m = std::copy(std::move(first), last, this->begin_);
				this->destruct_at_end(m);
			}
		} else {
			vdeallocate();
			vallocate(recommend(new_size));
			construct_at_end(std::move(first), std::move(last), new_size);
		}
	}

	template <class T, class Allocator>
	constexpr void
	vector<T, Allocator>::assign(size_type n, const_reference u) {
		if (n <= capacity()) {
			size_type s = size();
			std::fill_n(this->begin_, std::min(n, s), u);
			if (n > s) {
				construct_at_end(n - s, u);
			} else {
				this->destruct_at_end(this->begin_ + n);
			}
		} else {
			vdeallocate();
			vallocate(recommend(static_cast<size_type>(n)));
			construct_at_end(n, u);
		}
	}

	template <class T, class Allocator>
	constexpr void vector<T, Allocator>::reserve(size_type n) {
		if (n > capacity()) {
			if (n > max_size()) {
				this->throw_length_error();
			}
			split_buffer<value_type, allocator_type&> v(n, size(), this->alloc_);
			swap_out_circular_buffer(v);
		}
	}

	template <class T, class Allocator>
	constexpr void vector<T, Allocator>::shrink_to_fit() noexcept {
		if (capacity() > size()) {
#if PLUGIFY_HAS_EXCEPTIONS
			try {
#endif	// PLUGIFY_HAS_EXCEPTIONS
				split_buffer<value_type, allocator_type&> v(size(), size(), this->alloc_);
				// The Standard mandates shrink_to_fit() does not increase the capacity.
				// With equal capacity keep the existing buffer. This avoids extra work
				// due to swapping the elements.
				if (v.capacity() < capacity()) {
					swap_out_circular_buffer(v);
				}
#if PLUGIFY_HAS_EXCEPTIONS
			} catch (...) {
			}
#endif	// PLUGIFY_HAS_EXCEPTIONS
		}
	}

	template <class T, class Allocator>
	template <class... Args>
	constexpr typename vector<T, Allocator>::pointer
	vector<T, Allocator>::emplace_back_slow_path(Args&&... args) {
		split_buffer<value_type, allocator_type&> v(recommend(size() + 1), size(), this->alloc_);
		//    v.emplace_back(std::forward<Args>(args)...);
		pointer end = v.end();
		alloc_traits::construct(this->alloc_, std::to_address(end), std::forward<Args>(args)...);
		v.set_sentinel(++end);
		swap_out_circular_buffer(v);
		return this->end_;
	}

	// This makes the compiler inline `else()` if `cond` is known to be false. Currently LLVM
	// doesn't do that without the `builtin_constant_p`, since it considers `else` unlikely even
	// through it's known to be run. See https://llvm.org/PR154292
	template <class If, class Else>
	constexpr void
	if_likely_else(bool cond, If _if, Else _else) {
		if (__builtin_constant_p(cond)) {
			if (cond) {
				_if();
			} else {
				_else();
			}
		} else {
			if (cond) [[likely]] {
				_if();
			} else {
				_else();
			}
		}
	}

	template <class T, class Allocator>
	template <class... Args>
	constexpr inline
		typename vector<T, Allocator>::reference
		vector<T, Allocator>::emplace_back(Args&&... args) {
		pointer end = this->end_;
		if_likely_else(
			end < this->cap_,
			[&] {
				emplace_back_assume_capacity(std::forward<Args>(args)...);
				++end;
			},
			[&] { end = emplace_back_slow_path(std::forward<Args>(args)...); }
		);

		this->end_ = end;
		return *(end - 1);
	}

	template <class T, class Allocator>
	constexpr inline
		typename vector<T, Allocator>::iterator
		vector<T, Allocator>::erase(const_iterator position) {
		PLUGIFY_ASSERT(
			position != end(),
			"vector::erase(iterator) called with a non-dereferenceable iterator"
		);
		difference_type ps = position - cbegin();
		pointer p = this->begin_ + ps;
		this->destruct_at_end(std::move(p + 1, this->end_, p));
		return make_iter(p);
	}

	template <class T, class Allocator>
	constexpr typename vector<T, Allocator>::iterator
	vector<T, Allocator>::erase(const_iterator first, const_iterator last) {
		PLUGIFY_ASSERT(
			first <= last,
			"vector::erase(first, last) called with invalid range"
		);
		pointer p = this->begin_ + (first - begin());
		if (first != last) {
			this->destruct_at_end(std::move(p + (last - first), this->end_, p));
		}
		return make_iter(p);
	}

	template <class T, class Allocator>
	constexpr void
	vector<T, Allocator>::move_range(pointer from_s, pointer from_e, pointer to) {
		pointer old_last = this->end_;
		difference_type n = old_last - to;
		{
			pointer i = from_s + n;
			ConstructTransaction tx(*this, from_e - i);
			for (pointer pos = tx.pos_; i < from_e; ++i, (void) ++pos, tx.pos_ = pos) {
				alloc_traits::construct(this->alloc_, std::to_address(pos), std::move(*i));
			}
		}
		std::move_backward(from_s, from_s + n, old_last);
	}

	template <class T, class Allocator>
	constexpr typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert(const_iterator position, const_reference x) {
		pointer p = this->begin_ + (position - begin());
		if (this->end_ < this->cap_) {
			if (p == this->end_) {
				emplace_back_assume_capacity(x);
			} else {
				move_range(p, this->end_, p + 1);
				const_pointer xr = std::pointer_traits<const_pointer>::pointer_to(x);
				if (is_pointer_in_range(
						std::to_address(p),
						std::to_address(end_),
						std::addressof(x)
					)) {
					++xr;
				}
				*p = *xr;
			}
		} else {
			split_buffer<value_type, allocator_type&> v(
				recommend(size() + 1),
				p - this->begin_,
				this->alloc_
			);
			v.emplace_back(x);
			p = swap_out_circular_buffer(v, p);
		}
		return make_iter(p);
	}

	template <class T, class Allocator>
	constexpr typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert(const_iterator position, value_type&& x) {
		pointer p = this->begin_ + (position - begin());
		if (this->end_ < this->cap_) {
			if (p == this->end_) {
				emplace_back_assume_capacity(std::move(x));
			} else {
				move_range(p, this->end_, p + 1);
				*p = std::move(x);
			}
		} else {
			split_buffer<value_type, allocator_type&> v(
				recommend(size() + 1),
				p - this->begin_,
				this->alloc_
			);
			v.emplace_back(std::move(x));
			p = swap_out_circular_buffer(v, p);
		}
		return make_iter(p);
	}

	template <class T, class Allocator>
	template <class... Args>
	constexpr typename vector<T, Allocator>::iterator
	vector<T, Allocator>::emplace(const_iterator position, Args&&... args) {
		pointer p = this->begin_ + (position - begin());
		if (this->end_ < this->cap_) {
			if (p == this->end_) {
				emplace_back_assume_capacity(std::forward<Args>(args)...);
			} else {
				detail::temp_value<value_type, Allocator> tmp(this->alloc_, std::forward<Args>(args)...);
				move_range(p, this->end_, p + 1);
				*p = std::move(tmp.get());
			}
		} else {
			split_buffer<value_type, allocator_type&> v(
				recommend(size() + 1),
				p - this->begin_,
				this->alloc_
			);
			v.emplace_back(std::forward<Args>(args)...);
			p = swap_out_circular_buffer(v, p);
		}
		return make_iter(p);
	}

	template <class T, class Allocator>
	constexpr typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert(const_iterator position, size_type n, const_reference x) {
		pointer p = this->begin_ + (position - begin());
		if (n > 0) {
			if (n <= static_cast<size_type>(this->cap_ - this->end_)) {
				size_type old_n = n;
				pointer old_last = this->end_;
				if (n > static_cast<size_type>(this->end_ - p)) {
					size_type cx = n - (this->end_ - p);
					construct_at_end(cx, x);
					n -= cx;
				}
				if (n > 0) {
					move_range(p, old_last, p + old_n);
					const_pointer xr = std::pointer_traits<const_pointer>::pointer_to(x);
					if (is_pointer_in_range(
							std::to_address(p),
							std::to_address(end_),
							std::addressof(x)
						)) {
						xr += old_n;
					}
					std::fill_n(p, n, *xr);
				}
			} else {
				split_buffer<value_type, allocator_type&> v(
					recommend(size() + n),
					p - this->begin_,
					this->alloc_
				);
				v.construct_at_end(n, x);
				p = swap_out_circular_buffer(v, p);
			}
		}
		return make_iter(p);
	}

	template <class T, class Allocator>
	template <class InputIterator, class Sentinel>
	constexpr typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert_with_sentinel(
		const_iterator position,
		InputIterator first,
		Sentinel last
	) {
		difference_type off = position - begin();
		pointer p = this->begin_ + off;
		pointer old_last = this->end_;
		for (; this->end_ != this->cap_ && first != last; ++first) {
			emplace_back_assume_capacity(*first);
		}

		if (first == last) {
			(void) std::rotate(p, old_last, this->end_);
		} else {
			split_buffer<value_type, allocator_type&> v(alloc_);
			auto guard = make_exception_guard(
				AllocatorDestroyRangeReverse<allocator_type, pointer>(alloc_, old_last, this->end_)
			);
			v.construct_at_end_with_sentinel(std::move(first), std::move(last));
			split_buffer<value_type, allocator_type&> merged(
				recommend(size() + v.size()),
				off,
				alloc_
			);	// has `off` positions available at the front
			uninitialized_allocator_relocate(
				alloc_,
				std::to_address(old_last),
				std::to_address(this->end_),
				std::to_address(merged.end())
			);
			guard.complete();  // Release the guard once objects in [old_last_, end_) have been
							   // successfully relocated.
			merged.set_sentinel(merged.end() + (this->end_ - old_last));
			this->end_ = old_last;
			uninitialized_allocator_relocate(
				alloc_,
				std::to_address(v.begin()),
				std::to_address(v.end()),
				std::to_address(merged.end())
			);
			merged.set_sentinel(merged.size() + v.size());
			v.set_sentinel(v.begin());
			p = swap_out_circular_buffer(merged, p);
		}
		return make_iter(p);
	}

	template <class T, class Allocator>
	template <class Iterator, class Sentinel>
	constexpr typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert_with_size(
		const_iterator position,
		Iterator first,
		Sentinel last,
		difference_type n
	) {
		pointer p = this->begin_ + (position - begin());
		if (n > 0) {
			if (n <= this->cap_ - this->end_) {
				pointer old_last = this->end_;
				difference_type dx = this->end_ - p;
				if (n > dx) {
#if PLUGIFY_HAS_CXX23
					if constexpr (!std::forward_iterator<Iterator>) {
						construct_at_end(std::move(first), std::move(last), n);
						std::rotate(p, old_last, this->end_);
					} else
#endif
					{
						Iterator m = std::next(first, dx);
						construct_at_end(m, last, n - dx);
						if (dx > 0) {
							move_range(p, old_last, p + n);
							insert_assign_n_unchecked(first, dx, p);
						}
					}
				} else {
					move_range(p, old_last, p + n);
					insert_assign_n_unchecked(std::move(first), n, p);
				}
			} else {
				split_buffer<value_type, allocator_type&> v(
					recommend(size() + n),
					p - this->begin_,
					this->alloc_
				);
				v.construct_at_end_with_size(std::move(first), n);
				p = swap_out_circular_buffer(v, p);
			}
		}
		return make_iter(p);
	}

	template <class T, class Allocator>
	constexpr void vector<T, Allocator>::resize(size_type sz) {
		size_type cs = size();
		if (cs < sz) {
			this->append(sz - cs);
		} else if (cs > sz) {
			this->destruct_at_end(this->begin_ + sz);
		}
	}

	template <class T, class Allocator>
	constexpr void
	vector<T, Allocator>::resize(size_type sz, const_reference x) {
		size_type cs = size();
		if (cs < sz) {
			this->append(sz - cs, x);
		} else if (cs > sz) {
			this->destruct_at_end(this->begin_ + sz);
		}
	}

	template <class T, class Allocator>
	constexpr void vector<T, Allocator>::swap(vector& x)
		noexcept
	{
		PLUGIFY_ASSERT(
			alloc_traits::propagate_on_container_swap::value || this->alloc_ == x.alloc_,
			"vector::swap: Either propagate_on_container_swap must be true"
			" or the allocators must compare equal"
		);
		std::swap(this->begin_, x.begin_);
		std::swap(this->end_, x.end_);
		std::swap(this->cap_, x.cap_);
		swap_allocator(this->alloc_, x.alloc_);
	}

	template <class T, class Allocator>
	constexpr bool vector<T, Allocator>::invariants() const {
		if (this->begin_ == nullptr) {
			if (this->end_ != nullptr || this->cap_ != nullptr) {
				return false;
			}
		} else {
			if (this->begin_ > this->end_) {
				return false;
			}
			if (this->begin_ == this->cap_) {
				return false;
			}
			if (this->end_ > this->cap_) {
				return false;
			}
		}
		return true;
	}

	// comparisons
	template<typename T, typename Allocator>
	constexpr bool operator==(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	template<typename T, typename Allocator>
	constexpr auto operator<=>(const vector<T, Allocator>& lhs, const vector<T, Allocator>& rhs) {
		return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	// global swap for vector
	template<typename T, typename Allocator>
	constexpr void swap(vector<T, Allocator>& lhs, vector<T, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
		lhs.swap(rhs);
	}

	template<typename T, typename Allocator, typename U>
	constexpr vector<T, Allocator>::size_type erase(vector<T, Allocator>& c, const U& value) {
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	template<typename T, typename Allocator, typename Pred>
	constexpr vector<T, Allocator>::size_type erase_if(vector<T, Allocator>& c, Pred pred) {
		auto it = std::remove_if(c.begin(), c.end(), pred);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	namespace pmr {
		template<typename T>
		using vector = ::plg::vector<T, std::pmr::polymorphic_allocator<T>>;
	} // namespace pmr
} // namespace plg

#ifndef PLUGIFY_VECTOR_NO_STD_UTIL
#include <optional>
#include <string_view>
#include <charconv>

namespace plg {
	constexpr vector<std::string_view> split(std::string_view str, std::string_view delims = " ") {
		vector<std::string_view> output;
		size_t first = 0;

		while (first < str.size()) {
			const size_t second = str.find_first_of(delims, first);

			if (first != second)
				output.emplace_back(str.substr(first, second - first));

			if (second == std::string_view::npos)
				break;

			first = second + 1;
		}

		return output;
	}

	template<typename T>
	constexpr std::optional<T> cast_to(std::string_view str) {
		T value;
		auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
		if (ec == std::errc()) {
			return value;
		}
		return std::nullopt;
	}

#if PLUGIFY_PLATFORM_APPLE
	template<>
	inline std::optional<double> cast_to(std::string_view str) {
		std::string p(str);
		size_t pos = 0;
		try {
			double v = std::stod(p, &pos);
			if (pos != p.size()) return std::nullopt;
			return v;
		} catch (...) {
			return std::nullopt;
		}
	}

	template<>
	inline std::optional<float> cast_to(std::string_view str) {
		std::string p(str);
		size_t pos = 0;
		try {
			float v = std::stof(p, &pos);
			if (pos != p.size()) return std::nullopt;
			return v;
		} catch (...) {
			return std::nullopt;
		}
	}
#endif

	template<typename T>
	constexpr vector<T> parse(std::string_view str, std::string_view delims = " ") {
		vector<T> vec;
		auto items = split(str, delims);
		vec.reserve(items.size());
		for (const auto& item : items) {
			if (auto value = cast_to<T>(item)) {
				vec.emplace_back(*value);
			}
		}
		return vec;
	}
}  // namespace plugify
#endif // PLUGIFY_VECTOR_NO_STD_UTIL
