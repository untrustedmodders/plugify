#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <cassert>
#include <format>

// Adapted from https://github.com/klementtan/string_cpp/

namespace plg {
	// Memory layout for the different type of strings
	// clang-format off
		// byte     [00] [01] [02] [03] [04] [05] [06] [07] [08] [09] [10] [11] [12] [13] [14] [15] [16] [17] [18] [19] [20] [21] [22] [23]
		// small:   [value0                                                                                                   value22] [l ]
		// medium:  [pointer                              ] [len                                  ] [cap                                  ]
		// large:   [cb_pointer                           ] [len                                  ] [cap                                  ]
	// clang-format on

	template<class Alloc>
	class basic_string {
	public:
		typedef Alloc allocator_type;
		typedef std::allocator_traits<allocator_type> __alloc_traits;
		typedef typename __alloc_traits::size_type size_type;
		typedef ptrdiff_t difference_type;
		typedef char value_type;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef typename __alloc_traits::pointer pointer;
		typedef typename __alloc_traits::const_pointer const_pointer;
		typedef pointer iterator;
		typedef const_pointer const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		static const size_type npos = static_cast<size_type>(-1);

		basic_string() noexcept(std::is_nothrow_default_constructible<allocator_type>::value);
		explicit basic_string(const allocator_type& a);
		basic_string(const basic_string& str);
		basic_string(const value_type* s, const allocator_type& a = allocator_type());
		basic_string(basic_string &&str) noexcept(std::is_nothrow_move_constructible<allocator_type>::value);
		//basic_string(const basic_string &str, size_type pos,const allocator_type &a = allocator_type());
		//basic_string(const basic_string &str, size_type pos, size_type n, const allocator_type &a = allocator_type());
		template <class T>
		basic_string(const T &t, size_type pos, size_type n, const allocator_type &a = allocator_type());
		template <class T>
		basic_string(const T &t, const allocator_type &a = allocator_type());
		basic_string(const value_type *s, size_type n, const allocator_type &a = allocator_type());
		basic_string(size_type n, value_type c, const allocator_type &a = allocator_type());
		template <class InputIterator>
		basic_string(InputIterator first, InputIterator last, const allocator_type &a = allocator_type());
		basic_string(std::initializer_list<value_type>, const allocator_type & = allocator_type());
		basic_string(const basic_string &, const allocator_type &);
		//basic_string(basic_string &&, const allocator_type &);
		~basic_string();

		// Allocator
		const allocator_type& get_allocator() const noexcept;
		allocator_type& get_allocator() noexcept;
		void set_allocator(const allocator_type& allocator);

		//operator std::string_view() const noexcept;

		bool empty() const noexcept;
		size_type size() const noexcept;
		size_type length() const noexcept;
		size_type capacity() const noexcept;
		const_reference operator[](size_type pos) const;
		reference operator[](size_type pos);
		value_type *data() noexcept; // C++17
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

		/** Most std:: namespace containers never shrink. I want to point out that this may be an anti-feature. **/
		/* void shrink_to_fit(); */
		void reserve(size_type n = 0);

		basic_string &erase(size_type pos = 0, size_type n = npos);
		iterator erase(const_iterator position);
		iterator erase(const_iterator first, const_iterator last);
		reverse_iterator erase(reverse_iterator position);
		reverse_iterator erase(reverse_iterator first, reverse_iterator last);
		void clear() noexcept;

		basic_string &assign(const basic_string &str);
		template<typename T>
		basic_string &assign(const T &t); // C++17 */
		basic_string &assign(basic_string &&str);
		basic_string &assign(const basic_string &str, size_type pos, size_type n = npos); // C++14 */
		template<typename T>
		basic_string &assign(const T &t, size_type pos, size_type n = npos); // C++17 */
		basic_string &assign(const value_type *s, size_type n);
		basic_string &assign(const value_type *s);
		basic_string &assign(size_type n, value_type c);
		//template<class InputIterator>
		//basic_string &assign(InputIterator first, InputIterator last); TODO: Fix (not works as bellow)
		basic_string &assign(const value_type* first, const value_type* last);
		basic_string &assign(std::initializer_list<value_type>);

		basic_string &operator=(const basic_string &str);
		template <class T> basic_string &operator=(const T &t); // C++17 */
		basic_string &operator=(basic_string &&str) noexcept(allocator_type::propagate_on_container_move_assignment::value || allocator_type::is_always_equal::value); // C++17 */
		basic_string &operator=(const value_type *s);
		basic_string &operator=(value_type c);
		basic_string &operator=(std::initializer_list<value_type>);

		bool operator==(const value_type* str) const noexcept;
		bool operator==(const basic_string& str) const noexcept;
		template<typename T>
		bool operator==(const T &t) const noexcept;

		basic_string &operator+=(const basic_string &str);
		template <class T> basic_string &operator+=(const T &t); // C++17
		basic_string &operator+=(const value_type *s);
		basic_string &operator+=(value_type c);
		basic_string &operator+=(std::initializer_list<value_type>);

		basic_string &append(const basic_string &str);
		template <class T> basic_string &append(const T &t); // C++17
		basic_string &append(const basic_string &str, size_type pos, size_type n = npos); // C++14
		template <class T>
		basic_string &append(const T &t, size_type pos, size_type n = npos); // C++17
		basic_string &append(const value_type *s, size_type n);
		basic_string &append(const value_type *s);
		basic_string &append(size_type n, value_type c);
		template <class InputIterator>
		basic_string &append(InputIterator first, InputIterator last);
		basic_string &append(std::initializer_list<value_type>);

		int compare(const basic_string &str) const noexcept;
		template <class T>
		int compare(const T &t) const noexcept; // C++17
		int compare(size_type pos1, size_type n1, const basic_string &str) const;
		template <class T>
		int compare(size_type pos1, size_type n1, const T &t) const; // C++17
		int compare(size_type pos1, size_type n1, const basic_string &str, size_type pos2, size_type n2 = npos) const; // C++14
		template <class T>
		int compare(size_type pos1, size_type n1, const T &t, size_type pos2, size_type n2 = npos) const; // C++17
		int compare(const value_type *s) const noexcept;
		int compare(size_type pos1, size_type n1, const value_type *s) const;
		int compare(size_type pos1, size_type n1, const value_type *s, size_type pos2, size_type n2 = npos) const;
		static int compare(const value_type* begin1, const value_type* end1, const value_type* begin2, const value_type* end2);

	private:
		static constexpr size_type __string_max_size = sizeof(size_type) * 2 + sizeof(pointer);
		static constexpr size_type __max_short_size = 23;
		static constexpr size_type __max_mid_size = 255;

		enum class Category {
			Short = 0,
			Mid,
			Long
		};

#if _MSC_VER
#define nounique msvc::no_unique_address
#else
#define nounique no_unique_address
#endif

		class ControlBlock {
			pointer _ptr;
			std::atomic<size_type> _count;
			size_type _len;
			size_type _cap;
			[[nounique]] allocator_type _allocator;

		public:
			ControlBlock(pointer p, size_type len, size_type cap, const allocator_type& a = allocator_type());

			ControlBlock(const ControlBlock& other);
			ControlBlock& operator=(const ControlBlock&) = delete;
			ControlBlock(ControlBlock&&) = delete;
			ControlBlock& operator=(ControlBlock&& p) = delete;
			~ControlBlock();

			ControlBlock* acquire();
			void release();
			pointer get() const;
			size_type count() const;
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
				Short _short{};
				Mid _mid;
				Long _long;
			};
			[[nounique]] allocator_type _allocator{};
		} _members;

		static_assert(sizeof(Members) <= __string_max_size,
					  "Layout struct is larger than max size");

		Category category() const;
		static Category get_category(size_type cap);

		[[nodiscard]] bool msb() const;

		uint8_t& msbyte();
		uint8_t cmsbyte() const;

		value_type* begin_ptr();
		const value_type* begin_ptr() const;
		value_type* end_ptr();
		const value_type* end_ptr() const;

		void deallocate_self();

		void set_capacity(size_type cap);
		static size_type gen_capacity(size_type len);
		void set_length(size_type len, Category cat);
		//void print_mem() const;

		void construct(const value_type *s);
		void construct(const value_type *s, size_type len);
		void construct(const basic_string &str);
		template<class InputIterator>
		void construct(InputIterator first, InputIterator last);
		void construct(size_type n, value_type c);

		void construct_string_empty();
		void construct_string_short(const value_type* s, size_type len, size_type cap);
		void construct_string_mid(const value_type* s, size_type len, size_type cap);
		void construct_string_long(const value_type* s, size_type len, size_type cap);

		void mutable_cb();
	};

	template<class Alloc>
	void basic_string<Alloc>::set_allocator(const allocator_type &allocator) {
		_members._allocator = allocator;
	}

	template<class Alloc>
	typename basic_string<Alloc>::allocator_type &
	basic_string<Alloc>::get_allocator() noexcept {
		return _members._allocator;
	}

	template<class Alloc>
	const typename basic_string<Alloc>::allocator_type &
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
	basic_string<Alloc>::basic_string(const value_type* s, const allocator_type& a) : _members{ ._allocator = a } {
		construct(s);
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string() noexcept(std::is_nothrow_default_constructible<allocator_type>::value) {
		construct_string_empty();
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string(const allocator_type& a) : _members{ ._allocator = a } {
		construct_string_empty();
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string(const basic_string& str) : basic_string(str, allocator_type()) {
	}

	template <typename Alloc>
	basic_string<Alloc>::basic_string(basic_string&& str) noexcept(std::is_nothrow_move_constructible<allocator_type>::value) : _members{str._members} {
		str._members = {};
	}

	/*template <typename Alloc>
	basic_string<Alloc>::basic_string(const basic_string& str, size_type pos, const allocator_type& a) : _members{ ._allocator = a } {
		// TODO:
	}

	template <typename Alloc>
	basic_string<Alloc>::basic_string(const basic_string& str, size_type pos, size_type n, const allocator_type& a) : _members{ ._allocator = a } {
	   // TODO:
	}*/

	template <typename Alloc>
	template <class T>
	basic_string<Alloc>::basic_string(const T &t, size_type pos, size_type n, const allocator_type &a) : _members{ ._allocator = a } {
		if (pos > t.size()) {
			throw std::out_of_range("basic_string::basic_string -- out of range");
		}

		const auto len_to_assign = (n == npos || pos + n > t.size()) ? t.size() - pos : n;
		construct(t.data() + pos, len_to_assign);
	}

	template <typename Alloc>
	template <class T>
	basic_string<Alloc>::basic_string(const T& t, const allocator_type& a) : _members{ ._allocator = a } {
		construct(t.data(), t.size());
	}

	template <typename Alloc>
	basic_string<Alloc>::basic_string(const value_type* s, size_type n, const allocator_type& a) : _members{ ._allocator = a } {
		construct(s, n);
	}

	template <typename Alloc>
	basic_string<Alloc>::basic_string(size_type n, value_type c, const allocator_type& a) : _members{ ._allocator = a } {
		construct(n, c);
	}

	template <typename Alloc>
	template <class InputIterator>
	basic_string<Alloc>::basic_string(InputIterator first, InputIterator last, const allocator_type& a) : _members{ ._allocator = a } {
		construct(first, last);
	}

	template <typename Alloc>
	basic_string<Alloc>::basic_string(std::initializer_list<value_type> ilist, const allocator_type& a) : _members{ ._allocator = a } {
		construct(ilist.begin(), ilist.size());
	}

	template <typename Alloc>
	basic_string<Alloc>::basic_string(const basic_string& str, const allocator_type& a) : _members{ ._allocator = a } {
		construct(str);
	}

	/*template <typename Alloc>
	basic_string<Alloc>::basic_string(basic_string&& str, const allocator_type& a) : _members{ ._allocator = a } {
		// TODO:
	}*/
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
	basic_string<Alloc>::size() const noexcept {
		return length();
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
	}

#pragma endregion Basic

#pragma region Iterators

	template <typename Alloc>
	typename basic_string<Alloc>::iterator
	basic_string<Alloc>::begin() noexcept {
		return begin_ptr();
	}

	template <typename Alloc>
	typename basic_string<Alloc>::iterator
	basic_string<Alloc>::end() noexcept {
		return end_ptr();
	}

	template <typename Alloc>
	typename basic_string<Alloc>::const_iterator
	basic_string<Alloc>::begin() const noexcept {
		return begin_ptr();
	}

	template <typename Alloc>
	typename basic_string<Alloc>::const_iterator
	basic_string<Alloc>::cbegin() const noexcept {
		return begin_ptr();
	}

	template <typename Alloc>
	typename basic_string<Alloc>::const_iterator
	basic_string<Alloc>::end() const noexcept {
		return end_ptr();
	}

	template <typename Alloc>
	typename basic_string<Alloc>::const_iterator
	basic_string<Alloc>::cend() const noexcept {
		return end_ptr();
	}

	template <typename Alloc>
	typename basic_string<Alloc>::reverse_iterator
	basic_string<Alloc>::rbegin() noexcept {
		return reverse_iterator(end_ptr());
	}

	template <typename Alloc>
	typename basic_string<Alloc>::reverse_iterator
	basic_string<Alloc>::rend() noexcept {
		return reverse_iterator(begin_ptr());
	}

	template <typename Alloc>
	typename basic_string<Alloc>::const_reverse_iterator
	basic_string<Alloc>::rbegin() const noexcept {
		return const_reverse_iterator(end_ptr());
	}

	template <typename Alloc>
	typename basic_string<Alloc>::const_reverse_iterator
	basic_string<Alloc>::crbegin() const noexcept {
		return const_reverse_iterator(end_ptr());
	}

	template <typename Alloc>
	typename basic_string<Alloc>::const_reverse_iterator
	basic_string<Alloc>::rend() const noexcept {
		return const_reverse_iterator(begin_ptr());
	}

	template <typename Alloc>
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
		append((size_t)1, c);
	}

	template<class Alloc>
	void basic_string<Alloc>::pop_back() {
		end()[-1] = '\0';
		set_length(length() - 1, category());
	}

#pragma region Erase
	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::erase(size_type pos, size_type n) {
		erase(begin() + pos,
			  begin() + pos + std::min(n, length() - pos));
		return *this;
	}

	template<class Alloc>
	basic_string<Alloc>::iterator basic_string<Alloc>::erase(const_iterator p) {
		std::memmove(const_cast<value_type*>(p), p + 1, (size_t)(end() - p) * sizeof(value_type));
		set_length(length() - 1, category());
		return const_cast<value_type*>(p);
	}

	template<class Alloc>
	basic_string<Alloc>::iterator basic_string<Alloc>::erase(const_iterator first, const_iterator last) {
		if (first != last) {
			std::memmove(const_cast<value_type*>(first), last, (size_t)((end() - last) + 1) * sizeof(value_type));
			const size_type n = (size_type)(last - first);
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
	inline char* Assign(char* dest, size_t n, char c)  {
		if (n) {
			return reinterpret_cast<char*>(std::memset(dest, c, (size_t)n));
		}
		return dest;
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(const basic_string& str) {
		return assign(str.begin(), str.end());
	}

	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(const basic_string &str, size_type pos, size_type n) {
		return assign(
				str.begin() + pos,
				str.begin() + pos + std::min(n, str.length() - pos));
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
	template<typename T> basic_string<Alloc>& basic_string<Alloc>::assign(const T &t) {
		return assign(t.data(), t.data() + t.size());
	}

	template<class Alloc>
	template<typename T>
	basic_string<Alloc>& basic_string<Alloc>::assign(const T& t, size_type pos, size_type n) {
		return assign(
				t.begin() + pos,
				t.begin() + pos + std::min(n, t.size() - pos));
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
		const auto n = (size_t)(last - first);
		const auto len = length();
		if (n <= len) {
			if (n) {
				std::memmove(begin(), first, n * sizeof(value_type));
			}
			erase(begin() + n, end());
		} else {
			std::memmove(begin(), first, (size_t)(len) * sizeof(value_type));
			append(first + len, last);
		}
		return *this;
	}

#pragma endregion Assign

#pragma region Append
	inline char* Fill(char* dest, size_t n, const char c) {
		if (n) {
			std::memset(dest, (uint8_t)c, (size_t)n);
		}
		return dest + n;
	}

	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::append(const basic_string &str) {
		return append(str.begin(), str.end());
	}

	template<class Alloc>
	template <class T>
	basic_string<Alloc> &basic_string<Alloc>::append(const T &t) {
		return append(t.begin(), t.end());
	}

	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::append(const basic_string &str, size_type pos, size_type n) {
		return append(str.begin() + pos,
					  str.begin() + pos + std::min(n, str.size() - pos));
	}

	template<class Alloc>
	template <class T>
	basic_string<Alloc> &basic_string<Alloc>::append(const T &t, size_type pos, size_type n) {
		return append(t.begin() + pos,
					  t.begin() + pos + std::min(n, t.size() - pos));
	}

	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::append(const value_type *s, size_type n) {
		return append(s, s + n);
	}

	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::append(const value_type *s) {
		return append(s, s + std::strlen(s));
	}

	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::append(size_type n, value_type c) {
		if (n > 0) {
			const auto len = length();
			const auto cap = capacity();
			const auto newCap = len + n;

			if (newCap > cap)
				reserve(gen_capacity(newCap + 1));

			pointer newEnd = Fill(end(), n, c);
			*newEnd = 0;
			set_length(newCap, category());
		}
		return *this;
	}

	template<typename T>
	inline T* UninitializedCopy(const T* first, const T* last, T* dest) {
		std::memmove(dest, first, (size_t)(last - first) * sizeof(T));
		return dest + (last - first);
	}

	template<class Alloc>
	template <class InputIterator>
	basic_string<Alloc> &basic_string<Alloc>::append(InputIterator first, InputIterator last) {
		if(first != last) {
			const auto n = std::distance(first, last);
			const auto len = length() + n;
			auto cap = capacity();

			if (len > cap) {
				cap = gen_capacity(len + 1);

				auto a = get_allocator();
				pointer newBegin = a.allocate(cap);

				pointer newEnd = UninitializedCopy(begin(), end(), newBegin);
				newEnd = UninitializedCopy(first, last, newEnd);
				*newEnd = 0;

				deallocate_self();

				const auto cat = get_category(cap);
				switch (cat) {
					case Category::Short: {
						assert(n < __max_short_size);
						std::memcpy(_members._short._data, newBegin, n * sizeof(value_type));
						set_length(len, Category::Short);
						a.deallocate(newBegin, n);
						break;
					}
					case Category::Mid:
						_members._mid._ptr = newBegin;
						set_length(len, Category::Mid);
						set_capacity(cap);
						break;
					case Category::Long:
						_members._long._cbptr = new ControlBlock(newBegin, n, cap, a);
						set_length(len, Category::Long);
						set_capacity(cap);
						break;
				}

			} else {
				pointer newEnd = UninitializedCopy(first, last, end());
				*newEnd = 0;
				set_length(len, category());
			}
		}

		return *this;
	}

	template<class Alloc>
	basic_string<Alloc> &basic_string<Alloc>::append(std::initializer_list<value_type> ilist) {
		append(ilist.begin(), ilist.end());
	}
#pragma endregion Append

#pragma region Compare
	// All main compilers offer a constexpr __builtin_memcmp as soon as C++17 was available.
	constexpr int Compare(const char* p1, const char* p2, size_t n) { return __builtin_memcmp(p1, p2, n); }

	template<class Alloc>
	int basic_string<Alloc>::compare(const basic_string &str) const noexcept {
		return compare(begin(), end(), str.begin(), str.end());
	}
	template<class Alloc>
	template <class T>
	int basic_string<Alloc>::compare(const T &t) const noexcept {
		return compare(begin(), end(), t.begin(), t.end());
	}
	template<class Alloc>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const basic_string &str) const {
		return compare(
				begin() + pos1,
				begin() + pos1 + std::min(n1, size() - pos1),
				str.begin(), str.end());

	}
	template<class Alloc>
	template <class T>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const T &t) const {
		return compare(
				begin() + pos1,
				begin() + pos1 + std::min(n1, size() - pos1),
				t.begin(), t.end());

	}
	template<class Alloc>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const basic_string &str, size_type pos2, size_type n2) const {
		return compare(begin() + pos1,
					   begin() + pos1 + std::min(n1, size() - pos1),
					   str.begin() + pos2,
					   str.begin() + pos2 + std::min(n2, str.size() - pos2));
	}

	template<class Alloc>
	template <class T>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const T &t, size_type pos2, size_type n2) const {
		return compare(begin() + pos1,
					   begin() + pos1 + std::min(n1, size() - pos1),
					   t.begin() + pos2,
					   t.begin() + pos2 + std::min(n2, t.size() - pos2));
	}

	template<class Alloc>
	int basic_string<Alloc>::compare(const value_type *s) const noexcept {
		return compare(begin(), end(), s, s + std::strlen(s));

	}

	template<class Alloc>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const value_type *s) const {
		return compare(begin() + pos1,
					   begin() + pos1 + std::min(n1, size() - pos1),
					   s,
					   s + std::strlen(s));
	}

	template<class Alloc>
	int basic_string<Alloc>::compare(size_type pos1, size_type n1, const value_type *s, size_type pos2, size_type n2) const {
		return compare(begin() + pos1,
					   begin() + pos1 + std::min(n1, size() - pos1),
					   s,
					   s + n2);
	}

	template<class Alloc>
	int basic_string<Alloc>::compare(const value_type* begin1, const value_type* end1, const value_type* begin2, const value_type* end2) {
		const difference_type n1 = end1 - begin1;
		const difference_type n2 = end2 - begin2;
		const difference_type min = std::min(n1, n2);
		const int cmp  = Compare(begin1, begin2, (size_t)min);
		return (cmp != 0 ? cmp : (n1 < n2 ? -1 : (n1 > n2 ? 1 : 0)));
	}
#pragma endregion Compare

#pragma region Operators
	template<class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(const basic_string& str) {
		return assign(str);
	}

	template<class Alloc>
	template<typename T>
	basic_string<Alloc>& basic_string<Alloc>::operator=(const T& t) {
		return assign(t);
	}

	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(basic_string&& str) noexcept(allocator_type::propagate_on_container_move_assignment::value || allocator_type::is_always_equal::value) {
		return assign(std::move(str));
	}

	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(const value_type* s) {
		return assign(s);
	}

	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(value_type c) {
		return assign(1, c);
	}

	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator=(std::initializer_list<value_type> ilist) {
		return assign(ilist);
	}

	template <class Alloc>
	basic_string<Alloc>&  basic_string<Alloc>::operator+=(const basic_string &str) {
		return append(str);
	}
	template <class Alloc>
	template <class T>
	basic_string<Alloc> &basic_string<Alloc>::operator+=(const T &t) {
		return append(t);
	}
	template <class Alloc>
	basic_string<Alloc>&  basic_string<Alloc>::operator+=(const value_type *s) {
		return append(s);
	}
	template <class Alloc>
	basic_string<Alloc>&  basic_string<Alloc>::operator+=(value_type c) {
		return append(c);
	}
	template <class Alloc>
	basic_string<Alloc>&  basic_string<Alloc>::operator+=(std::initializer_list<value_type> ilist) {
		return append(ilist);
	}

	template <class Alloc>
	bool basic_string<Alloc>::operator==(const value_type* str) const noexcept {
		auto s1 = size();
		auto s2 = std::strlen(str);
		return s1 == s2 && Compare(data(), str, s2) == 0;
	}

	template <class Alloc>
	bool basic_string<Alloc>::operator==(const basic_string& str) const noexcept {
		if (this == &str) {
			return true;
		}

		auto s1 = size();
		auto s2 = str.size();
		return s1 == s2 && Compare(data(), str.data(), s2) == 0;
	}

	template <class Alloc>
	template<typename T>
	bool basic_string<Alloc>::operator==(const T &t) const noexcept {
		auto s1 = size();
		auto s2 = t.size();
		return s1 == s2 && Compare(data(), t.data(), s2) == 0;
	}

#pragma endregion Operators

#pragma region Helpers

	template<class Alloc>
	void basic_string<Alloc>::reserve(size_type n) {
		// C++20 says if the passed in capacity is less than the current capacity we do not shrink
		// If new_cap is less than or equal to the current capacity(), there is no effect.
		n = std::max(n, size());
		if (n <= capacity())
			return;

		auto a = get_allocator();
		auto ptr = a.allocate(n);
		std::memcpy(ptr, data(), size() * sizeof(value_type));

		const auto len = n - 1;
		ptr[len] = value_type{'\0'};

		deallocate_self();

		const auto cap = gen_capacity(n);
		const auto cat = get_category(cap);

		switch (cat) {
			case Category::Short: {
				assert(n < __max_short_size);
				std::memcpy(_members._short._data, ptr, n * sizeof(value_type));
				set_length(len, Category::Short);
				a.deallocate(ptr, n);
				break;
			}
			case Category::Mid:
				_members._mid._ptr = ptr;
				set_length(len, Category::Mid);
				set_capacity(cap);
				break;
			case Category::Long:
				_members._long._cbptr = new ControlBlock(ptr, n, cap, a);
				set_length(len, Category::Long);
				set_capacity(cap);
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::construct(const value_type *s) {
		const auto len = std::strlen(s);
		construct(s, len);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct(const value_type *s, size_type len) {
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
	void basic_string<Alloc>::construct(const basic_string &str) {
		switch (str.category()) {
			case Category::Short:
				construct_string_short(str.c_str(), str.length(), __max_short_size);
				break;
			case Category::Mid:
				construct_string_mid(str.c_str(), str.length(), str.capacity());
				break;
			case Category::Long:
				_members._long._cbptr = str._members._long._cbptr->acquire();
				_members._long._len = str._members._long._len;
				_members._long._cap = str._members._long._cap;
				break;
		}
	}

	/*namespace details {
		template<typename T>
		struct is_reverse_iterator : std::false_type { };

		template<typename T>
		struct is_reverse_iterator<std::reverse_iterator<T>> : std::true_type { };

		template<typename T>
		struct is_const_reverse_iterator : std::false_type { };

		template<typename T>
		struct is_const_reverse_iterator<const std::reverse_iterator<T>> : std::true_type { };
	}*/

	template<class Alloc>
	template<class InputIterator>
	void basic_string<Alloc>::construct(InputIterator first, InputIterator last) {
		const auto len = std::distance(first, last);
		const auto cap = gen_capacity(len + 1);
		const auto cat = get_category(cap);

		constexpr bool is_random = std::random_access_iterator<InputIterator>;
		//constexpr bool is_reverse = details::is_reverse_iterator<InputIterator>::value || details::is_const_reverse_iterator<InputIterator>::value;

		switch (cat) {
			case Category::Short:
				/*for (size_type i = 0; i < len; ++i, ++first) {
					std::construct_at(_members._short._data + i, *first);
				}
				std::construct_at(_members._short._data + len, '\0');*/
				if constexpr (is_random) {
					std::memmove(_members._short._data, *first, len);
				} else {
					for (size_type i = 0; i < len; ++i, ++first) {
						_members._short._data[i] = *first;
					}
				}
				_members._short._data[len] = value_type{'\0'};
				set_length(len, Category::Short);
				break;
			case Category::Mid:
				_members._mid._ptr = get_allocator().allocate(cap);
				/*for (size_type i = 0; i < len; ++i, ++first) {
					std::construct_at(_members._mid._ptr + i, *first);
				}
				std::construct_at(_members._mid._ptr + len, '\0');*/
				if constexpr (is_random) {
					std::memmove(_members._mid._ptr, *first, len);
				} else {
					for (size_type i = 0; i < len; ++i, ++first) {
						_members._mid._ptr[i] = *first;
					}
				}
				_members._short._data[len] = value_type{'\0'};
				set_length(len, Category::Mid);
				set_capacity(cap);
				break;
			case Category::Long:
				auto a = get_allocator();
				auto ptr = a.allocate(cap);
				/*for (size_type i = 0; i < len; ++i, ++first) {
					std::construct_at(ptr + i, *first);
				}
				std::construct_at(ptr + len, '\0');*/
				if constexpr (is_random) {
					std::memmove(ptr, *first, len);
				} else {
					for (size_type i = 0; i < len; ++i, ++first) {
						ptr[i] = *first;
					}
				}
				ptr[len] = value_type{'\0'};
				set_length(len, Category::Long);
				set_capacity(cap);
				_members._long._cbptr = new ControlBlock(ptr, len + 1, cap, a);
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::construct(size_type n, value_type c) {
		const auto cap = gen_capacity(n + 1);
		const auto cat = get_category(cap);

		switch (cat) {
			case Category::Short: {
				assert(n < __max_short_size);
				/*for (size_type i = 0; i < n; i++) {
					std::construct_at(_members._short._data + i, c);
				}
				std::construct_at(_members._short._data + n, '\0');*/
				std::memset(_members._short._data, c, n);
				_members._short._data[n] = value_type{'\0'};
				set_length(n, Category::Short);
				break;
			}
			case Category::Mid:
				_members._mid._ptr = get_allocator().allocate(cap);
				/*for (size_type i = 0; i < n; i++) {
					std::construct_at(_members._mid._ptr + i, c);
				}
				std::construct_at(_members._mid._ptr + n, '\0');*/
				std::memset(_members._mid._ptr, c, n);
				_members._mid._ptr[n] = value_type{'\0'};
				set_length(n, Category::Mid);
				set_capacity(cap);
				break;
			case Category::Long:
				auto a = get_allocator();
				auto ptr = a.allocate(cap);
				/*for (size_type i = 0; i < n; i++) {
					std::construct_at(ptr + i, c);
				}
				std::construct_at(ptr + n, '\0');*/
				std::memset(ptr, c, n);
				ptr[n] = value_type{'\0'};
				set_length(n, Category::Long);
				set_capacity(cap);
				_members._long._cbptr = new ControlBlock(ptr, n + 1, cap, a);
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_string_empty() {
		//std::construct_at(_members._short._data, '\0');
		_members._short._data[0] = value_type{'\0'};
		set_length(0, Category::Short);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_string_short(const basic_string::value_type *s, basic_string::size_type len,
													 basic_string::size_type /*cap*/) {
		assert(len < __max_short_size);
		/*for (size_type i = 0; i < len; i++) {
			std::construct_at(_members._short._data + i, s[i]);
		}
		std::construct_at(_members._short._data + len, '\0');*/
		std::memmove(_members._short._data, s, len * sizeof(value_type));
		_members._short._data[len] = value_type{'\0'};

		set_length(len, Category::Short);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_string_mid(const basic_string::value_type *s, basic_string::size_type len,
												   basic_string::size_type cap) {
		assert(len < cap);
		_members._mid._ptr = get_allocator().allocate(cap);
		// use placement new to construct each character at the allocated memory
		/*for (size_type i = 0; i < len; i++) {
			std::construct_at(_members._mid._ptr + i, s[i]);
		}
		std::construct_at(_members._mid._ptr + len, '\0');*/
		std::memcpy(_members._mid._ptr, s, len * sizeof(value_type));
		_members._mid._ptr[len] = value_type{'\0'};

		set_length(len, Category::Mid);
		set_capacity(cap);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_string_long(const basic_string::value_type *s, basic_string::size_type len,
													basic_string::size_type cap) {
		assert(len < cap);
		auto a = get_allocator();
		auto ptr = a.allocate(cap);
		/*for (size_type i = 0; i < len; i++) {
			std::construct_at(ptr + i, s[i]);
		}
		std::construct_at(ptr + len, '\0');*/
		std::memcpy(ptr, s, len * sizeof(value_type));
		ptr[len] = value_type{'\0'};

		set_length(len, Category::Long);
		set_capacity(cap);
		_members._long._cbptr = new ControlBlock(ptr, len + 1, cap, a);
	}

	template<class Alloc>
	uint8_t& basic_string<Alloc>::msbyte() {
		return *reinterpret_cast<uint8_t*>(this + 23);
	}

	/*template<class Alloc>
	void basic_string<Alloc>::print_mem() const {
		uint8_t mem[24];
		memcpy(mem, this, 24);
		std::cerr << "Mem: ";
		for (int i = 0; i < 24; i++)
			std::cerr << std::hex << uint32_t{mem[i]} << " ";
		std::cerr << std::endl;
	}*/

	template<class Alloc>
	uint8_t basic_string<Alloc>::cmsbyte() const {
		return reinterpret_cast<const uint8_t*>(this)[23];
	}

	template<class Alloc>
	typename basic_string<Alloc>::value_type *
	basic_string<Alloc>::begin_ptr() {
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
	typename basic_string<Alloc>::value_type *
	basic_string<Alloc>::end_ptr() {
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
	const typename basic_string<Alloc>::value_type *
	basic_string<Alloc>::begin_ptr() const {
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
	const typename basic_string<Alloc>::value_type *
	basic_string<Alloc>::end_ptr() const {
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
	void basic_string<Alloc>::deallocate_self() {
		// TODO: What if invalid/freed after move
		auto cat = category();
		switch (cat) {
			case Category::Short:
				//std::destroy_at(_members._short._data);
				break;
			case Category::Mid:
				/*for (size_type i = 0; i < _members._mid._len; i++) {
					std::destroy_at(_members._mid._ptr + i);
				}*/
				get_allocator().deallocate(_members._mid._ptr, _members._mid._cap);
				break;
			case Category::Long:
				_members._long._cbptr->release();
				break;
		}
	}

	template<class Alloc>
	typename basic_string<Alloc>::size_type
	basic_string<Alloc>::gen_capacity(size_type len) {
		if (len <= __max_short_size)
			return len;
		size_type ret{1};
		while (ret < len)
			ret = ret << 1;
		return ret;
	}

	template<class Alloc>
	bool basic_string<Alloc>::msb() const {
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
	typename basic_string<Alloc>::Category basic_string<Alloc>::category() const {
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
	basic_string<Alloc>::get_category(size_type cap) {
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
		assert(this->category() == Category::Long);

		// do not need to do anything if you are holding the only reference
		if (_members._long._cbptr->count() == 1)
			return;

		auto prev_cb = _members._long._cbptr;
		_members._long._cbptr = new ControlBlock(*prev_cb);
		prev_cb->release();
	}

#pragma endregion Helpers

#pragma region ControlBlock
	template<class Alloc>
	basic_string<Alloc>::ControlBlock::ControlBlock(pointer p, size_type len, size_type cap, const allocator_type& a)
		: _ptr{p}, _count{0}, _len{len}, _cap{cap}, _allocator{a} {
		acquire();
	}

	template<class Alloc>
	basic_string<Alloc>::ControlBlock::ControlBlock(const ControlBlock& other)
		: _ptr{nullptr}, _count{1}, _len{other._len}, _cap{other._cap}, _allocator{other._allocator} {
		_ptr = _allocator.allocate(_cap);
		/*for (size_type i = 0; i < _len; i++) {
			std::construct_at(_ptr + i, other._ptr[i]);
		}
		std::construct_at(_ptr + _len, '\0');*/
		std::memcpy(_ptr, other._ptr, _len * sizeof(value_type));
		_ptr[_len] = value_type{'\0'};
	}

	template<class Alloc>
	basic_string<Alloc>::ControlBlock::~ControlBlock() {
		assert(_count == 0);
		assert(_ptr);
		/*for (size_type i = 0; i < _len; i++) {
			std::destroy_at(_ptr + i);
		}*/
		_allocator.deallocate(_ptr, _cap);
	}

	template<class Alloc>
	typename basic_string<Alloc>::ControlBlock*
	basic_string<Alloc>::ControlBlock::acquire() {
		_count++;
		return this;
	}

	template<class Alloc>
	typename basic_string<Alloc>::pointer
	basic_string<Alloc>::ControlBlock::get() const {
		return _ptr;
	}

	template<class Alloc>
	typename basic_string<Alloc>::size_type
	basic_string<Alloc>::ControlBlock::count() const {
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
}// namespace plugify

template <>
struct std::formatter<plg::string> : std::formatter<std::string_view> {
	auto format(const plg::string& str, std::format_context& ctx) const {
		std::string temp;
		std::format_to(std::back_inserter(temp), "{}", std::string_view(str.data(), str.size()));
		return std::formatter<string_view>::format(temp, ctx);
	}
};

// TODO: implement the following API

/* size_type max_size() const noexcept; */

/* void resize(size_type n, value_type c); */
/* void resize(size_type n); */

/* basic_string &insert(size_type pos1, const basic_string &str); */
/* template <class T> basic_string &insert(size_type pos1, const T &t); */
/* basic_string &insert(size_type pos1, const basic_string &str, */
/*					   size_type pos2, size_type n); */
/* template <class T> */
/* basic_string &insert(size_type pos1, const T &t, size_type pos2, */
/*					   size_type n); // C++17 */
/* basic_string &insert(size_type pos, const value_type *s, */
/*					   size_type n = npos); // C++14 */
/* basic_string &insert(size_type pos, const value_type *s); */
/* basic_string &insert(size_type pos, size_type n, value_type c); */
/* iterator insert(const_iterator p, value_type c); */
/* iterator insert(const_iterator p, size_type n, value_type c); */
/* template <class InputIterator> */
/* iterator insert(const_iterator p, InputIterator first, InputIterator last);
 */
/* iterator insert(const_iterator p, std::initializer_list<value_type>); */

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

/* size_type copy(value_type *s, size_type n, size_type pos = 0) const; */
/* basic_string substr(size_type pos = 0, size_type n = npos) const; */

/* void swap(basic_string &str) noexcept( */
/*	 std::allocator_traits< */
/*		 allocator_type>::propagate_on_container_swap::value || */
/*	 std::allocator_traits<allocator_type>::is_always_equal::value); //
 * C++17 */

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