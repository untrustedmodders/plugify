#pragma once

#include "plg/config.hpp"
#include "plg/concepts.hpp"

namespace plg {
	template <class T, class Allocator, template <class, class, class> class Layout>
	class split_buffer;

	template <class SplitBuffer, class T, class Allocator>
	class split_buffer_pointer_layout {
	protected:
		using value_type = T;
		using allocator_type = Allocator;
		using alloc_rr = std::remove_reference_t<allocator_type>;
		using alloc_traits = std::allocator_traits<alloc_rr>;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = typename alloc_traits::size_type;
		using difference_type = typename alloc_traits::difference_type;
		using pointer = typename alloc_traits::pointer;
		using const_pointer = typename alloc_traits::const_pointer;
		using iterator = pointer;
		using constIterator = const_pointer;
		using sentinel_type = pointer;

	public:
		constexpr split_buffer_pointer_layout()
			: _back_cap(nullptr) {
		}

		constexpr explicit split_buffer_pointer_layout(const allocator_type& alloc)
			: _back_cap(nullptr)
			, _alloc(alloc) {
		}

		constexpr pointer front_cap() noexcept {
			return _front_cap;
		}

		constexpr const_pointer front_cap() const noexcept {
			return _front_cap;
		}

		constexpr pointer begin() noexcept {
			return _begin;
		}

		constexpr const_pointer begin() const noexcept {
			return _begin;
		}

		constexpr pointer end() noexcept {
			return _end;
		}

		constexpr pointer end() const noexcept {
			return _end;
		}

		constexpr size_type size() const noexcept {
			return static_cast<size_type>(_end - _begin);
		}

		constexpr bool empty() const noexcept {
			return _begin == _end;
		}

		constexpr size_type capacity() const noexcept {
			return static_cast<size_type>(_back_cap - _front_cap);
		}

		constexpr allocator_type& get_allocator() noexcept {
			return _alloc;
		}

		constexpr const allocator_type& get_allocator() const noexcept {
			return _alloc;
		}

		// Returns the sentinel object directly. Should be used in conjunction with automatic type
		// deduction, not explicit types.
		constexpr sentinel_type raw_sentinel() const noexcept {
			return _end;
		}

		constexpr sentinel_type raw_capacity() const noexcept {
			return _back_cap;
		}

		constexpr void set_data(pointer new_first) noexcept {
			_front_cap = new_first;
		}

		constexpr void
		set_valid_range(pointer new_begin, pointer new_end) noexcept {
			_begin = new_begin;
			_end = new_end;
		}

		constexpr void
		set_valid_range(pointer new_begin, size_type new_size) noexcept {
			_begin = new_begin;
			_end = _begin + new_size;
		}

		constexpr void set_sentinel(pointer new_end) noexcept {
			PLUGIFY_ASSERT(_front_cap <= new_end, "new_end cannot precede _front_cap");
			_end = new_end;
		}

		constexpr void set_sentinel(size_type new_size) noexcept {
			_end = _begin + new_size;
		}

		constexpr void set_capacity(size_type new_capacity) noexcept {
			_back_cap = _front_cap + new_capacity;
		}

		constexpr void set_capacity(pointer new_capacity) noexcept {
			_back_cap = new_capacity;
		}

		constexpr size_type front_spare() const noexcept {
			return static_cast<size_type>(_begin - _front_cap);
		}

		constexpr size_type back_spare() const noexcept {
			return static_cast<size_type>(_back_cap - _end);
		}

		constexpr reference back() noexcept {
			return *(_end - 1);
		}

		constexpr const_reference back() const noexcept {
			return *(_end - 1);
		}

		constexpr void swap_without_allocator(
			split_buffer_pointer_layout<
				split_buffer<value_type, alloc_rr&, split_buffer_pointer_layout>,
				value_type,
				alloc_rr&>& other
		) noexcept {
			std::swap(_front_cap, other._front_cap);
			std::swap(_begin, other._begin);
			std::swap(_back_cap, other._back_cap);
			std::swap(_end, other._end);
		}

		constexpr void swap(split_buffer_pointer_layout& other) noexcept {
			std::swap(_front_cap, other._front_cap);
			std::swap(_begin, other._begin);
			std::swap(_back_cap, other._back_cap);
			std::swap(_end, other._end);
			swap_allocator(_alloc, other._alloc);
		}

		constexpr void reset() noexcept {
			_front_cap = nullptr;
			_begin = nullptr;
			_end = nullptr;
			_back_cap = nullptr;
		}

		constexpr void copy_without_alloc(
			const split_buffer_pointer_layout& other
		) noexcept(std::is_nothrow_copy_assignable<pointer>::value) {
			_front_cap = other._front_cap;
			_begin = other._begin;
			_end = other._end;
			_back_cap = other._back_cap;
		}

	private:
		pointer _front_cap = nullptr;
		pointer _begin = nullptr;
		pointer _end = nullptr;
		pointer _back_cap = nullptr;
		PLUGIFY_NO_UNIQUE_ADDRESS allocator_type _alloc;

		template <class, class, class>
		friend class split_buffer_pointer_layout;
	};

	template <class SplitBuffer, class T, class Allocator>
	class split_buffer_size_layout {
	protected:
		using value_type = T;
		using allocator_type = Allocator;
		using alloc_rr = std::remove_reference_t<allocator_type>;
		using alloc_traits = std::allocator_traits<alloc_rr>;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = typename alloc_traits::size_type;
		using difference_type = typename alloc_traits::difference_type;
		using pointer = typename alloc_traits::pointer;
		using const_pointer = typename alloc_traits::const_pointer;
		using iterator = pointer;
		using constIterator = const_pointer;
		using sentinel_type = size_type;

	public:
		constexpr split_buffer_size_layout() = default;

		constexpr explicit split_buffer_size_layout(const allocator_type& alloc)
			: _alloc(alloc) {
		}

		constexpr pointer front_cap() noexcept {
			return _front_cap;
		}

		constexpr const_pointer front_cap() const noexcept {
			return _front_cap;
		}

		constexpr pointer begin() noexcept {
			return _begin;
		}

		constexpr const_pointer begin() const noexcept {
			return _begin;
		}

		constexpr pointer end() noexcept {
			return _begin + _size;
		}

		constexpr pointer end() const noexcept {
			return _begin + _size;
		}

		constexpr size_type size() const noexcept {
			return _size;
		}

		constexpr bool empty() const noexcept {
			return _size == 0;
		}

		constexpr size_type capacity() const noexcept {
			return _cap;
		}

		constexpr allocator_type& get_allocator() noexcept {
			return _alloc;
		}

		constexpr const allocator_type& get_allocator() const noexcept {
			return _alloc;
		}

		// Returns the sentinel object directly. Should be used in conjunction with automatic type
		// deduction, not explicit types.
		constexpr sentinel_type raw_sentinel() const noexcept {
			return _size;
		}

		constexpr sentinel_type raw_capacity() const noexcept {
			return _cap;
		}

		constexpr void set_data(pointer new_first) noexcept {
			_front_cap = new_first;
		}

		constexpr void
		set_valid_range(pointer new_begin, pointer new_end) noexcept {
			// Size-based split_buffers track their size directly: we need to explicitly update the size
			// when the front is adjusted.
			_size -= new_begin - _begin;
			_begin = new_begin;
			set_sentinel(new_end);
		}

		constexpr void
		set_valid_range(pointer new_begin, size_type new_size) noexcept {
			// Size-based split_buffers track their size directly: we need to explicitly update the size
			// when the front is adjusted.
			_size -= new_begin - _begin;
			_begin = new_begin;
			set_sentinel(new_size);
		}

		constexpr void set_sentinel(pointer new_end) noexcept {
			_LIBCPP_ASSERT_INTERNAL(_front_cap <= new_end, "new_end cannot precede _front_cap");
			_size += new_end - end();
		}

		constexpr void set_sentinel(size_type new_size) noexcept {
			_size = new_size;
		}

		constexpr void set_capacity(size_type new_capacity) noexcept {
			_cap = new_capacity;
		}

		constexpr void set_capacity(pointer new_capacity) noexcept {
			_cap = new_capacity - _begin;
		}

		constexpr size_type front_spare() const noexcept {
			return static_cast<size_type>(_begin - _front_cap);
		}

		constexpr size_type back_spare() const noexcept {
			// `_cap - _end` tells us the total number of spares when in size-mode. We need to remove
			// the front_spare from the count.
			return _cap - _size - front_spare();
		}

		constexpr reference back() noexcept {
			return _begin[_size - 1];
		}

		constexpr const_reference back() const noexcept {
			return _begin[_size - 1];
		}

		constexpr void swap_without_allocator(
			split_buffer_pointer_layout<
				split_buffer<value_type, alloc_rr&, split_buffer_pointer_layout>,
				value_type,
				alloc_rr&>& other
		) noexcept {
			std::swap(_front_cap, other._front_cap);
			std::swap(_begin, other._begin);
			std::swap(_cap, other._cap);
			std::swap(_size, other._size);
		}

		constexpr void swap(split_buffer_size_layout& other) noexcept {
			std::swap(_front_cap, other._front_cap);
			std::swap(_begin, other._begin);
			std::swap(_cap, other._cap);
			std::swap(_size, other._size);
			swap_allocator(_alloc, other._alloc);
		}

		constexpr void reset() noexcept {
			_front_cap = nullptr;
			_begin = nullptr;
			_size = 0;
			_cap = 0;
		}

		constexpr void copy_without_alloc(
			const split_buffer_size_layout& other
		) noexcept(std::is_nothrow_copy_assignable<pointer>::value) {
			_front_cap = other._front_cap;
			_begin = other._begin;
			_cap = other._cap;
			_size = other._size;
		}

	private:
		pointer _front_cap = nullptr;
		pointer _begin = nullptr;
		size_type _size = 0;
		size_type _cap = 0;
		PLUGIFY_NO_UNIQUE_ADDRESS allocator_type _alloc;

		template <class, class, class>
		friend class split_buffer_size_layout;
	};

	// `split_buffer` is a contiguous array data structure. It may hold spare capacity at both ends of
	// the sequence. This allows for a `split_buffer` to grow from both the front and the back without
	// relocating its contents until it runs out of room. This characteristic sets it apart from
	// `std::vector`, which only holds spare capacity at its end. As such, `split_buffer` is useful
	// for implementing both `std::vector` and `std::deque`.
	//
	// The sequence is stored as a contiguous chunk of memory delimited by the following "pointers" (`o`
	// denotes uninitialized memory and `x` denotes a valid object):
	//
	//     |oooooooooooooooooooxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxoooooooooooooooooooooooo|
	//      ^                  ^                                    ^                       ^
	//  _front_cap        _begin                              _end               _back_cap
	//
	// The range [_front_cap, _begin) contains uninitialized memory. It is referred to as the "front
	// spare capacity". The range [_begin, _end) contains valid objects. It is referred to as the "valid
	// range". The range [_end, _back_cap) contains uninitialized memory. It is referred to as the "back
	// spare capacity".
	//
	// The layout of `split_buffer` is determined by the `Layout` template template parameter. This
	// `Layout` allows the above pointers to be stored as different representations, such as integer
	// offsets. A layout class template must provide the following interface:
	//
	//    template<class T, class Allocator, class Layout>
	//    class layout {
	//    protected:
	//      using value_type                     = T;
	//      using allocator_type                 = Allocator;
	//      using alloc_rr                     = std::remove_reference_t<allocator_type>;
	//      using alloc_traits                 = allocator_traits<alloc_rr>;
	//      using reference                      = value_type&;
	//      using const_reference                = const value_type&;
	//      using size_type                      = typename alloc_traits::size_type;
	//      using difference_type                = typename alloc_traits::difference_type;
	//      using pointer                        = typename alloc_traits::pointer;
	//      using const_pointer                  = typename alloc_traits::const_pointer;
	//      using iterator                       = pointer;
	//      using constIterator                 = const_pointer;
	//      using sentinel_type                = /* type that represents the layout's sentinel */;
	//
	//    public:
	//      layout() = default;
	//      explicit layout(const allocator_type&);
	//
	//      pointer front_cap();
	//      const_pointer front_cap() const;
	//
	//      pointer begin();
	//      const_pointer begin() const;
	//
	//      pointer end();
	//      pointer end() const;
	//
	//      size_type size() const;
	//      bool empty() const;
	//      size_type capacity() const;
	//
	//      allocator_type& get_allocator();
	//      allocator_type const& get_allocator() const;
	//
	//      sentinel_type raw_sentinel() const;
	//      sentinel_type raw_capacity() const;
	//
	//      void set_data(pointer);
	//      void set_valid_range(pointer begin, pointer end);
	//      void set_valid_range(pointer begin, size_type size);
	//      void set_sentinel(pointer end);
	//      void set_sentinel(size_type size);
	//
	//      void set_capacity(size_type capacity);
	//      void set_capacity(pointer capacity);
	//
	//      size_type front_spare() const;
	//      size_type back_spare() const;
	//
	//      reference back();
	//      const_reference back() const;
	//
	//      template<class _OtherLayout>
	//      void swap_without_allocator(_OtherLayout&);
	//      void swap(layout&);
	//
	//      void reset();
	//      void copy_without_alloc(layout const&);
	//    };
	//
	template <class T, class Allocator, template <class, class, class> class Layout>
	class split_buffer : Layout<split_buffer<T, Allocator, Layout>, T, Allocator> {
		using base_type = Layout<split_buffer<T, Allocator, Layout>, T, Allocator>;

	public:
		using base_type::back_spare;
		using base_type::copy_without_alloc;
		using base_type::front_cap;
		using base_type::front_spare;
		using base_type::get_allocator;
		using base_type::raw_capacity;
		using base_type::raw_sentinel;
		using base_type::reset;
		using base_type::set_capacity;
		using base_type::set_data;
		using base_type::set_sentinel;
		using base_type::set_valid_range;

		using typename base_type::alloc_rr;
		using typename base_type::alloc_traits;
		using typename base_type::allocator_type;
		using typename base_type::constIterator;
		using typename base_type::const_pointer;
		using typename base_type::const_reference;
		using typename base_type::difference_type;
		using typename base_type::iterator;
		using typename base_type::pointer;
		using typename base_type::reference;
		using typename base_type::size_type;
		using typename base_type::value_type;

		// A split_buffer contains the following members which may be trivially relocatable:
		// - pointer: may be trivially relocatable, so it's checked
		// - allocator_type: may be trivially relocatable, so it's checked
		// split_buffer doesn't have any self-references, so it's trivially relocatable if its members
		// are.
		using trivially_relocatable = std::conditional_t<
			is_trivially_relocatable<pointer>::value &&
			is_trivially_relocatable<allocator_type>::value,
			split_buffer,
			void>;

		split_buffer(const split_buffer&) = delete;
		split_buffer& operator=(const split_buffer&) = delete;

		split_buffer() = default;

		constexpr explicit split_buffer(alloc_rr& a)
			: base_type(a) {
		}

		constexpr explicit split_buffer(const alloc_rr& a)
			: base_type(a) {
		}

		constexpr split_buffer(size_type cap, size_type start, alloc_rr& a);

		constexpr
		split_buffer(split_buffer&& c) noexcept(std::is_nothrow_move_constructible_v<allocator_type>);

		constexpr split_buffer(split_buffer&& c, const alloc_rr& a);


		constexpr split_buffer& operator=(split_buffer&& c) noexcept(
			(alloc_traits::propagate_on_container_move_assignment::value
			 && std::is_nothrow_move_assignable_v<allocator_type>)
			|| !alloc_traits::propagate_on_container_move_assignment::value
		);

		constexpr ~split_buffer();

		using base_type::back;
		using base_type::begin;
		using base_type::capacity;
		using base_type::empty;
		using base_type::end;
		using base_type::size;

		constexpr void clear() noexcept {
			destruct_at_end(begin());
		}

		constexpr reference front() {
			return *begin();
		}

		constexpr const_reference front() const {
			return *begin();
		}

		constexpr void shrink_to_fit() noexcept;

		template <class... Args>
		constexpr void emplace_front(Args&&... args);
		template <class... Args>
		constexpr void emplace_back(Args&&... args);

		constexpr void pop_front() {
			destruct_at_begin(begin() + 1);
		}

		constexpr void pop_back() {
			destruct_at_end(end() - 1);
		}

		constexpr void construct_at_end(size_type n);
		constexpr void construct_at_end(size_type n, const_reference x);

		template <std::forward_iterator ForwardIterator>
		constexpr void
		construct_at_end(ForwardIterator first, ForwardIterator last);

		template <class Iterator, class Sentinel>
		constexpr void
		construct_at_end_with_sentinel(Iterator first, Sentinel last);

		template <class Iterator>
		constexpr void construct_at_end_with_size(Iterator first, size_type n);

		constexpr void destruct_at_begin(pointer new_begin) {
			destruct_at_begin(new_begin, std::is_trivially_destructible<value_type>());
		}

		constexpr void destruct_at_begin(pointer new_begin, std::false_type);
		constexpr void destruct_at_begin(pointer new_begin, std::true_type);

		constexpr void destruct_at_end(pointer new_last) noexcept {
			destruct_at_end(new_last, std::false_type());
		}

		constexpr void destruct_at_end(pointer new_last, std::false_type) noexcept;
		constexpr void destruct_at_end(pointer new_last, std::true_type) noexcept;

		constexpr void swap(
			split_buffer& x
		) noexcept(!alloc_traits::propagate_on_container_swap::value || std::is_nothrow_swappable_v<alloc_rr>);

		constexpr bool invariants() const {
			if (front_cap() == nullptr) {
				if (begin() != nullptr) {
					return false;
				}

				if (!empty()) {
					return false;
				}

				if (capacity() != 0) {
					return false;
				}

				return true;
			} else {
				if (begin() < front_cap()) {
					return false;
				}

				if (capacity() < size()) {
					return false;
				}

				if (end() < begin()) {
					return false;
				}

				return true;
			}
		}

		constexpr void
		swap_without_allocator(split_buffer<value_type, alloc_rr&, Layout>& other) noexcept {
			base_type::swap_without_allocator(other);
		}

	private:
		constexpr void move_assign_alloc(split_buffer& c, std::true_type) noexcept(
			std::is_nothrow_move_assignable_v<allocator_type>
		) {
			get_allocator() = std::move(c.get_allocator());
		}

		constexpr void move_assign_alloc(split_buffer&, std::false_type) noexcept {
		}

		struct ConstructTransaction {
			constexpr explicit ConstructTransaction(
				split_buffer* parent,
				pointer p,
				size_type n
			) noexcept
				: _pos(p)
				, _end(p + n)
				, _parent(parent) {
			}

			constexpr ~ConstructTransaction() {
				_parent->set_sentinel(_pos);
			}

			pointer _pos;
			const pointer _end;

		private:
			split_buffer* _parent;
		};

		template <class T2, class A2, template <class, class, class> class L2>
		friend class split_buffer;
	};

	//  Default constructs n objects starting at `end()`
	//  throws if construction throws
	//  Precondition:  n > 0
	//  Precondition:  size() + n <= capacity()
	//  Postcondition:  size() == size() + n
	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr void split_buffer<T, Allocator, Layout>::construct_at_end(size_type n) {
		ConstructTransaction tx(this, end(), n);
		for (; tx._pos != tx._end; ++tx._pos) {
			alloc_traits::construct(get_allocator(), std::to_address(tx._pos));
		}
	}

	//  Copy constructs n objects starting at `end()` from x
	//  throws if construction throws
	//  Precondition:  n > 0
	//  Precondition:  size() + n <= capacity()
	//  Postcondition:  size() == old size() + n
	//  Postcondition:  [i] == x for all i in [size() - n, n)
	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr void
	split_buffer<T, Allocator, Layout>::construct_at_end(size_type n, const_reference x) {
		ConstructTransaction tx(this, end(), n);
		for (; tx._pos != tx._end; ++tx._pos) {
			alloc_traits::construct(get_allocator(), std::to_address(tx._pos), x);
		}
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	template <class Iterator, class Sentinel>
	constexpr void
	split_buffer<T, Allocator, Layout>::construct_at_end_with_sentinel(Iterator first, Sentinel last) {
		alloc_rr& a = get_allocator();
		for (; first != last; ++first) {
			if (back_spare() == 0) {
				size_type old_cap = capacity();
				size_type new_cap = std::max<size_type>(2 * old_cap, 8);
				split_buffer buf(new_cap, 0, a);
				pointer buf_end = buf.end();
				pointer cur_end = end();
				for (pointer p = begin(); p != cur_end; ++p) {
					alloc_traits::construct(buf.get_allocator(), std::to_address(buf_end), std::move(*p));
					buf.set_sentinel(++buf_end);
				}
				swap(buf);
			}

			alloc_traits::construct(a, std::to_address(end()), *first);
			set_sentinel(size() + 1);
		}
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	template <std::forward_iterator ForwardIterator>
	constexpr void
	split_buffer<T, Allocator, Layout>::construct_at_end(ForwardIterator first, ForwardIterator last) {
		construct_at_end_with_size(first, std::distance(first, last));
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	template <class ForwardIterator>
	constexpr void
	split_buffer<T, Allocator, Layout>::construct_at_end_with_size(ForwardIterator first, size_type n) {
		ConstructTransaction tx(this, end(), n);
		for (; tx._pos != tx._end; ++tx._pos, (void) ++first) {
			alloc_traits::construct(get_allocator(), std::to_address(tx._pos), *first);
		}
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	inline constexpr void
	split_buffer<T, Allocator, Layout>::destruct_at_begin(pointer new_begin, std::false_type) {
		pointer pos = begin();
		// Updating begin at every iteration is unnecessary because destruction can't throw.
		while (pos != new_begin) {
			alloc_traits::destroy(get_allocator(), std::to_address(pos++));
		}
		set_valid_range(pos, end());
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	inline constexpr void
	split_buffer<T, Allocator, Layout>::destruct_at_begin(pointer new_begin, std::true_type) {
		set_valid_range(new_begin, end());
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	inline constexpr void
	split_buffer<T, Allocator, Layout>::destruct_at_end(pointer new_last, std::false_type) noexcept {
		pointer cur_end = end();
		// Updating begin at every iteration is unnecessary because destruction can't throw.
		while (new_last != cur_end) {
			alloc_traits::destroy(get_allocator(), std::to_address(--cur_end));
		}
		set_sentinel(cur_end);
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr split_buffer<T, Allocator, Layout>::split_buffer(size_type cap, size_type start, alloc_rr& a)
		: base_type(a) {
		PLUGIFY_ASSERT(cap >= start, "can't have a start point outside the capacity");
		if (cap > 0) {
			auto allocation = allocate_at_least(get_allocator(), cap);
			set_data(allocation.ptr);
			cap = allocation.count;
		}

		pointer pos = front_cap() + start;
		set_valid_range(pos, pos);
		set_capacity(cap);
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr split_buffer<T, Allocator, Layout>::~split_buffer() {
		clear();
		if (front_cap()) {
			alloc_traits::deallocate(get_allocator(), front_cap(), capacity());
		}
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr split_buffer<T, Allocator, Layout>::split_buffer(split_buffer&& c) noexcept(
		std::is_nothrow_move_constructible_v<allocator_type>
	)
		: base_type(std::move(c)) {
		c.reset();
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr split_buffer<T, Allocator, Layout>::split_buffer(split_buffer&& c, const alloc_rr& a)
		: base_type(a) {
		if (a == c.get_allocator()) {
			set_data(c.front_cap());
			set_valid_range(c.begin(), c.end());
			set_capacity(c.capacity());
			c.reset();
		} else {
			auto allocation = allocate_at_least(get_allocator(), c.size());
			set_data(allocation.ptr);
			set_valid_range(front_cap(), front_cap());
			set_capacity(allocation.count);
			using Ip = std::move_iterator<iterator>;
			construct_at_end(Ip(c.begin()), Ip(c.end()));
		}
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr split_buffer<T, Allocator, Layout>&
	split_buffer<T, Allocator, Layout>::operator=(split_buffer&& c) noexcept(
		(alloc_traits::propagate_on_container_move_assignment::value
		 && std::is_nothrow_move_assignable_v<allocator_type>)
		|| !alloc_traits::propagate_on_container_move_assignment::value
	) {
		clear();
		shrink_to_fit();
		copy_without_alloc(c);
		move_assign_alloc(
			c,
			std::integral_constant<bool, alloc_traits::propagate_on_container_move_assignment::value>()
		);
		c.reset();
		return *this;
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr void split_buffer<T, Allocator, Layout>::swap(
		split_buffer& x
	) noexcept(!alloc_traits::propagate_on_container_swap::value || std::is_nothrow_swappable_v<alloc_rr>) {
		base_type::swap(x);
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	constexpr void split_buffer<T, Allocator, Layout>::shrink_to_fit() noexcept {
		if (capacity() > size()) {
#if PLUGIFY_HAS_EXCEPTIONS
			try {
#endif	// PLUGIFY_HAS_EXCEPTIONS
				split_buffer<value_type, alloc_rr&, Layout> t(size(), 0, get_allocator());
				if (t.capacity() < capacity()) {
					t.construct_at_end(std::move_iterator<pointer>(begin()), std::move_iterator<pointer>(end()));
					t.set_sentinel(size());
					swap_without_allocator(t);
				}
#if PLUGIFY_HAS_EXCEPTIONS
			} catch (...) {
			}
#endif	// PLUGIFY_HAS_EXCEPTIONS
		}
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	template <class... Args>
	constexpr void split_buffer<T, Allocator, Layout>::emplace_front(Args&&... args) {
		if (front_spare() == 0) {
			pointer cur_end = end();
			if (back_spare() > 0) {
				// The elements are pressed up against the front of the buffer: we need to move them
				// back a little bit to make `emplace_front` have amortised O(1) complexity.
				difference_type d = back_spare();
				d = (d + 1) / 2;
				auto new_end = cur_end + d;
				set_valid_range(std::move_backward(begin(), cur_end, new_end), new_end);
			} else {
				size_type c = std::max<size_type>(2 * capacity(), 1);
				split_buffer<value_type, alloc_rr&, Layout> t(c, (c + 3) / 4, get_allocator());
				t.construct_at_end(std::move_iterator<pointer>(begin()), std::move_iterator<pointer>(cur_end));
				base_type::swap_without_allocator(t);
			}
		}

		alloc_traits::construct(get_allocator(), std::to_address(begin() - 1), std::forward<Args>(args)...);
		set_valid_range(begin() - 1, size() + 1);
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	template <class... Args>
	constexpr void split_buffer<T, Allocator, Layout>::emplace_back(Args&&... args) {
		pointer cur_end = end();
		if (back_spare() == 0) {
			if (front_spare() > 0) {
				difference_type d = front_spare();
				d = (d + 1) / 2;
				cur_end = std::move(begin(), cur_end, begin() - d);
				set_valid_range(begin() - d, cur_end);
			} else {
				size_type c = std::max<size_type>(2 * capacity(), 1);
				split_buffer<value_type, alloc_rr&, Layout> t(c, c / 4, get_allocator());
				t.construct_at_end(std::move_iterator<pointer>(begin()), std::move_iterator<pointer>(cur_end));
				base_type::swap_without_allocator(t);
			}
		}

		alloc_traits::construct(get_allocator(), std::to_address(cur_end), std::forward<Args>(args)...);
		set_sentinel(++cur_end);
	}

	template <class T, class Allocator, template <class, class, class> class Layout>
	inline constexpr void
	swap(split_buffer<T, Allocator, Layout>& x, split_buffer<T, Allocator, Layout>& y) noexcept(
		noexcept(x.swap(y))
	) {
		x.swap(y);
	}
} // namespace plg
