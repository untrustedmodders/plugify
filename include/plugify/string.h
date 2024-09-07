#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace plugify {
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
		typedef char& reference;
		typedef const char& const_reference;
		typedef typename __alloc_traits::pointer pointer;
		typedef typename __alloc_traits::const_pointer const_pointer;
		typedef pointer iterator;
		typedef const_pointer const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		static const size_type npos = static_cast<size_type>(-1);

		basic_string(const basic_string& str);
		basic_string(const char* s, const allocator_type& a = allocator_type());
		~basic_string();

		size_type size() const noexcept;
		size_type length() const noexcept;
		size_type capacity() const noexcept;
		const_reference operator[](size_type pos) const;
		reference operator[](size_type pos);
		const char* c_str() const noexcept;

	private:
		static constexpr size_type __string_max_size = sizeof(size_type) * 2 + sizeof(pointer);
		static constexpr size_type __max_short_size = 23;
		static constexpr size_type __max_mid_size = 255;

		enum class Category {
			kShort = 0,
			kMid,
			kLong
		};

		class ControlBlock {
			pointer _ptr;
			std::atomic<size_type> _count;
			size_type _len;
			size_type _cap;

		public:
			ControlBlock(pointer p, size_type len, size_type cap);

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
			char _data[(__string_max_size - sizeof(uint8_t)) / sizeof(char)];
			uint8_t _len;
		};
		struct ShortTag {};
		static constexpr ShortTag short_tag{};

		struct Mid {
			pointer _ptr;
			size_type _len;
			size_type _cap;
		};
		struct MidTag {};
		static constexpr MidTag mid_tag{};

		struct Long {
			ControlBlock* _cbptr;
			size_type _len;
			size_type _cap;
		};
		struct LongTag {};
		static constexpr LongTag long_tag{};

		static_assert(sizeof(Short) <= __string_max_size,
					  "Short struct is larger than max size");
		static_assert(sizeof(Mid) <= __string_max_size,
					  "Mid struct is larger than max size");
		static_assert(sizeof(Long) <= __string_max_size,
					  "Long struct is larger than max size");

		union Members {
			Short _short;
			Mid _mid;
			Long _long;
		};
		Members _members;

		Category category() const;
		static Category get_category(size_type cap);

		bool msb() const;

		uint8_t& msbyte();
		uint8_t cmsbyte() const;

		void set_capacity(size_type cap);
		static size_type gen_capacity(size_type len);
		void set_length(size_type len, Category cat);
		//void print_mem() const;

		void construct_basic_string(const char* s, size_type len,
									const allocator_type& a, ShortTag);
		void construct_basic_string(const char* s, size_type len, size_type cap,
									const allocator_type& a, MidTag);
		void construct_basic_string(const char* s, size_type len, size_type cap,
									const allocator_type& a, LongTag);

		void mutable_cb();
	};

	using string = basic_string<std::allocator<char>>;

	/** Constructors ***********************************************************/

	template<class Alloc>
	basic_string<Alloc>::~basic_string() {
		auto cat = category();
		switch (cat) {
			case Category::kShort:
				std::destroy_at(_members._short._data);
				break;
			case Category::kMid:
				for (size_type i = 0; i < _members._mid._len; i++) {
					std::destroy_at(_members._mid._ptr + i);
				}
				allocator_type().deallocate(_members._mid._ptr, _members._mid._cap);
				break;
			case Category::kLong:
				_members._long._cbptr->release();
				break;
		}
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string(const char* s, const allocator_type& a) {
		const auto len = std::strlen(s);
		const auto cap = gen_capacity(len + 1);
		const auto cat = get_category(cap);
		switch (cat) {
			case Category::kShort:
				construct_basic_string(s, len, a, short_tag);
				break;
			case Category::kMid:
				construct_basic_string(s, len, cap, a, mid_tag);
				break;
			case Category::kLong:
				construct_basic_string(s, len, cap, a, long_tag);
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_basic_string(const char* s, size_type len,
													 const allocator_type& a,
													 ShortTag) {
		assert(len < __max_short_size);
		for (size_type i = 0; i < len; i++) {
			new (_members._short._data + i) char{s[i]};
		}
		new (_members._short._data + len) char{'\0'};
		set_length(len, Category::kShort);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_basic_string(const char* s, size_type len,
													 size_type cap,
													 const allocator_type& a,
													 MidTag) {
		assert(len < cap);
		_members._mid._ptr = allocator_type().allocate(cap);

		// use placement new to construct each character at the allocated memory
		for (size_type i = 0; i < len; i++) {
			std::construct_at(_members._mid._ptr + i, s[i]);
		}
		std::construct_at(_members._mid._ptr + len, '\0');

		set_length(len, Category::kMid);
		set_capacity(cap);
	}

	template<class Alloc>
	void basic_string<Alloc>::construct_basic_string(const char* s, size_type len,
													 size_type cap,
													 const allocator_type& a,
													 LongTag) {
		assert(len < cap);
		auto ptr = allocator_type().allocate(cap);
		for (size_type i = 0; i < len; i++) {
			std::construct_at(ptr + i, s[i]);
		}
		std::construct_at(ptr + len, '\0');
		set_length(len, Category::kLong);
		set_capacity(cap);
		_members._long._cbptr = new ControlBlock(ptr, len + 1, cap);
	}

	template<class Alloc>
	uint8_t& basic_string<Alloc>::msbyte() {
		return *reinterpret_cast<uint8_t*>(this + 23);
	}

	template<class Alloc>
	basic_string<Alloc>::basic_string(const basic_string& str) {
		auto cat = str.category();
		switch (cat) {
			case Category::kShort:
				assert(str.length() < __max_short_size);
				construct_basic_string(str.c_str(), str.length(), allocator_type(),
									   short_tag);
				break;
			case Category::kMid:
				construct_basic_string(str.c_str(), str.length(), str.capacity(),
									   allocator_type(), mid_tag);
				break;
			case Category::kLong:
				_members._long._cap = str._members._long._cap;
				_members._long._cbptr = str._members._long._cbptr->acquire();
				_members._long._len = str._members._long._len;
				break;
		}
	}

	/** Public Functions *******************************************************/

	template<class Alloc>
	const char* basic_string<Alloc>::c_str() const noexcept {
		switch (category()) {
			case Category::kShort:
				return _members._short._data;
			case Category::kMid:
				return _members._mid._ptr;
			case Category::kLong:
				return _members._long._cbptr->get();
		}
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
			case Category::kShort:
				return _members._short._len ^ (1 << 7);
			case Category::kMid:
				return _members._mid._len;
			case Category::kLong:
				return _members._long._len;
		}
	}

	template<class Alloc>
	typename basic_string<Alloc>::const_reference basic_string<Alloc>::operator[](
			typename basic_string<Alloc>::size_type pos) const {
		return c_str()[pos];
	};

	template<class Alloc>
	typename basic_string<Alloc>::reference
	basic_string<Alloc>::operator[](typename basic_string<Alloc>::size_type pos) {
		if (category() == Category::kLong)
			mutable_cb();
		switch (category()) {
			case Category::kShort:
				return _members._short._data[pos];
			case Category::kMid:
				return _members._mid._ptr[pos];
			case Category::kLong:
				return _members._long._cbptr->get()[pos];
		}
	}

	/** Helpers ****************************************************************/

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
	void basic_string<Alloc>::set_length(basic_string<Alloc>::size_type length, Category cat) {
		switch (cat) {
			case Category::kShort:
				_members._short._len = static_cast<uint8_t>(static_cast<uint8_t>(length) | static_cast<uint8_t>(1 << 7));
				break;
			case Category::kMid:
				_members._mid._len = length;
				break;
			case Category::kLong:
				_members._long._len = length;
				break;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::set_capacity(basic_string<Alloc>::size_type capacity) {
		const auto cat = get_category(capacity);
		switch (cat) {
			case Category::kShort:
				assert(capacity == __max_short_size);
				break;
			case Category::kMid:
				_members._mid._cap = capacity;
				break;
			case Category::kLong:
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
			//std::shared_ptr<char> s;
			return *reinterpret_cast<const size_type*>(cap_byte_ptr);
		}
	}

	template<class Alloc>
	typename basic_string<Alloc>::Category basic_string<Alloc>::category() const {
		if (msb()) {
			return Category::kShort;
		} else {
			const auto cap = capacity();
			if (cap > __max_mid_size) {
				return Category::kLong;
			} else {
				return Category::kMid;
			}
		}
	}

	template<class Alloc>
	typename basic_string<Alloc>::Category
	basic_string<Alloc>::get_category(size_type cap) {
		if (cap <= __max_short_size) {
			return Category::kShort;
		} else if (cap <= __max_mid_size) {
			return Category::kMid;
		} else {
			return Category::kLong;
		}
	}

	template<class Alloc>
	void basic_string<Alloc>::mutable_cb() {
		assert(this->category() == Category::kLong);

		// do not need to do anything if you are holding the only reference
		if (_members._long._cbptr->count() == 1)
			return;

		auto prev_cb = _members._long._cbptr;
		_members._long._cbptr = new ControlBlock(*prev_cb);
		prev_cb->release();
	}

	/** Control Block **********************************************************/

	template<class Alloc>
	basic_string<Alloc>::ControlBlock::ControlBlock(pointer p, size_type len, size_type cap)
		: _ptr{p}, _count{0}, _len{len}, _cap{cap} {
		acquire();
	}

	template<class Alloc>
	basic_string<Alloc>::ControlBlock::ControlBlock(const ControlBlock& other)
		: _ptr{nullptr}, _count{1}, _len{other._len}, _cap{other._cap} {
		_ptr = allocator_type().allocate(_cap);
		for (size_type i = 0; i < _len; i++) {
			std::construct_at(_ptr + i, other._ptr[i]);
		}
		std::construct_at(_ptr + _len, '\0');
	}

	template<class Alloc>
	basic_string<Alloc>::ControlBlock::~ControlBlock() {
		assert(_count == 0);
		assert(_ptr);
		for (size_type i = 0; i < _len; i++) {
			std::destroy_at(_ptr + i);
		}
		allocator_type().deallocate(_ptr, _cap);
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

}// namespace plugify

// TODO: implement the following API
/* basic_string() noexcept( */
/*     std::is_nothrow_default_constructible<allocator_type>::value); */
/* explicit basic_string(const allocator_type &a); */
/* basic_string(basic_string &&str) noexcept( */
/*     std::is_nothrow_move_constructible<allocator_type>::value); */
/* basic_string(const basic_string &str, size_type pos, */
/*               const allocator_type &a = allocator_type()); */
/* basic_string(const basic_string &str, size_type pos, size_type n, */
/*               const Alloc &a = Alloc()); */
/* template <class T> */
/* basic_string(const T &t, size_type pos, size_type n, */
/*               const Alloc &a = Alloc()); */
/* template <class T> */
/* explicit basic_string(const T &t, const Alloc &a = Alloc()); */
/* basic_string(const char *s, size_type n, */
/*               const allocator_type &a = allocator_type()); */
/* basic_string(size_type n, char c, */
/*               const allocator_type &a = allocator_type()); */
/* template <class InputIterator> */
/* basic_string(InputIterator begin, InputIterator end, */
/*               const allocator_type &a = allocator_type()); */
/* basic_string(std::initializer_list<char>, const Alloc & = Alloc()); */
/* basic_string(const basic_string &, const Alloc &); */
/* basic_string(basic_string &&, const Alloc &); */


/* basic_string &operator=(const basic_string &str); */
/* template <class T> basic_string &operator=(const T &t); // C++17 */
/* basic_string &operator=(basic_string &&str) noexcept( */
/*     allocator_type::propagate_on_container_move_assignment::value || */
/*     allocator_type::is_always_equal::value); // C++17 */
/* basic_string &operator=(const char *s); */
/* basic_string &operator=(char c); */
/* basic_string &operator=(std::initializer_list<char>); */

/* iterator begin() noexcept; */
/* const_iterator begin() const noexcept; */
/* iterator end() noexcept; */
/* const_iterator end() const noexcept; */

/* reverse_iterator rbegin() noexcept; */
/* const_reverse_iterator rbegin() const noexcept; */
/* reverse_iterator rend() noexcept; */
/* const_reverse_iterator rend() const noexcept; */

/* const_iterator cbegin() const noexcept; */
/* const_iterator cend() const noexcept; */
/* const_reverse_iterator crbegin() const noexcept; */
/* const_reverse_iterator crend() const noexcept; */

/* size_type max_size() const noexcept; */

/* void resize(size_type n, char c); */
/* void resize(size_type n); */

/* void reserve(size_type res_arg = 0); */
/* void shrink_to_fit(); */
/* void clear() noexcept; */
/* bool empty() const noexcept; */

/* const_reference at(size_type n) const; */
/* reference at(size_type n); */

/* basic_string &operator+=(const basic_string &str); */
/* template <class T> basic_string &operator+=(const T &t); // C++17 */
/* basic_string &operator+=(const char *s); */
/* basic_string &operator+=(char c); */
/* basic_string &operator+=(std::initializer_list<char>); */

/* basic_string &append(const basic_string &str); */
/* template <class T> basic_string &append(const T &t); // C++17 */
/* basic_string &append(const basic_string &str, size_type pos, */
/*                       size_type n = npos); // C++14 */
/* template <class T> */
/* basic_string &append(const T &t, size_type pos, size_type n = npos); //
 * C++17 */
/* basic_string &append(const char *s, size_type n); */
/* basic_string &append(const char *s); */
/* basic_string &append(size_type n, char c); */
/* template <class InputIterator> */
/* basic_string &append(InputIterator first, InputIterator last); */
/* basic_string &append(std::initializer_list<char>); */

/* void push_back(char c); */
/* void pop_back(); */
/* reference front(); */
/* const_reference front() const; */
/* reference back(); */
/* const_reference back() const; */

/* basic_string &assign(const basic_string &str); */
/* template <class T> basic_string &assign(const T &t); // C++17 */
/* basic_string &assign(basic_string &&str); */
/* basic_string &assign(const basic_string &str, size_type pos, */
/*                       size_type n = npos); // C++14 */
/* template <class T> */
/* basic_string &assign(const T &t, size_type pos, size_type n = npos); //
 * C++17 */
/* basic_string &assign(const char *s, size_type n); */
/* basic_string &assign(const char *s); */
/* basic_string &assign(size_type n, char c); */
/* template <class InputIterator> */
/* basic_string &assign(InputIterator first, InputIterator last); */
/* basic_string &assign(std::initializer_list<char>); */

/* basic_string &insert(size_type pos1, const basic_string &str); */
/* template <class T> basic_string &insert(size_type pos1, const T &t); */
/* basic_string &insert(size_type pos1, const basic_string &str, */
/*                       size_type pos2, size_type n); */
/* template <class T> */
/* basic_string &insert(size_type pos1, const T &t, size_type pos2, */
/*                       size_type n); // C++17 */
/* basic_string &insert(size_type pos, const char *s, */
/*                       size_type n = npos); // C++14 */
/* basic_string &insert(size_type pos, const char *s); */
/* basic_string &insert(size_type pos, size_type n, char c); */
/* iterator insert(const_iterator p, char c); */
/* iterator insert(const_iterator p, size_type n, char c); */
/* template <class InputIterator> */
/* iterator insert(const_iterator p, InputIterator first, InputIterator last);
 */
/* iterator insert(const_iterator p, std::initializer_list<char>); */

/* basic_string &erase(size_type pos = 0, size_type n = npos); */
/* iterator erase(const_iterator position); */
/* iterator erase(const_iterator first, const_iterator last); */

/* basic_string &replace(size_type pos1, size_type n1, */
/*                        const basic_string &str); */
/* template <class T> */
/* basic_string &replace(size_type pos1, size_type n1, const T &t); // C++17
 */
/* basic_string &replace(size_type pos1, size_type n1, const basic_string
 * &str, */
/*                        size_type pos2, size_type n2 = npos); // C++14 */
/* template <class T> */
/* basic_string &replace(size_type pos1, size_type n1, const T &t, */
/*                        size_type pos2, size_type n); // C++17 */
/* basic_string &replace(size_type pos, size_type n1, const char *s, */
/*                        size_type n2); */
/* basic_string &replace(size_type pos, size_type n1, const char *s); */
/* basic_string &replace(size_type pos, size_type n1, size_type n2, char c);
 */
/* basic_string &replace(const_iterator i1, const_iterator i2, */
/*                        const basic_string &str); */
/* template <class T> */
/* basic_string &replace(const_iterator i1, const_iterator i2, */
/*                        const T &t); // C++17 */
/* basic_string &replace(const_iterator i1, const_iterator i2, const char *s,
 */
/*                        size_type n); */
/* basic_string &replace(const_iterator i1, const_iterator i2, const char
 * *s); */
/* basic_string &replace(const_iterator i1, const_iterator i2, size_type n,
 */
/*                        char c); */
/* template <class InputIterator> */
/* basic_string &replace(const_iterator i1, const_iterator i2, InputIterator
 * j1, */
/*                        InputIterator j2); */
/* basic_string &replace(const_iterator i1, const_iterator i2, */
/*                        std::initializer_list<char>); */

/* size_type copy(char *s, size_type n, size_type pos = 0) const; */
/* basic_string substr(size_type pos = 0, size_type n = npos) const; */

/* void swap(basic_string &str) noexcept( */
/*     std::allocator_traits< */
/*         allocator_type>::propagate_on_container_swap::value || */
/*     std::allocator_traits<allocator_type>::is_always_equal::value); //
 * C++17 */

/* const char *data() const noexcept; */
/* char *data() noexcept; // C++17 */

/* allocator_type get_allocator() const noexcept; */

/* size_type find(const basic_string &str, size_type pos = 0) const noexcept;
 */
/* template <class T> */
/* size_type find(const T &t, size_type pos = 0) const; // C++17 */
/* size_type find(const char *s, size_type pos, size_type n) const noexcept;
 */
/* size_type find(const char *s, size_type pos = 0) const noexcept; */
/* size_type find(char c, size_type pos = 0) const noexcept; */

/* size_type rfind(const basic_string &str, */
/*                 size_type pos = npos) const noexcept; */
/* template <class T> */
/* size_type rfind(const T &t, size_type pos = npos) const; // C++17 */
/* size_type rfind(const char *s, size_type pos, size_type n) const noexcept;
 */
/* size_type rfind(const char *s, size_type pos = npos) const noexcept; */
/* size_type rfind(char c, size_type pos = npos) const noexcept; */

/* size_type find_first_of(const basic_string &str, */
/*                         size_type pos = 0) const noexcept; */
/* template <class T> */
/* size_type find_first_of(const T &t, size_type pos = 0) const; // C++17 */
/* size_type find_first_of(const char *s, size_type pos, */
/*                         size_type n) const noexcept; */
/* size_type find_first_of(const char *s, size_type pos = 0) const noexcept;
 */
/* size_type find_first_of(char c, size_type pos = 0) const noexcept; */

/* size_type find_last_of(const basic_string &str, */
/*                        size_type pos = npos) const noexcept; */
/* template <class T> */
/* size_type find_last_of(const T &t, */
/*                        size_type pos = npos) const noexcept; // C++17 */
/* size_type find_last_of(const char *s, size_type pos, */
/*                        size_type n) const noexcept; */
/* size_type find_last_of(const char *s, size_type pos = npos) const noexcept;
 */
/* size_type find_last_of(char c, size_type pos = npos) const noexcept; */

/* size_type find_first_not_of(const basic_string &str, */
/*                             size_type pos = 0) const noexcept; */
/* template <class T> */
/* size_type find_first_not_of(const T &t, size_type pos = 0) const; // C++17
 */
/* size_type find_first_not_of(const char *s, size_type pos, */
/*                             size_type n) const noexcept; */
/* size_type find_first_not_of(const char *s, size_type pos = 0) const
 * noexcept; */
/* size_type find_first_not_of(char c, size_type pos = 0) const noexcept; */

/* size_type find_last_not_of(const basic_string &str, */
/*                            size_type pos = npos) const noexcept; */
/* template <class T> */
/* size_type find_last_not_of(const T &t, size_type pos = npos) const; //
 * C++17 */
/* size_type find_last_not_of(const char *s, size_type pos, */
/*                            size_type n) const noexcept; */
/* size_type find_last_not_of(const char *s, */
/*                            size_type pos = npos) const noexcept; */
/* size_type find_last_not_of(char c, size_type pos = npos) const noexcept; */

/* int compare(const basic_string &str) const noexcept; */
/* template <class T> int compare(const T &t) const noexcept; // C++17 */
/* int compare(size_type pos1, size_type n1, const basic_string &str) const;
 */
/* template <class T> */
/* int compare(size_type pos1, size_type n1, const T &t) const; // C++17 */
/* int compare(size_type pos1, size_type n1, const basic_string &str, */
/*             size_type pos2, size_type n2 = npos) const; // C++14 */
/* template <class T> */
/* int compare(size_type pos1, size_type n1, const T &t, size_type pos2, */
/*             size_type n2 = npos) const; // C++17 */
/* int compare(const char *s) const noexcept; */
/* int compare(size_type pos1, size_type n1, const char *s) const; */
/* int compare(size_type pos1, size_type n1, const char *s, size_type n2)
 * const; */

/* bool starts_with(std::basic_string_view<char> sv) const noexcept; // C++2a
 */
/* bool starts_with(char c) const noexcept;                          // C++2a
 */
/* bool starts_with(const char *s) const;                            // C++2a
 */
/* bool ends_with(std::basic_string_view<char> sv) const noexcept;   // C++2a
 */
/* bool ends_with(char c) const noexcept;                            // C++2a
 */
/* bool ends_with(const char *s) const;                              // C++2a
 */

/* bool __invariants() const; */