#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <atomic>
#include <string_view>
#include <type_traits>

#include <plugify/compat_format.h>

// Adapted from https://github.com/klementtan/string_cpp/

namespace plg {
	namespace detail {
		template<typename Allocator, typename = void>
		struct is_allocator : std::false_type {};

		template<typename Allocator>
		struct is_allocator<Allocator, std::void_t<typename Allocator::value_type, decltype(std::declval<Allocator&>().allocate(std::size_t{}))>> : std::true_type {};

		template<typename Allocator>
		constexpr inline bool is_allocator_v = is_allocator<Allocator>::value;

		struct uninitialized_size_tag {};
	}// namespace detail

	// Memory layout for the different type of strings
    	// byte     [00] [01] [02] [03] [04] [05] [06] [07] [08] [09] [10] [11] [12] [13] [14] [15] [16] [17] [18] [19] [20] [21] [22] [23]
    	// small:   [value0                                                                                                   value22] [l ]
    	// medium:  [pointer                              ] [len                                  ] [cap                                  ]
    	// large:   [cb_pointer                           ] [len                                  ] [cap                                  ]
	template<class Alloc>
	class basic_string {
	public:
		typedef Alloc allocator_type;
		typedef std::allocator_traits<allocator_type> __alloc_traits;
		typedef typename __alloc_traits::size_type size_type;
		typedef typename __alloc_traits::pointer pointer;
		typedef typename __alloc_traits::const_pointer const_pointer;
		typedef ptrdiff_t difference_type;
		typedef char value_type; // Could be potentially expand to different types
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef pointer iterator;
		typedef const_pointer const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef std::basic_string_view<value_type> basic_string_view;

		static const size_type npos = static_cast<size_type>(-1);

		basic_string() noexcept(std::is_nothrow_default_constructible<allocator_type>::value);
		explicit basic_string(const allocator_type& a) requires (detail::is_allocator_v<Alloc>);
		basic_string(const basic_string& str);
		basic_string(const value_type* s, const allocator_type& a = allocator_type()) requires (detail::is_allocator_v<Alloc>);
		basic_string(basic_string&& str) noexcept(std::is_nothrow_move_constructible<allocator_type>::value);
		basic_string(const basic_string& str, size_type pos, size_type n = npos, const allocator_type& a = allocator_type()) requires (detail::is_allocator_v<Alloc>);
		template<class T>
		basic_string(const T& t, size_type pos, size_type n, const allocator_type& a = allocator_type()) requires (detail::is_allocator_v<Alloc>);
		template<class T>
		basic_string(const T& t, const allocator_type& a = allocator_type()) requires (detail::is_allocator_v<Alloc>);
		basic_string(const value_type* s, size_type n, const allocator_type& a = allocator_type()) requires (detail::is_allocator_v<Alloc>);
		basic_string(size_type n, value_type c, const allocator_type& a = allocator_type()) requires (detail::is_allocator_v<Alloc>);
		template<class InputIterator>
		basic_string(InputIterator first, InputIterator last, const allocator_type& a = allocator_type()) requires (detail::is_allocator_v<Alloc>);
		basic_string(std::initializer_list<value_type>, const allocator_type& = allocator_type()) requires (detail::is_allocator_v<Alloc>);
		basic_string(const basic_string&, const allocator_type&) requires (detail::is_allocator_v<Alloc>);
		basic_string(basic_string&&, const allocator_type&) requires (detail::is_allocator_v<Alloc>);
		explicit basic_string(detail::uninitialized_size_tag, size_type size, const allocator_type &a) requires (detail::is_allocator_v<Alloc>);
		~basic_string();

		// Allocator
		[[nodiscard]] const allocator_type& get_allocator() const noexcept;
		[[nodiscard]] allocator_type& get_allocator() noexcept;
		void set_allocator(const allocator_type& allocator) requires (detail::is_allocator_v<Alloc>);

		[[nodiscard]] operator basic_string_view() const noexcept;

		[[nodiscard]] bool empty() const noexcept;
		[[nodiscard]] size_type size() const noexcept;
		[[nodiscard]] size_type length() const noexcept;
		[[nodiscard]] size_type capacity() const noexcept;
		[[nodiscard]] const_reference operator[](size_type pos) const;
		[[nodiscard]] reference operator[](size_type pos);
		[[nodiscard]] value_type* data() noexcept;
		[[nodiscard]] const value_type* data() const noexcept;
		[[nodiscard]] const value_type* c_str() const noexcept;

		[[nodiscard]] const_reference at(size_type pos) const;
		[[nodiscard]] reference at(size_type pos);

		[[nodiscard]] iterator begin() noexcept;
		[[nodiscard]] const_iterator begin() const noexcept;
		[[nodiscard]] iterator end() noexcept;
		[[nodiscard]] const_iterator end() const noexcept;

		[[nodiscard]] reverse_iterator rbegin() noexcept;
		[[nodiscard]] const_reverse_iterator rbegin() const noexcept;
		[[nodiscard]] reverse_iterator rend() noexcept;
		[[nodiscard]] const_reverse_iterator rend() const noexcept;

		[[nodiscard]] const_iterator cbegin() const noexcept;
		[[nodiscard]] const_iterator cend() const noexcept;
		[[nodiscard]] const_reverse_iterator crbegin() const noexcept;
		[[nodiscard]] const_reverse_iterator crend() const noexcept;

		[[nodiscard]] reference front();
		[[nodiscard]] const_reference front() const;
		[[nodiscard]] reference back();
		[[nodiscard]] const_reference back() const;

		void push_back(value_type c);
		void pop_back();

		size_type copy(value_type* s, size_type n, size_type pos = 0) const;
		basic_string substr(size_type pos = 0, size_type n = npos) const;

		/** Most std:: namespace containers never shrink. I want to point out that this may be an anti-feature. **/
		/* void shrink_to_fit(); */
		void reserve(size_type n = 0);
		void resize(size_type n, value_type c);
		void resize(size_type n);
		void swap(basic_string& str) noexcept(std::allocator_traits<allocator_type>::propagate_on_container_swap::value || std::allocator_traits<allocator_type>::is_always_equal::value);

		basic_string& erase(size_type pos = 0, size_type n = npos);
		iterator erase(const_iterator position);
		iterator erase(const_iterator first, const_iterator last);
		reverse_iterator erase(reverse_iterator position);
		reverse_iterator erase(reverse_iterator first, reverse_iterator last);
		void clear() noexcept;

		basic_string& assign(const basic_string& str);
		template<typename T>
		basic_string& assign(const T& t);
		basic_string& assign(basic_string&& str);
		basic_string& assign(const basic_string& str, size_type pos, size_type n = npos);
		template<typename T>
		basic_string& assign(const T& t, size_type pos, size_type n = npos);
		basic_string& assign(const value_type* s, size_type n);
		basic_string& assign(const value_type* s);
		basic_string& assign(size_type n, value_type c);
		//template<class InputIterator>
		//basic_string& assign(InputIterator first, InputIterator last); TODO: Fix
		basic_string& assign(const value_type* first, const value_type* last);
		basic_string& assign(std::initializer_list<value_type>);

		// TODO: Implement
		/*basic_string &insert(size_type pos1, const basic_string &str);
		template <class T>
		basic_string &insert(size_type pos1, const T &t);
		basic_string &insert(size_type pos1, const basic_string &str, size_type pos2, size_type n);
		template <class T>
		basic_string &insert(size_type pos1, const T &t, size_type pos2, size_type n);
		basic_string &insert(size_type pos, const value_type *s, size_type n = npos);
		basic_string &insert(size_type pos, const value_type *s);
		basic_string &insert(size_type pos, size_type n, value_type c);
		iterator insert(const_iterator p, value_type c);
		iterator insert(const_iterator p, size_type n, value_type c);
		template <class InputIterator>
		iterator insert(const_iterator p, InputIterator first, InputIterator last);
		iterator insert(const_iterator p, std::initializer_list<value_type>);*/

		// TODO: Implement replace + find methods

		basic_string& operator=(const basic_string& str);
		template<class T>
		basic_string& operator=(const T& t);
		basic_string& operator=(basic_string&& str) noexcept(allocator_type::propagate_on_container_move_assignment::value || allocator_type::is_always_equal::value);
		basic_string& operator=(const value_type* s);
		basic_string& operator=(value_type c);
		basic_string& operator=(std::initializer_list<value_type>);

		[[nodiscard]] bool operator==(const value_type* str) const noexcept;
		[[nodiscard]] bool operator==(const basic_string& str) const noexcept;
		template<typename T>
		[[nodiscard]] bool operator==(const T& t) const noexcept;

		basic_string& operator+=(const basic_string& str);
		template<class T>
		basic_string& operator+=(const T& t);
		basic_string& operator+=(const value_type* s);
		basic_string& operator+=(value_type c);
		basic_string& operator+=(std::initializer_list<value_type>);

		basic_string& append(const basic_string& str);
		template<class T>
		basic_string& append(const T& t);
		basic_string& append(const basic_string& str, size_type pos, size_type n = npos);
		template<class T>
		basic_string& append(const T& t, size_type pos, size_type n = npos);
		basic_string& append(const value_type* s, size_type n);
		basic_string& append(const value_type* s);
		basic_string& append(size_type n, value_type c);
		template<class InputIterator>
		basic_string& append(InputIterator first, InputIterator last);
		basic_string& append(std::initializer_list<value_type>);

		[[nodiscard]] int compare(const basic_string& str) const noexcept;
		template<class T>
		[[nodiscard]] int compare(const T& t) const noexcept;
		[[nodiscard]] int compare(size_type pos1, size_type n1, const basic_string& str) const;
		template<class T>
		[[nodiscard]] int compare(size_type pos1, size_type n1, const T& t) const;
		[[nodiscard]] int compare(size_type pos1, size_type n1, const basic_string& str, size_type pos2, size_type n2 = npos) const;
		template<class T>
		[[nodiscard]] int compare(size_type pos1, size_type n1, const T& t, size_type pos2, size_type n2 = npos) const;
		[[nodiscard]] int compare(const value_type* s) const noexcept;
		[[nodiscard]] int compare(size_type pos1, size_type n1, const value_type* s) const;
		[[nodiscard]] int compare(size_type pos1, size_type n1, const value_type* s, size_type pos2, size_type n2 = npos) const;
		[[nodiscard]] static int compare(const value_type* begin1, const value_type* end1, const value_type* begin2, const value_type* end2);

	private:
		static constexpr size_type __string_max_size = sizeof(size_type) * 2 + sizeof(pointer);
		static constexpr size_type __max_short_size = 23;
		static constexpr size_type __max_mid_size = 255;
		static constexpr value_type __terminator = 0;
		static inline value_type _ = __terminator; // for no-return

		enum class Category {
			Short = 0,
			Mid,
			Long
		};

#if _MSC_VER
#define no_unique_address msvc::no_unique_address
#endif
		class ControlBlock {
			pointer _ptr;
			std::atomic<size_type> _count;
			size_type _len;
			size_type _cap;
			[[no_unique_address]] allocator_type _allocator;

		public:
			ControlBlock(pointer p, size_type len, size_type cap, const allocator_type& a = allocator_type());

			ControlBlock(const ControlBlock& other);
			ControlBlock& operator=(const ControlBlock&) = delete;
			ControlBlock(ControlBlock&&) = delete;
			ControlBlock& operator=(ControlBlock&& p) = delete;
			~ControlBlock();

			ControlBlock* acquire() noexcept;
			pointer get() const noexcept;
			size_type count() const noexcept;
			void release();
		};

		struct Short {
			value_type _data[(__string_max_size - sizeof(uint8_t)) / sizeof(value_type)]{};
			uint8_t _len{};
		};

		struct Mid {
			pointer _ptr{};
			size_type _len{};
			size_type _cap{};
		};

		struct Long {
			ControlBlock* _cbptr{};
			size_type _len{};
			size_type _cap{};
		};

		static_assert(sizeof(Short) <= __string_max_size,
					  "Short struct is larger than max size");
		static_assert(sizeof(Mid) <= __string_max_size,
					  "Mid struct is larger than max size");
		static_assert(sizeof(Long) <= __string_max_size,
					  "Long struct is larger than max size");

		struct Members {
			union {
				Short _short;
				Mid _mid;
				Long _long;
			};
			[[no_unique_address]] allocator_type _allocator{};
		} _members;

		static_assert(sizeof(Members) <= __string_max_size,
					  "Layout struct is larger than max size");

		Category category() const noexcept;
		static Category get_category(size_type cap) noexcept;

		[[nodiscard]] bool msb() const noexcept;

		[[nodiscard]] uint8_t& msbyte() noexcept;
		[[nodiscard]] uint8_t cmsbyte() const noexcept;

		[[nodiscard]] value_type* begin_ptr() noexcept;
		[[nodiscard]] const value_type* begin_ptr() const noexcept;
		[[nodiscard]] value_type* end_ptr() noexcept;
		[[nodiscard]] const value_type* end_ptr() const noexcept;

		[[nodiscard]] pointer allocate(size_type n);
		void deallocate(pointer ptr, size_type n);
		void deallocate_self();

		[[nodiscard]] size_type remaining_capacity() const noexcept;
		[[nodiscard]] static size_type gen_capacity(size_type len) noexcept;
		void set_capacity(size_type cap);
		void set_length(size_type len, Category cat);
		void set_heap(pointer ptr, size_type len, size_type cap);

		void construct_internal(const value_type* s);
		void construct_internal(const value_type* s, size_type len);
		void construct_internal(const basic_string& str);
		template<class InputIterator>
		void construct_internal(InputIterator first, InputIterator last);
		void construct_internal(size_type n, value_type c);

		void construct_string_empty();
		void construct_string_short(const value_type* s, size_type len, size_type cap);
		void construct_string_mid(const value_type* s, size_type len, size_type cap);
		void construct_string_long(const value_type* s, size_type len, size_type cap);

		void mutable_cb();
	};

	template<class Alloc>
	void basic_string<Alloc>::set_allocator(const allocator_type& allocator) requires (detail::is_allocator_v<Alloc>) {
		_members._allocator = allocator;
		// TODO: What if new allocator was set ?
	}

	template<class Alloc>
	typename basic_string<Alloc>::allocator_type&
	basic_string<Alloc>::get_allocator() noexcept {
		return _members._allocator;
	}

	template<class Alloc>
	const typename basic_string<Alloc>::allocator_type&
	basic_string<Alloc>::get_allocator() const noexcept {
		return _members._allocator;
	}

	using string = basic_string<std::allocator<char>>;

#pragma region Constructors

	template<class Alloc>
	basic_string<Alloc>::~basic_string() {
		deallocate_self();
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string(const value_type* s, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_internal(s);
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string() noexcept(std::is_nothrow_default_constructible<allocator_type>::value) : _members{{}, allocator_type()} {
		construct_string_empty();
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string(const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_string_empty();
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string(const basic_string& str) : basic_string(str, allocator_type()) {
	}

	template<typename Alloc>
	basic_string<Alloc>::basic_string(basic_string&& str) noexcept(std::is_nothrow_move_constructible<allocator_type>::value) : _members{str._members} {
		str._members = {};
	}

	template<typename Alloc>
	basic_string<Alloc>::basic_string(const basic_string& str, size_type pos, size_type n, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_internal(
				str.begin() + pos,
				str.begin() + pos + std::min(n, str.size() - pos));
	}

	template<typename Alloc>
	template<class T>
	basic_string<Alloc>::basic_string(const T& t, size_type pos, size_type n, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		const basic_string_view str = t;
		if (pos > str.size()) {
			throw std::out_of_range("basic_string::basic_string -- out of range");
		}
		const auto len_to_assign = (n == npos || pos + n > str.size()) ? str.size() - pos : n;
		construct_internal(str.data() + pos, len_to_assign);
	}

	template<typename Alloc>
	template<class T>
	basic_string<Alloc>::basic_string(const T& t, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		const basic_string_view str = t;
		construct_internal(str.data(), str.size());
	}

	template<typename Alloc>
	basic_string<Alloc>::basic_string(const value_type* s, size_type n, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_internal(s, n);
	}

	template<typename Alloc>
	basic_string<Alloc>::basic_string(size_type n, value_type c, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_internal(n, c);
	}

	template<typename Alloc>
	template<class InputIterator>
	basic_string<Alloc>::basic_string(InputIterator first, InputIterator last, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_internal(first, last);
	}

	template<typename Alloc>
	basic_string<Alloc>::basic_string(std::initializer_list<value_type> ilist, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_internal(ilist.begin(), ilist.size());
	}

	template<typename Alloc>
	basic_string<Alloc>::basic_string(const basic_string& str, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_internal(str);
	}

	template<typename Alloc>
	basic_string<Alloc>::basic_string(basic_string&& str, const allocator_type& a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		if (get_allocator() == str.get_allocator()) {
			_members = str._members;
			str._members = {};
		} else if (str.begin()) {
			construct_internal(str);
		}
	}

	template<typename Alloc>
	basic_string<Alloc>::basic_string(detail::uninitialized_size_tag, size_type size, const allocator_type &a) requires (detail::is_allocator_v<Alloc>) : _members{{}, a} {
		construct_string_empty();
		reserve(size);
	}

#pragma endregion Constructors

#pragma region Basic
	template<class Alloc>
	typename basic_string<Alloc>::value_type*
	basic_string<Alloc>::data() noexcept {
		switch (category()) {
			case Category::Short:
				return _members._short._data;
			case Category::Mid:
				return _members._mid._ptr;
			case Category::Long:
				return _members._long._cbptr->get();
		}
		return nullptr;
	}

	template<class Alloc>
	const typename basic_string<Alloc>::value_type*
	basic_string<Alloc>::data() const noexcept {
		switch (category()) {
			case Category::Short:
				return _members._short._data;
			case Category::Mid:
				return _members._mid._ptr;
			case Category::Long:
				return _members._long._cbptr->get();
		}
		return nullptr;
	}

	template<class Alloc>
	const typename basic_string<Alloc>::value_type*
	basic_string<Alloc>::c_str() const noexcept {
		return data();
	}

	template<class Alloc>
	typename basic_string<Alloc>::size_type
	basic_string<Alloc>::length() const noexcept {
		switch (category()) {
			case Category::Short:
				return _members._short._len ^ (1 << 7);
			case Category::Mid:
				return _members._mid._len;
			case Category::Long:
				return _members._long._len;
		}
		return npos;
	}

	template<class Alloc>
	typename basic_string<Alloc>::size_type
	basic_string<Alloc>::size() const noexcept {
		return length();
	}

	template<class Alloc>
	bool basic_string<Alloc>::empty() const noexcept {
		return length() == 0;
	}

	template<class Alloc>
	typename basic_string<Alloc>::const_reference
	basic_string<Alloc>::operator[](typename basic_string<Alloc>::size_type pos) const {
		return c_str()[pos];
	}

	template<class Alloc>
	typename basic_string<Alloc>::reference
	basic_string<Alloc>::operator[](typename basic_string<Alloc>::size_type pos) {
		if (category() == Category::Long) {
			mutable_cb();
		}
		switch (category()) {
			case Category::Short:
				return _members._short._data[pos];
			case Category::Mid:
				return _members._mid._ptr[pos];
			case Category::Long:
				return _members._long._cbptr->get()[pos];
		}
		return _;
	}

	template<class Alloc>
	typename basic_string<Alloc>::const_reference
	basic_string<Alloc>::at(typename basic_string<Alloc>::size_type pos) const {
		if (pos > length()) {
			throw std::out_of_range("basic_string::at -- out of range");
		}
		return c_str()[pos];
	}

	template<class Alloc>
	typename basic_string<Alloc>::reference
	basic_string<Alloc>::at(typename basic_string<Alloc>::size_type pos) {
		if (pos > length()) {
			throw std::out_of_range("basic_string::at -- out of range");
		}
		if (category() == Category::Long) {
			mutable_cb();
		}
		switch (category()) {
			case Category::Short:
				return _members._short._data[pos];
			case Category::Mid:
				return _members._mid._ptr[pos];
			case Category::Long:
				return _members._long._cbptr->get()[pos];
		}
		return _;
	}

#pragma endregion Basic

#pragma region Iterators

	template<typename Alloc>
	typename basic_string<Alloc>::iterator
	basic_string<Alloc>::begin() noexcept {
		return begin_ptr();
	}

	template<typename Alloc>
	typename basic_string<Alloc>::iterator
	basic_string<Alloc>::end() noexcept {
		return end_ptr();
	}

	template<typename Alloc>
	typename basic_string<Alloc>::const_iterator
	basic_string<Alloc>::begin() const noexcept {
		return begin_ptr();
	}

	template<typename Alloc>
	typename basic_string<Alloc>::const_iterator
	basic_string<Alloc>::cbegin() const noexcept {
		return begin_ptr();
	}

	template<typename Alloc>
	typename basic_string<Alloc>::const_iterator
	basic_string<Alloc>::end() const noexcept {
		return end_ptr();
	}

	template<typename Alloc>
	typename basic_string<Alloc>::const_iterator
	basic_string<Alloc>::cend() const noexcept {
		return end_ptr();
	}

	template<typename Alloc>
	typename basic_string<Alloc>::reverse_iterator
	basic_string<Alloc>::rbegin() noexcept {
		return reverse_iterator(end_ptr());
	}

	template<typename Alloc>
	typename basic_string<Alloc>::reverse_iterator
	basic_string<Alloc>::rend() noexcept {
		return reverse_iterator(begin_ptr());
	}

	template<typename Alloc>
	typename basic_string<Alloc>::const_reverse_iterator
	basic_string<Alloc>::rbegin() const noexcept {
		return const_reverse_iterator(end_ptr());
	}

	template<typename Alloc>
	typename basic_string<Alloc>::const_reverse_iterator
	basic_string<Alloc>::crbegin() const noexcept {
		return const_reverse_iterator(end_ptr());
	}

	template<typename Alloc>
	typename basic_string<Alloc>::const_reverse_iterator
	basic_string<Alloc>::rend() const noexcept {
		return const_reverse_iterator(begin_ptr());
	}

	template<typename Alloc>
	typename basic_string<Alloc>::const_reverse_iterator
	basic_string<Alloc>::crend() const noexcept {
		return const_reverse_iterator(begin_ptr());
	}

	template<class Alloc>
	typename basic_string<Alloc>::reference
	basic_string<Alloc>::front() {
		return *begin_ptr();
	}

	template<class Alloc>
	typename basic_string<Alloc>::const_reference
	basic_string<Alloc>::front() const {
		return *begin_ptr();
	}

	template<class Alloc>
	typename basic_string<Alloc>::reference
	basic_string<Alloc>::back() {
		return *end_ptr();
	}

	template<class Alloc>
	typename basic_string<Alloc>::const_reference
	basic_string<Alloc>::back() const {
		return *end_ptr();
	}

#pragma endregion Iterators

	template<class Alloc>
	void basic_string<Alloc>::push_back(value_type c) {
		append((size_t) 1, c);
	}

	template<class Alloc>
	void basic_string<Alloc>::pop_back() {
		end()[-1] = __terminator;
		set_length(length() - 1, category());
	}

#pragma region Erase
	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::erase(size_type pos, size_type n) {
		erase(begin() + pos,
			  begin() + pos + std::min(n, length() - pos));
		return *this;
	}

	template<class Alloc>
	basic_string<Alloc>::iterator basic_string<Alloc>::erase(const_iterator p) {
		std::memmove(const_cast<value_type*>(p), p + 1, (size_t) (end() - p) * sizeof(value_type));
		set_length(length() - 1, category());
		return const_cast<value_type*>(p);
	}

	template<class Alloc>
	basic_string<Alloc>::iterator basic_string<Alloc>::erase(const_iterator first, const_iterator last) {
		if (first != last) {
			std::memmove(const_cast<value_type*>(first), last, (size_t) ((end() - last) + 1) * sizeof(value_type));
			const auto n = (size_type) (last - first);
			set_length(length() - n, category());
		}
		return const_cast<value_type*>(first);
	}

	template<class Alloc>
	basic_string<Alloc>::reverse_iterator basic_string<Alloc>::erase(reverse_iterator position) {
		return reverse_iterator(erase((++position).base()));
	}

	template<class Alloc>
	basic_string<Alloc>::reverse_iterator basic_string<Alloc>::erase(reverse_iterator first, reverse_iterator last) {
		return reverse_iterator(erase((++last).base(), (++first).base()));
	}

	template<class Alloc>
	void basic_string<Alloc>::clear() noexcept {
		deallocate_self();
		construct_string_empty();
	}

#pragma endregion Erase

#pragma region Assign
	inline char* Assign(char* dest, size_t n, char c) {
		if (n) {
			return reinterpret_cast<char*>(std::memset(dest, c, (size_t) n));
		}
		return dest;
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(const basic_string& str) {
		if (this == &str) {
			return *this;
		}

		if (get_allocator() == str.get_allocator() && str.category() == Category::Long) {
			_members._long._cbptr = str._members._long._cbptr->acquire();
			_members._long._len = str._members._long._len;
			_members._long._cap = str._members._long._cap;
			return *this;
		} else {
			return assign(str.begin(), str.end());
		}
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(const basic_string& str, size_type pos, size_type n) {
		return assign(
				str.begin() + pos,
				str.begin() + pos + std::min(n, str.size() - pos));
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(basic_string&& str) {
		if (get_allocator() == str.get_allocator()) {
			std::swap(_members, str._members);
		} else {
			assign(begin(), end());
		}
		return *this;
	}

	template<class Alloc>
	template<typename T>
	basic_string<Alloc>& basic_string<Alloc>::assign(const T& t) {
		const basic_string_view str = t;
		return assign(str.data(), str.data() + str.size());
	}

	template<class Alloc>
	template<typename T>
	basic_string<Alloc>& basic_string<Alloc>::assign(const T& t, size_type pos, size_type n) {
		const basic_string_view str = t;
		return assign(
				str.begin() + pos,
				str.begin() + pos + std::min(n, str.size() - pos));
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(const value_type* s) {
		return assign(s, s + std::strlen(s));
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(const value_type* s, size_type n) {
		return assign(s, s + n);
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(size_type n, value_type c) {
		const auto len = length();
		if (n <= len) {
			Assign(begin(), n, c);
			erase(begin() + n, end());
		} else {
			Assign(begin(), len, c);
			append(n - len, c);
		}
		return *this;
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(std::initializer_list<value_type> ilist) {
		return assign(ilist.begin(), ilist.end());
	}

	/*template<class Alloc>
    template<class InputIterator>
    basic_string<Alloc>& basic_string<Alloc>::assign(InputIterator first, InputIterator last) {
        const auto n = std::distance(first, last);
        const auto len = length();
        if (n <= len) {
            if (n)
                std::memmove(begin(), first, (size_t)n * sizeof(value_type));
            erase(begin() + n, end());
        } else {
            std::memmove(begin(), first, (size_t)(len) * sizeof(value_type));
            append(first + len, last);
        }
        return *this;
    }*/

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(const value_type* first, const value_type* last) {
		const auto n = (size_t) (last - first);
		const auto len = length();
		if (n <= len) {
			if (n) {
				std::memmove(begin(), first, n * sizeof(value_type));
			}
			erase(begin() + n, end());
		} else {
			std::memmove(begin(), first, (size_t) (len) * sizeof(value_type));
			append(first + len, last);
		}
		return *this;
	}

#pragma endregion Assign

#pragma region Append
	inline char* Fill(char* dest, size_t n, const char c) {
		if (n) {
			std::memset(dest, (uint8_t) c, (size_t) n);
		}
		return dest + n;
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(const basic_string& str) {
		return append(str.begin(), str.end());
	}

	template<class Alloc>
	template<class T>
	basic_string<Alloc>& basic_string<Alloc>::append(const T& t) {
		const basic_string_view str = t;
		return append(str.begin(), str.end());
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(const basic_string& str, size_type pos, size_type n) {
		return append(str.begin() + pos,
					  str.begin() + pos + std::min(n, str.size() - pos));
	}

	template<class Alloc>
	template<class T>
	basic_string<Alloc>& basic_string<Alloc>::append(const T& t, size_type pos, size_type n) {
		const basic_string_view str = t;
		return append(str.begin() + pos,
					  str.begin() + pos + std::min(n, str.size() - pos));
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(const value_type* s, size_type n) {
		return append(s, s + n);
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(const value_type* s) {
		return append(s, s + std::strlen(s));
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(size_type n, value_type c) {
		if (n > 0) {
			const auto len = length();
			const auto cap = capacity();
			const auto newLen = len + n;
			const auto newCap = newLen + 1;

			if (newCap > cap)
				reserve(gen_capacity(newCap));

			pointer newEnd = Fill(end(), n, c);
			*newEnd = __terminator;
			set_length(newLen, category());
		}
		return *this;
	}

	template<typename T>
	inline T* Copy(const T* first, const T* last, T* dest) {
		std::memmove(dest, first, (size_t) (last - first) * sizeof(T));
		return dest + (last - first);
	}

	template<class Alloc>
	template<class InputIterator>
	basic_string<Alloc>& basic_string<Alloc>::append(InputIterator first, InputIterator last) {
		if (first != last) {
			const auto n = std::distance(first, last);
			const auto cap = capacity();
			const auto len = length();
			const auto newLen = len + n;

			if (newLen > cap) {
				const auto newCap = gen_capacity(newLen + 1);
				pointer newBegin = allocate(newCap);

				pointer newEnd = Copy(begin(), end(), newBegin);
				newEnd = Copy(first, last, newEnd);
				*newEnd = __terminator;

				deallocate_self();

				set_heap(newBegin, newLen, newCap);

			} else {
				pointer newEnd = Copy(first, last, end());
				*newEnd = __terminator;
				set_length(newLen, category());
			}
		}

		return *this;
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(std::initializer_list<value_type> ilist) {
		append(ilist.begin(), ilist.end());
	}
#pragma endregion Append

#pragma region Compare
	constexpr int Compare(const char* p1, const char* p2, size_t n) {
		return __builtin_memcmp(p1, p2, n);
	}

	template<class Alloc>
	int basic_string<Alloc>::compare(const basic_string& str) const noexcept {
		return compare(begin(), end(), str.begin(), str.end());
	}
	template<class Alloc>
	template<class T>
	int basic_string<Alloc>::compare(const T& t) const noexcept {
		const basic_string_view str = t;
		return compare(begin(), end(), str.begin(), str.end());
	}
	template<class Alloc>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const basic_string& str) const {
		return compare(
				begin() + pos1,
				begin() + pos1 + std::min(n1, size() - pos1),
				str.begin(), str.end());
	}
	template<class Alloc>
	template<class T>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const T& t) const {
		const basic_string_view str = t;
		return compare(
				begin() + pos1,
				begin() + pos1 + std::min(n1, size() - pos1),
				str.begin(), str.end());
	}
	template<class Alloc>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const basic_string& str, size_type pos2, size_type n2) const {
		return compare(begin() + pos1,
					   begin() + pos1 + std::min(n1, size() - pos1),
					   str.begin() + pos2,
					   str.begin() + pos2 + std::min(n2, str.size() - pos2));
	}

	template<class Alloc>
	template<class T>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const T& t, size_type pos2, size_type n2) const {
		const basic_string_view str = t;
		return compare(begin() + pos1,
					   begin() + pos1 + std::min(n1, size() - pos1),
					   str.begin() + pos2,
					   str.begin() + pos2 + std::min(n2, str.size() - pos2));
	}

	template<class Alloc>
	int basic_string<Alloc>::compare(const value_type* s) const noexcept {
		return compare(begin(), end(), s, s + std::strlen(s));
	}

	template<class Alloc>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const value_type* s) const {
		return compare(begin() + pos1,
					   begin() + pos1 + std::min(n1, size() - pos1),
					   s,
					   s + std::strlen(s));
	}

	template<class Alloc>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const value_type* s, size_type pos2, size_type n2) const {
		return compare(begin() + pos1,
					   begin() + pos1 + std::min(n1, size() - pos1),
					   s + pos2,
					   s + pos2 + n2);
	}

	template<class Alloc>
	int basic_string<Alloc>::compare(const value_type* begin1, const value_type* end1, const value_type* begin2, const value_type* end2) {
		const difference_type n1 = end1 - begin1;
		const difference_type n2 = end2 - begin2;
		const difference_type min = std::min(n1, n2);
		const int cmp = Compare(begin1, begin2, (size_t) min);
		return (cmp != 0 ? cmp : (n1 < n2 ? -1 : (n1 > n2 ? 1 : 0)));
	}
#pragma endregion Compare

#pragma region Insert
	/*template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::insert(size_type pos1, const basic_string &str) {
		insert(begin() + pos1, str.begin(), str.end());
		return *this;
	}

	template<class Alloc>
	template <class T>
	basic_string<Alloc> &basic_string<Alloc>::insert(size_type pos1, const T &t) {
		const basic_string_view str = t;
		insert(begin() + pos1, str.begin(), str.end());
		return *this;
	}
	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::insert(size_type pos1, const basic_string &str, size_type pos2, size_type n) {
		insert(begin() + pos1,
			   str.begin() + pos2,
			   str.begin() + pos2 + std::min(n, str.size() - pos2));
	}
	template<class Alloc>
	template <class T>
	basic_string<Alloc> &basic_string<Alloc>::insert(size_type pos1, const T &t, size_type pos2, size_type n) {
		const basic_string_view str = t;
		insert(begin() + pos1,
			   str.begin() + pos2,
			   str.begin() + pos2 + std::min(n, str.size() - pos2));
	}
	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::insert(size_type pos, const value_type *s, size_type n) {
		insert(begin() + pos, s, s + n);
		return *this;
	}
	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::insert(size_type pos, const value_type *s) {
		insert(begin() + pos, s, s + std::strlen(s));
		return *this;
	}
	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::insert(size_type pos, size_type n, value_type c) {
		insert(begin() + pos, n, c);
		return *this;
	}
	template<class Alloc>
	typename basic_string<Alloc>::iterator
	basic_string<Alloc>::insert(const_iterator p, value_type c) {
		if (p == end()) {
			push_back(c);
			return end() - 1;
		}
		// TODO:
		//return insert_internal(p, c);
	}
	template<class Alloc>
	typename basic_string<Alloc>::iterator
	basic_string<Alloc>::insert(const_iterator p, size_type n, value_type c) {
		// TODO:
	}

	template<class Alloc>
	template <class InputIterator>
	typename basic_string<Alloc>::iterator
	basic_string<Alloc>::insert(const_iterator p, InputIterator first, InputIterator last) {
		// TODO:
	}

	template<class Alloc>
	typename basic_string<Alloc>::iterator
	basic_string<Alloc>::insert(const_iterator p, std::initializer_list<value_type> ilist) {
		return insert(p, ilist.begin(), ilist.end());
	}*/
#pragma endregion Insert

#pragma region Operators
	template<class Alloc>
	basic_string<Alloc>::operator basic_string_view() const noexcept {
		return {data(), size()};
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(const basic_string& str) {
		return assign(str);
	}

	template<class Alloc>
	template<typename T>
	basic_string<Alloc>& basic_string<Alloc>::operator=(const T& t) {
		return assign(t);
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(basic_string&& str) noexcept(allocator_type::propagate_on_container_move_assignment::value || allocator_type::is_always_equal::value) {
		return assign(std::move(str));
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(const value_type* s) {
		return assign(s);
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(value_type c) {
		return assign(1, c);
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(std::initializer_list<value_type> ilist) {
		return assign(ilist);
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator+=(const basic_string& str) {
		return append(str);
	}
	template<class Alloc>
	template<class T>
	basic_string<Alloc>& basic_string<Alloc>::operator+=(const T& t) {
		return append(t);
	}
	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator+=(const value_type* s) {
		return append(s);
	}
	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator+=(value_type c) {
		return append(c);
	}
	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator+=(std::initializer_list<value_type> ilist) {
		return append(ilist);
	}

	template<class Alloc>
	bool basic_string<Alloc>::operator==(const value_type* str) const noexcept {
		auto s1 = size();
		auto s2 = std::strlen(str);
		return s1 == s2 && Compare(data(), str, s2) == 0;
	}

	template<class Alloc>
	bool basic_string<Alloc>::operator==(const basic_string& str) const noexcept {
		if (this == &str) {
			return true;
		}

		// TODO:
		/*if (str.category() == Category::Long && _members == str._members) {
			return true;
		}*/

		auto s1 = size();
		auto s2 = str.size();
		return s1 == s2 && Compare(data(), str.data(), s2) == 0;
	}

	template<class Alloc>
	template<typename T>
	bool basic_string<Alloc>::operator==(const T& t) const noexcept {
		const basic_string_view str = t;
		auto s1 = size();
		auto s2 = str.size();
		return s1 == s2 && Compare(data(), str.data(), s2) == 0;
	}

#pragma endregion Operators

	template<class Alloc>
	void basic_string<Alloc>::reserve(size_type n) {
		// C++20 says if the passed in capacity is less than the current capacity we do not shrink
		// If new_cap is less than or equal to the current capacity(), there is no effect.
		n = std::max(n, size());
		if (n <= capacity())
			return;

		const auto len = length();
		const auto cap = gen_capacity(n);

		auto ptr = allocate(cap);
		std::memcpy(ptr, data(), (len + 1) * sizeof(value_type));

		deallocate_self();

		set_heap(ptr, len, cap);
	}

	template<typename Alloc>
	void basic_string<Alloc>::resize(size_type n, value_type c) {
		const auto len = length();
		if (n < len) {
			erase(begin() + n, end());
		} else if (n > len) {
			append(n - len, c);
		}
	}

	template<typename Alloc>
	void basic_string<Alloc>::resize(size_type n) {
		const auto len = length();
		if (n < len) {
			erase(begin() + n, end());
		} else if (n > len) {
			append(n - len, value_type());
		}
	}

	template<class Alloc>
	basic_string<Alloc> basic_string<Alloc>::substr(basic_string::size_type pos, basic_string::size_type n) const {
		return basic_string(
				begin() + pos,
				begin() + pos + std::min(n, size() - pos), get_allocator());
	}

	template<class Alloc>
	basic_string<Alloc>::size_type basic_string<Alloc>::copy(basic_string::value_type* s, basic_string::size_type n,
															 basic_string::size_type pos) const {
		const auto length = std::min(n, size() - pos);
		Copy(begin() + pos, begin() + pos + length, s);
		return length;
	}

	template<class Alloc>
	void basic_string<Alloc>::swap(basic_string& str) noexcept(std::allocator_traits<allocator_type>::propagate_on_container_swap::value || std::allocator_traits<allocator_type>::is_always_equal::value) {
		if (get_allocator() == str.get_allocator()) {
			std::swap(_members, str._members);
		} else {
			const basic_string temp(*this);
			*this = str;
			str = std::move(temp);
		}
	}

#pragma region Constructs
	template<class Alloc>
	void basic_string<Alloc>::construct_internal(const value_type* s) {
		const auto len = std::strlen(s);
		construct_internal(s, len);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_internal(const value_type* s, size_type len) {
		const auto cap = gen_capacity(len + 1);
		const auto cat = get_category(cap);
		switch (cat) {
			case Category::Short:
				construct_string_short(s, len, cap);
				break;
			case Category::Mid:
				construct_string_mid(s, len, cap);
				break;
			case Category::Long:
				construct_string_long(s, len, cap);
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_internal(const basic_string& str) {
		switch (str.category()) {
			case Category::Short:
				construct_string_short(str.c_str(), str.size(), __max_short_size);
				break;
			case Category::Mid:
				construct_string_mid(str.c_str(), str.size(), str.capacity());
				break;
			case Category::Long:
				if (get_allocator() == str.get_allocator()) {
					_members._long._cbptr = str._members._long._cbptr->acquire();
					_members._long._len = str._members._long._len;
					_members._long._cap = str._members._long._cap;
				} else {
					construct_string_long(str.c_str(), str.size(), str.capacity());
				}
				break;
		}
	}

	template<class Alloc>
	template<class InputIterator>
	void basic_string<Alloc>::construct_internal(InputIterator first, InputIterator last) {
		const auto len = std::distance(first, last);
		const auto cap = gen_capacity(len + 1);
		const auto cat = get_category(cap);

		static_assert(std::random_access_iterator<InputIterator>, "Only random iterators are supported!");

		pointer end;
		switch (cat) {
			case Category::Short:
				end = Copy(first, last, _members._short._data);
				*end = __terminator;
				set_length(len, Category::Short);
				break;
			case Category::Mid:
				_members._mid._ptr = allocate(cap);
				end = Copy(first, last, _members._mid._ptr);
				*end = __terminator;
				set_length(len, Category::Mid);
				set_capacity(cap);
				break;
			case Category::Long:
				auto ptr = allocate(cap);
				end = Copy(first, last, ptr);
				*end = __terminator;
				set_length(len, Category::Long);
				set_capacity(cap);
				_members._long._cbptr = new ControlBlock(ptr, len + 1, cap, get_allocator());
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_internal(size_type n, value_type c) {
		const auto cap = gen_capacity(n + 1);
		const auto cat = get_category(cap);
		pointer end;
		switch (cat) {
			case Category::Short:
				assert(n < __max_short_size);
				end = Fill(_members._short._data, n, c);
				*end = __terminator;
				set_length(n, Category::Short);
				break;
			case Category::Mid:
				_members._mid._ptr = allocate(cap);
				end = Fill(_members._mid._ptr, n, c);
				*end = __terminator;
				set_length(n, Category::Mid);
				set_capacity(cap);
				break;
			case Category::Long:
				auto ptr = allocate(cap);
				end = Fill(ptr, n, c);
				*end = __terminator;
				set_length(n, Category::Long);
				set_capacity(cap);
				_members._long._cbptr = new ControlBlock(ptr, n + 1, cap, get_allocator());
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_string_empty() {
		_members._short._data[0] = __terminator;
		set_length(0, Category::Short);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_string_short(const value_type* s, size_type len, size_type /*cap*/) {
		assert(len < __max_short_size);

		pointer end = Copy(s, s + len, _members._short._data);
		*end = __terminator;

		set_length(len, Category::Short);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_string_mid(const value_type* s, size_type len, size_type cap) {
		assert(len < cap);
		_members._mid._ptr = allocate(cap);

		pointer end = Copy(s, s + len, _members._mid._ptr);
		*end = __terminator;

		set_length(len, Category::Mid);
		set_capacity(cap);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_string_long(const value_type* s, size_type len, size_type cap) {
		assert(len < cap);
		auto ptr = allocate(cap);

		pointer end = Copy(s, s + len, ptr);
		*end = __terminator;

		set_length(len, Category::Long);
		set_capacity(cap);
		_members._long._cbptr = new ControlBlock(ptr, len + 1, cap, get_allocator());
	}

#pragma endregion Constructs

#pragma region Helpers
	template<class Alloc>
	void basic_string<Alloc>::set_heap(pointer ptr, size_type len, size_type cap) {
		const auto cat = get_category(cap);
		switch (cat) {
			case Category::Short:
				assert(len < __max_short_size);
				std::memcpy(_members._short._data, ptr, (len + 1) * sizeof(value_type));
				set_length(len, Category::Short);
				deallocate(ptr, cap);
				break;
			case Category::Mid:
				_members._mid._ptr = ptr;
				set_length(len, Category::Mid);
				set_capacity(cap);
				break;
			case Category::Long:
				_members._long._cbptr = new ControlBlock(ptr, len + 1, cap, get_allocator());
				set_length(len, Category::Long);
				set_capacity(cap);
				break;
		}
	}

	template<class Alloc>
	uint8_t& basic_string<Alloc>::msbyte() noexcept {
		return *reinterpret_cast<uint8_t*>(this + 23);
	}

	/*template<class Alloc>
    void basic_string<Alloc>::print_mem() const {
        uint8_t mem[24];
        std::memcpy(mem, this, 24);
        std::cerr << "Mem: ";
        for (int i = 0; i < 24; i++)
            std::cerr << std::hex << uint32_t{mem[i]} << " ";
        std::cerr << std::endl;
    }*/

	template<class Alloc>
	uint8_t basic_string<Alloc>::cmsbyte() const noexcept {
		return reinterpret_cast<const uint8_t*>(this)[23];
	}

	template<class Alloc>
	typename basic_string<Alloc>::value_type*
	basic_string<Alloc>::begin_ptr() noexcept {
		switch (category()) {
			case Category::Short:
				return _members._short._data;
			case Category::Mid:
				return _members._mid._ptr;
			case Category::Long:
				return _members._long._cbptr->get();
		}
		return nullptr;
	}

	template<class Alloc>
	typename basic_string<Alloc>::value_type*
	basic_string<Alloc>::end_ptr() noexcept {
		switch (category()) {
			case Category::Short:
				return _members._short._data + (_members._short._len ^ (1 << 7));
			case Category::Mid:
				return _members._mid._ptr + _members._mid._len;
			case Category::Long:
				return _members._long._cbptr->get() + _members._long._len;
		}
		return nullptr;
	}

	template<class Alloc>
	const typename basic_string<Alloc>::value_type*
	basic_string<Alloc>::begin_ptr() const noexcept {
		switch (category()) {
			case Category::Short:
				return _members._short._data;
			case Category::Mid:
				return _members._mid._ptr;
			case Category::Long:
				return _members._long._cbptr->get();
		}
		return nullptr;
	}

	template<class Alloc>
	const typename basic_string<Alloc>::value_type*
	basic_string<Alloc>::end_ptr() const noexcept {
		switch (category()) {
			case Category::Short:
				return _members._short._data + (_members._short._len ^ (1 << 7));
			case Category::Mid:
				return _members._mid._ptr + _members._mid._len;
			case Category::Long:
				return _members._long._cbptr->get() + _members._long._len;
		}
		return nullptr;
	}

	template<class Alloc>
	typename basic_string<Alloc>::pointer basic_string<Alloc>::allocate(size_type n) {
		return get_allocator().allocate(n * sizeof(value_type));
	}

	template<class Alloc>
	void basic_string<Alloc>::deallocate(pointer ptr, size_type n) {
		return get_allocator().deallocate(ptr, n * sizeof(value_type));
	}

	template<class Alloc>
	void basic_string<Alloc>::deallocate_self() {
		auto cat = category();
		switch (cat) {
			case Category::Short:
				break;
			case Category::Mid:
				deallocate(_members._mid._ptr, _members._mid._cap);
				break;
			case Category::Long:
				_members._long._cbptr->release();
				break;
		}
	}

	template<class Alloc>
	typename basic_string<Alloc>::size_type
	basic_string<Alloc>::remaining_capacity() const noexcept {
		return capacity() - length();
	}

	template<class Alloc>
	typename basic_string<Alloc>::size_type
	basic_string<Alloc>::gen_capacity(size_type len) noexcept {
		if (len <= __max_short_size)
			return len;
		size_type ret{1};
		while (ret < len)
			ret = ret << 1;
		return ret;
	}

	template<class Alloc>
	bool basic_string<Alloc>::msb() const noexcept {
		return (cmsbyte() & (static_cast<uint8_t>(1 << 7)));
	}

	template<class Alloc>
	void basic_string<Alloc>::set_length(size_type length, Category cat) {
		switch (cat) {
			case Category::Short:
				_members._short._len = static_cast<uint8_t>(static_cast<uint8_t>(length) | static_cast<uint8_t>(1 << 7));
				break;
			case Category::Mid:
				_members._mid._len = length;
				break;
			case Category::Long:
				_members._long._len = length;
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::set_capacity(size_type capacity) {
		const auto cat = get_category(capacity);
		switch (cat) {
			case Category::Short:
				assert(capacity == __max_short_size);
				break;
			case Category::Mid:
				_members._mid._cap = capacity;
				break;
			case Category::Long:
				_members._long._cap = capacity;
				break;
		}
	}

	template<class Alloc>
	typename basic_string<Alloc>::size_type
	basic_string<Alloc>::capacity() const noexcept {
		if (msb()) {
			return __max_short_size;
		} else {
			auto cap_byte_ptr = reinterpret_cast<const std::byte*>(this) + 16;
			return *reinterpret_cast<const size_type*>(cap_byte_ptr);
		}
	}

	template<class Alloc>
	typename basic_string<Alloc>::Category basic_string<Alloc>::category() const noexcept {
		if (msb()) {
			return Category::Short;
		} else {
			const auto cap = capacity();
			if (cap > __max_mid_size) {
				return Category::Long;
			} else {
				return Category::Mid;
			}
		}
	}

	template<class Alloc>
	typename basic_string<Alloc>::Category
	basic_string<Alloc>::get_category(size_type cap) noexcept {
		if (cap <= __max_short_size) {
			return Category::Short;
		} else if (cap <= __max_mid_size) {
			return Category::Mid;
		} else {
			return Category::Long;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::mutable_cb() {
		assert(category() == Category::Long);

		// do not need to do anything if you are holding the only reference
		if (_members._long._cbptr->count() == 1)
			return;

		auto prev_cb = _members._long._cbptr;
		_members._long._cbptr = new ControlBlock(*prev_cb);
		prev_cb->release();
	}

#pragma endregion Helpers

#pragma region Convertions
	inline int stoi(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtol(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<std::size_t>(cstr - ptr);

		return static_cast<int>(ret);
	}

	inline long stol(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtol(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<std::size_t>(cstr - ptr);

		return ret;
	}

	inline long long stoll(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoll(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<std::size_t>(cstr - ptr);

		return ret;
	}

	inline unsigned long stoul(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoul(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<std::size_t>(cstr - ptr);

		return ret;
	}

	inline unsigned long long stoull(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoull(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<std::size_t>(cstr - ptr);

		return ret;
	}

	inline float stof(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtof(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<std::size_t>(cstr - ptr);

		return ret;
	}

	inline double stod(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtod(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<std::size_t>(cstr - ptr);

		return ret;
	}

	inline long double stold(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtold(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<std::size_t>(cstr - ptr);

		return ret;
	}

	namespace detail {
		template<typename Type>
		constexpr std::size_t to_chars_len(Type value) {
			constexpr Type b1 = 10;
			constexpr Type b2 = 100;
			constexpr Type b3 = 1000;
			constexpr Type b4 = 10000;

			for (std::size_t i = 1;; i += 4, value /= b4) {
				if (value < b1)
					return i;
				if (value < b2)
					return i + 1;
				if (value < b3)
					return i + 2;
				if (value < b4)
					return i + 3;
			}
		}

		static constexpr char digits[201] =
				"0001020304050607080910111213141516171819"
				"2021222324252627282930313233343536373839"
				"4041424344454647484950515253545556575859"
				"6061626364656667686970717273747576777879"
				"8081828384858687888990919293949596979899";

		constexpr void to_chars(char* first, std::size_t len, auto val) {
			std::size_t pos = len - 1;
			while (val >= 100) {
				auto const num = (val % 100) * 2;
				val /= 100;
				first[pos] = digits[num + 1];
				first[pos - 1] = digits[num];
				pos -= 2;
			}
			if (val >= 10) {
				auto const num = val * 2;
				first[1] = digits[num + 1];
				first[0] = digits[num];
			} else {
				first[0] = '0' + (char)val;
			}
		}

		template<std::signed_integral Type, std::unsigned_integral UType = std::make_unsigned_t<Type>>
		inline string to_string(Type value) {
			const auto negative = value < 0;
			const UType uvalue = negative ? static_cast<UType>(~value) + static_cast<UType>(1) : value;
			const auto length = to_chars_len(uvalue);
			string str(length + negative, '-');
			to_chars(&str[negative], length, uvalue);
			return str;
		}

		template<std::unsigned_integral Type>
		inline string to_string(Type value) {
			string str(to_chars_len(value), '\0');
			to_chars(&str[0], str.length(), value);
			return str;
		}
	}// namespace detail
#pragma endregion Convertions

#pragma region Hash
	// hash support
	namespace detail {
		constexpr uint64_t MurmurHash2_64A(const void* key, uint64_t len, uint64_t seed) {
			const uint64_t m = 0xC6A4A7935BD1E995;
			const int r = 47;

			uint64_t h = seed ^ (len * m);

			const uint64_t* data = static_cast<const uint64_t*>(key);
			const uint64_t* end = data + (len / 8);

			while (data != end) {
				uint64_t k = 0;
				k = *(data++);

				k *= m;
				k ^= k >> r;
				k *= m;

				h ^= k;
				h *= m;
			}

			auto data2 = static_cast<const uint8_t*>(static_cast<const void*>(data));

			switch (len & 7) {
				case 7:
					h ^= static_cast<uint64_t>(data2[6]) << 48;
					[[fallthrough]];
				case 6:
					h ^= static_cast<uint64_t>(data2[5]) << 40;
					[[fallthrough]];
				case 5:
					h ^= static_cast<uint64_t>(data2[4]) << 32;
					[[fallthrough]];
				case 4:
					h ^= static_cast<uint64_t>(data2[3]) << 24;
					[[fallthrough]];
				case 3:
					h ^= static_cast<uint64_t>(data2[2]) << 16;
					[[fallthrough]];
				case 2:
					h ^= static_cast<uint64_t>(data2[1]) << 8;
					[[fallthrough]];
				case 1:
					h ^= static_cast<uint64_t>(data2[0]);
					h *= m;
			};

			h ^= h >> r;
			h *= m;
			h ^= h >> r;

			return h;
		}

		static const uint32_t SEED = 0xE17A1465;

		template<typename Alloc, typename String = basic_string<Alloc>>
		struct string_hash_base {
			[[nodiscard]] constexpr std::size_t operator()(const String& str) const noexcept {
				return MurmurHash2_64A(str.c_str(), str.length(), SEED);
			}
		};
	}// namespace detail
#pragma endregion Hash

#pragma region ControlBlock
	template<class Alloc>
	basic_string<Alloc>::ControlBlock::ControlBlock(pointer p, size_type len, size_type cap, const allocator_type& a)
		: _ptr{p}, _count{0}, _len{len}, _cap{cap}, _allocator{a} {
		acquire();
	}

	template<class Alloc>
	basic_string<Alloc>::ControlBlock::ControlBlock(const ControlBlock& other)
		: _ptr{nullptr}, _count{1}, _len{other._len}, _cap{other._cap}, _allocator{other._allocator} {
		_ptr = _allocator.allocate(_cap * sizeof(value_type));
		std::memcpy(_ptr, other._ptr, (_len + 1) * sizeof(value_type));
	}

	template<class Alloc>
	basic_string<Alloc>::ControlBlock::~ControlBlock() {
		assert(_count == 0);
		assert(_ptr);
		_allocator.deallocate(_ptr, _cap * sizeof(value_type));
	}

	template<class Alloc>
	typename basic_string<Alloc>::ControlBlock*
	basic_string<Alloc>::ControlBlock::acquire() noexcept {
		_count++;
		return this;
	}

	template<class Alloc>
	typename basic_string<Alloc>::pointer
	basic_string<Alloc>::ControlBlock::get() const noexcept {
		return _ptr;
	}

	template<class Alloc>
	typename basic_string<Alloc>::size_type
	basic_string<Alloc>::ControlBlock::count() const noexcept {
		return _count;
	}

	template<class Alloc>
	void basic_string<Alloc>::ControlBlock::release() {
		_count--;
		if (_count == 0) {
			delete this;
		}
	}
#pragma endregion ControlBlock
	
#pragma region GlobalOperators
	template <typename Alloc>
	inline bool operator==(const typename basic_string<Alloc>::reverse_iterator& r1, const typename basic_string<Alloc>::reverse_iterator& r2) {
		return r1.base() == r2.base();
	}

	template <typename Alloc>
	inline bool operator!=(const typename basic_string<Alloc>::reverse_iterator& r1, const typename basic_string<Alloc>::reverse_iterator& r2) {
		return r1.base() != r2.base();
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(const basic_string<Alloc>& a, const basic_string<Alloc>& b) {
		typedef detail::uninitialized_size_tag uninitialized;
		uninitialized tag;
		basic_string<Alloc> result(tag, a.size() + b.size(), const_cast<basic_string<Alloc>&>(a).get_allocator());
		result.append(a);
		result.append(b);
		return result;
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(const typename basic_string<Alloc>::value_type* s, const basic_string<Alloc>& b) {
		typedef detail::uninitialized_size_tag uninitialized;
		uninitialized tag;
		const auto n = (typename basic_string<Alloc>::size_type) std::strlen(s);
		basic_string<Alloc> result(tag, n + b.size(), const_cast<basic_string<Alloc>&>(b).get_allocator());
		result.append(s, s + n);
		result.append(b);
		return result;
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(typename basic_string<Alloc>::value_type c, const basic_string<Alloc>& b) {
		typedef detail::uninitialized_size_tag uninitialized;
		uninitialized tag;
		basic_string<Alloc> result(tag, 1 + b.size(), const_cast<basic_string<Alloc>&>(b).get_allocator());
		result.push_back(c);
		result.append(b);
		return result;
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(const basic_string<Alloc>& a, const typename basic_string<Alloc>::value_type* s) {
		typedef detail::uninitialized_size_tag uninitialized;
		uninitialized tag;
		const auto n = (typename basic_string<Alloc>::size_type) std::strlen(s);
		basic_string<Alloc> result(tag, a.size() + n, const_cast<basic_string<Alloc>&>(a).get_allocator());
		result.append(a);
		result.append(s, s + n);
		return result;
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(const basic_string<Alloc>& a, typename basic_string<Alloc>::value_type c) {
		typedef detail::uninitialized_size_tag uninitialized;
		uninitialized tag;
		basic_string<Alloc> result(tag, a.size() + 1, const_cast<basic_string<Alloc>&>(a).get_allocator());
		result.append(a);
		result.push_back(c);
		return result;
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(basic_string<Alloc>&& a, basic_string<Alloc>&& b) {
		a.append(b);
		return std::move(a);
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(basic_string<Alloc>&& a, const basic_string<Alloc>& b) {
		a.append(b);
		return std::move(a);
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(const typename basic_string<Alloc>::value_type* s, basic_string<Alloc>&& b) {
		b.insert(0, s);
		return std::move(b);
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(basic_string<Alloc>&& a, const typename basic_string<Alloc>::value_type* s) {
		a.append(s);
		return std::move(a);
	}

	template<typename Alloc>
	basic_string<Alloc> operator+(basic_string<Alloc>&& a, typename basic_string<Alloc>::value_type c) {
		a.push_back(c);
		return std::move(a);
	}

	template<typename Alloc>
	inline bool operator==(const basic_string<Alloc>& a, const basic_string<Alloc>& b) {
		return ((a.size() == b.size()) && (Compare(a.data(), b.data(), (size_t)a.size()) == 0));
	}

	template<typename Alloc>
	inline bool operator==(const typename basic_string<Alloc>::value_type* s, const basic_string<Alloc>& b) {
		typedef typename basic_string<Alloc>::size_type string_size_type;
		const auto n = (string_size_type) std::strlen(s);
		return ((n == b.size()) && (Compare(s, b.data(), (size_t) n) == 0));
	}

	template<typename Alloc>
	inline bool operator==(const basic_string<Alloc>& a, const typename basic_string<Alloc>::value_type* s) {
		typedef typename basic_string<Alloc>::size_type string_size_type;
		const auto n = (string_size_type) std::strlen(s);
		return ((a.size() == n) && (Compare(a.data(), s, (size_t) n) == 0));
	}

	template<typename Alloc>
	inline auto operator<=>(const basic_string<Alloc>& a, const basic_string<Alloc>& b) {
		return basic_string<Alloc>::compare(a.begin(), a.end(), b.begin(), b.end()) <=> 0;
	}

	template<typename Alloc>
	inline auto operator<=>(const basic_string<Alloc>& a, const typename basic_string<Alloc>::value_type* s) {
		typedef typename basic_string<Alloc>::size_type string_size_type;
		const auto n = (string_size_type) std::strlen(s);
		return basic_string<Alloc>::compare(a.begin(), a.end(), s, s + n) <=> 0;
	}

	template<typename Alloc>
	inline auto operator<=>(const basic_string<Alloc>& a, const typename basic_string<Alloc>::basic_string_view v) {
		typedef typename basic_string<Alloc>::basic_string_view view_type;
		return static_cast<view_type>(a) <=> v;
	}

	template<typename Alloc>
	inline void swap(basic_string<Alloc>& a, basic_string<Alloc>& b) {
		a.swap(b);
	}

#pragma endregion GlobalOperators
}// namespace plg

// format support
template<typename Alloc>
struct std::formatter<plg::basic_string<Alloc>> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const plg::basic_string<Alloc>& str, std::format_context& ctx) const {
		return std::format_to(ctx.out(), "{}", str.c_str());
	}
};

// hash support
template<typename Alloc>
struct std::hash<plg::basic_string<Alloc>> : plg::detail::string_hash_base<Alloc> {};

// TODO: implement the following API

/* size_type max_size() const noexcept; */

/**  I don't think replace should live inside the class. They should be non-member-functions that provide this functionality generically.*/

/* basic_string &replace(size_type pos1, size_type n1, */
/*						const basic_string &str); */
/* template <class T> */
/* basic_string &replace(size_type pos1, size_type n1, const T &t); // C++17
 */
/* basic_string &replace(size_type pos1, size_type n1, const basic_string
 * &str, */
/*						size_type pos2, size_type n2 = npos); // C++14 */
/* template <class T> */
/* basic_string &replace(size_type pos1, size_type n1, const T &t, */
/*						size_type pos2, size_type n); // C++17 */
/* basic_string &replace(size_type pos, size_type n1, const value_type *s, */
/*						size_type n2); */
/* basic_string &replace(size_type pos, size_type n1, const value_type *s); */
/* basic_string &replace(size_type pos, size_type n1, size_type n2, value_type c);
 */
/* basic_string &replace(const_iterator i1, const_iterator i2, */
/*						const basic_string &str); */
/* template <class T> */
/* basic_string &replace(const_iterator i1, const_iterator i2, */
/*						const T &t); // C++17 */
/* basic_string &replace(const_iterator i1, const_iterator i2, const value_type *s,
 */
/*						size_type n); */
/* basic_string &replace(const_iterator i1, const_iterator i2, const value_type
 * *s); */
/* basic_string &replace(const_iterator i1, const_iterator i2, size_type n,
 */
/*						value_type c); */
/* template <class InputIterator> */
/* basic_string &replace(const_iterator i1, const_iterator i2, InputIterator
 * j1, */
/*						InputIterator j2); */
/* basic_string &replace(const_iterator i1, const_iterator i2, */
/*						std::initializer_list<value_type>); */

/* size_type find(const basic_string &str, size_type pos = 0) const noexcept;
 */
/* template <class T> */
/* size_type find(const T &t, size_type pos = 0) const; // C++17 */
/* size_type find(const value_type *s, size_type pos, size_type n) const noexcept;
 */
/* size_type find(const value_type *s, size_type pos = 0) const noexcept; */
/* size_type find(value_type c, size_type pos = 0) const noexcept; */

/* size_type rfind(const basic_string &str, */
/*				 size_type pos = npos) const noexcept; */
/* template <class T> */
/* size_type rfind(const T &t, size_type pos = npos) const; // C++17 */
/* size_type rfind(const value_type *s, size_type pos, size_type n) const noexcept;
 */
/* size_type rfind(const value_type *s, size_type pos = npos) const noexcept; */
/* size_type rfind(value_type c, size_type pos = npos) const noexcept; */

/* size_type find_first_of(const basic_string &str, */
/*						 size_type pos = 0) const noexcept; */
/* template <class T> */
/* size_type find_first_of(const T &t, size_type pos = 0) const; // C++17 */
/* size_type find_first_of(const value_type *s, size_type pos, */
/*						 size_type n) const noexcept; */
/* size_type find_first_of(const value_type *s, size_type pos = 0) const noexcept;
 */
/* size_type find_first_of(value_type c, size_type pos = 0) const noexcept; */

/* size_type find_last_of(const basic_string &str, */
/*						size_type pos = npos) const noexcept; */
/* template <class T> */
/* size_type find_last_of(const T &t, */
/*						size_type pos = npos) const noexcept; // C++17 */
/* size_type find_last_of(const value_type *s, size_type pos, */
/*						size_type n) const noexcept; */
/* size_type find_last_of(const value_type *s, size_type pos = npos) const noexcept;
 */
/* size_type find_last_of(value_type c, size_type pos = npos) const noexcept; */

/* size_type find_first_not_of(const basic_string &str, */
/*							 size_type pos = 0) const noexcept; */
/* template <class T> */
/* size_type find_first_not_of(const T &t, size_type pos = 0) const; // C++17
 */
/* size_type find_first_not_of(const value_type *s, size_type pos, */
/*							 size_type n) const noexcept; */
/* size_type find_first_not_of(const value_type *s, size_type pos = 0) const
 * noexcept; */
/* size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept; */

/* size_type find_last_not_of(const basic_string &str, */
/*							size_type pos = npos) const noexcept; */
/* template <class T> */
/* size_type find_last_not_of(const T &t, size_type pos = npos) const; //
 * C++17 */
/* size_type find_last_not_of(const value_type *s, size_type pos, */
/*							size_type n) const noexcept; */
/* size_type find_last_not_of(const value_type *s, */
/*							size_type pos = npos) const noexcept; */
/* size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept; */

/* bool starts_with(std::basic_string_view<value_type> sv) const noexcept; // C++2a
 */
/* bool starts_with(value_type c) const noexcept;						  // C++2a
 */
/* bool starts_with(const value_type *s) const;							// C++2a
 */
/* bool ends_with(std::basic_string_view<value_type> sv) const noexcept;   // C++2a
 */
/* bool ends_with(value_type c) const noexcept;							// C++2a
 */
/* bool ends_with(const value_type *s) const;							  // C++2a
 */

/* bool __invariants() const; */