#pragma once

// Just in case, because we can't ignore some warnings from `-Wpedantic` (about zero size arrays and anonymous structs when gnu extensions are disabled) on gcc
#if PLUGIFY_COMPILER_CLANG
#  pragma clang system_header
#elif PLUGIFY_COMPILER_GCC
#  pragma GCC system_header
#endif

#include <initializer_list>  // for std::initializer_list
#include <string_view>       // for std::basic_string_view
#include <type_traits>       // for std::is_constant_evaluated, std::declval, std::false_type
#include <algorithm>         // for std::min, std::max
#include <concepts>          // for std::unsigned_integral, std::signed_integral
#include <iterator>          // for std::distance, std::next, std::iterator_traits, std::input_iterator
#include <utility>           // for std::move, std::hash
#include <compare>           // for std::strong_ordering
#include <memory>            // for std::allocator, std::swap, std::allocator_traits
#include <limits>            // for std::numeric_limits
#include <charconv>          // for std::to_chars


#include <cstdint>
#include <cstddef>
#include <cstdarg>

#if PLUGIFY_STRING_CONTAINERS_RANGES && (__cplusplus <= 202002L || !__has_include(<ranges>) || !defined(__cpp_lib_containers_ranges))
#  undef PLUGIFY_STRING_CONTAINERS_RANGES
#  define PLUGIFY_STRING_CONTAINERS_RANGES 0
#endif

#if PLUGIFY_STRING_CONTAINERS_RANGES
#  include <ranges>
#endif

#ifndef PLUGIFY_STRING_NO_NUMERIC_CONVERSIONS
#  include <cstdlib>
#endif

#ifndef PLUGIFY_STRING_NO_STD_FORMAT
#include "plg/format.hpp"
#endif

#include "plg/allocator.hpp"

namespace plg {
	namespace detail {
		template<typename Alloc>
		concept is_allocator = requires(Alloc& a, std::size_t n) {
			typename std::allocator_traits<Alloc>::value_type;
			typename std::allocator_traits<Alloc>::pointer;
			typename std::allocator_traits<Alloc>::const_pointer;
			typename std::allocator_traits<Alloc>::size_type;
			typename std::allocator_traits<Alloc>::difference_type;

			{ std::allocator_traits<Alloc>::allocate(a, n) }
			-> std::convertible_to<typename std::allocator_traits<Alloc>::pointer>;

			requires requires(typename std::allocator_traits<Alloc>::pointer p) {
				std::allocator_traits<Alloc>::deallocate(a, p, n);
			};
		};

		template<typename Traits>
		concept is_char_traits = requires {
			// Required type definitions
			typename Traits::char_type;
			typename Traits::int_type;
			typename Traits::off_type;
			typename Traits::pos_type;
			typename Traits::state_type;
		} && requires(
			typename Traits::char_type c1,
			typename Traits::char_type c2,
			typename Traits::char_type& cr,
			const typename Traits::char_type& ccr,
			typename Traits::char_type* p,
			const typename Traits::char_type* cp,
			typename Traits::int_type i1,
			typename Traits::int_type i2,
			std::size_t n
		) {
			// Character operations
			{ Traits::assign(cr, ccr) } -> std::same_as<void>;
			{ Traits::eq(ccr, ccr) } -> std::convertible_to<bool>;
			{ Traits::lt(ccr, ccr) } -> std::convertible_to<bool>;

			// String operations
			{ Traits::compare(cp, cp, n) } -> std::convertible_to<int>;
			{ Traits::length(cp) } -> std::convertible_to<std::size_t>;
			{ Traits::find(cp, n, ccr) } -> std::convertible_to<const typename Traits::char_type*>;

			// Memory operations
			{ Traits::move(p, cp, n) } -> std::same_as<typename Traits::char_type*>;
			{ Traits::copy(p, cp, n) } -> std::same_as<typename Traits::char_type*>;
			{ Traits::assign(p, n, c1) } -> std::same_as<typename Traits::char_type*>;

			// int_type operations
			{ Traits::not_eof(i1) } -> std::same_as<typename Traits::int_type>;
			{ Traits::to_char_type(i1) } -> std::same_as<typename Traits::char_type>;
			{ Traits::to_int_type(c1) } -> std::same_as<typename Traits::int_type>;
			{ Traits::eq_int_type(i1, i2) } -> std::convertible_to<bool>;
			{ Traits::eof() } -> std::same_as<typename Traits::int_type>;
		};

		struct uninitialized_size_tag {};

		template<typename>
		constexpr bool dependent_false = false;

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<typename Range, typename Type>
		concept string_compatible_range = std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, Type>;
#endif // PLUGIFY_STRING_CONTAINERS_RANGES

	} // namespace detail

	// basic_string
	// based on implementations from libc++, libstdc++ and Microsoft STL
	template<typename Char, detail::is_char_traits Traits = std::char_traits<Char>, detail::is_allocator Allocator = plg::allocator<Char>>
	class basic_string {
	private:
		using allocator_traits = std::allocator_traits<Allocator>;
	public:
		using traits_type = Traits;
		using value_type = typename traits_type::char_type;
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
		using sview_type = std::basic_string_view<Char, Traits>;

		constexpr static size_type npos = static_cast<size_t>(-1);

	private:
		constexpr static auto _terminator = value_type();

		PLUGIFY_NO_UNIQUE_ADDRESS
		allocator_type _allocator;

		PLUGIFY_WARN_PUSH()

#if PLUGIFY_COMPILER_CLANG
		PLUGIFY_WARN_IGNORE("-Wgnu-anonymous-struct")
		PLUGIFY_WARN_IGNORE("-Wzero-length-array")
#elif PLUGIFY_COMPILER_GCC
		PLUGIFY_WARN_IGNORE("-Wpedantic")
#elif PLUGIFY_COMPILER_MSVC
		PLUGIFY_WARN_IGNORE(4201)
		PLUGIFY_WARN_IGNORE(4200)
#endif

		template<typename CharT, std::size_t = sizeof(CharT)>
		struct padding {
			[[maybe_unused]] uint8_t pad[sizeof(CharT) - 1];
		};

		template<typename CharT>
		struct padding<CharT, 1> {
			// template specialization to remove the padding structure to avoid warnings on zero length arrays
			// also, this allows us to take advantage of the empty-base-class optimization.
		};

		// size must correspond to the last byte of long_data.cap, so we don't want the compiler to insert
		// padding after size if sizeof(value_type) != 1; Also ensures both layouts are the same size.
		struct sso_size : padding<value_type> {
			PLUGIFY_PACK(struct {
				uint8_t spare_size : 7;
				uint8_t is_long : 1;
			});
		};

		static constexpr int char_bit = std::numeric_limits<uint8_t>::digits + std::numeric_limits<uint8_t>::is_signed;
		static_assert(char_bit == 8, "assumes an 8 bit byte.");

		struct long_data {
			pointer data;
			size_type size;
			PLUGIFY_PACK(struct {
				size_type cap : sizeof(size_type) * char_bit - 1;
				size_type is_long : 1;
			});
		};

		static constexpr size_type min_cap = (sizeof(long_data) - 1) / sizeof(value_type) > 2 ? (sizeof(long_data) - 1) / sizeof(value_type) : 2;

		struct short_data {
			value_type data[min_cap];
			sso_size size;
		};

		PLUGIFY_WARN_POP()

		static_assert(sizeof(short_data) == (sizeof(value_type) * (min_cap + 1)), "short has an unexpected size.");
		static_assert(sizeof(short_data) == sizeof(long_data), "short and long layout structures must be the same size");

		union {
			long_data _long;
			short_data _short{};
		} _storage;

		constexpr static bool fits_in_sso(size_type size) noexcept {
			return size < min_cap;
		}

		constexpr void long_init() noexcept {
			set_long(true);
			set_long_data(nullptr);
			set_long_size(0);
			set_long_cap(0);
		}

		constexpr void short_init() noexcept {
			set_long(false);
			set_short_size(0);
		}

		constexpr void default_init(size_type size) noexcept {
			if (fits_in_sso(size))
				short_init();
			else
				long_init();
		}

		constexpr auto& get_long_data() noexcept {
			return _storage._long.data;
		}

		constexpr const auto& get_long_data() const noexcept {
			return _storage._long.data;
		}

		constexpr auto& get_short_data() noexcept {
			return _storage._short.data;
		}

		constexpr const auto& get_short_data() const noexcept {
			return _storage._short.data;
		}

		constexpr void set_short_size(size_type size) noexcept {
			_storage._short.size.spare_size = min_cap - (size & 0x7F);
		}

		constexpr size_type get_short_size() const noexcept {
			return min_cap - _storage._short.size.spare_size;
		}

		constexpr void set_long_size(size_type size) noexcept {
			_storage._long.size = size;
		}

		constexpr size_type get_long_size() const noexcept {
			return _storage._long.size;
		}

		constexpr void set_long_cap(size_type cap) noexcept {
			_storage._long.cap = (cap & 0x7FFFFFFFFFFFFFFF);
		}

		constexpr size_type get_long_cap() const noexcept {
			return _storage._long.cap;
		}

		constexpr void set_long_data(value_type* data) noexcept {
			_storage._long.data = data;
		}

		constexpr bool is_long() const noexcept {
			return _storage._long.is_long == true;
		}

		constexpr void set_long(bool is_long) noexcept {
			_storage._long.is_long = is_long;
		}

		constexpr void set_size(size_type size) noexcept {
			if (is_long())
				set_long_size(size);
			else
				set_short_size(size);
		}

		constexpr sview_type view() const noexcept {
			return sview_type(data(), size());
		}

		constexpr void reallocate(size_type new_cap, bool copy_old) {
			if (new_cap == get_long_cap())
				return;

			auto old_len = get_long_size();
			auto old_cap = get_long_cap();
			auto& old_buffer = get_long_data();

			auto new_len = std::min(new_cap, old_len);
			auto new_data = allocator_traits::allocate(_allocator, new_cap + 1);

			if (old_buffer != nullptr) {
				if (old_len != 0 && copy_old)
					Traits::copy(new_data, old_buffer, new_len);
				allocator_traits::deallocate(_allocator, old_buffer, old_cap + 1);
			}

			set_long_data(new_data);
			set_long_size(new_len);
			set_long_cap(new_cap);
		}

		constexpr void deallocate() {
			if (is_long()) {
				if (auto& buffer = get_long_data(); buffer != nullptr) {
					allocator_traits::deallocate(_allocator, buffer, get_long_cap() + 1);
					buffer = nullptr;
				}
			}
		}

		constexpr void grow_to(size_type new_cap) {
			if (is_long() == true) {
				reallocate(new_cap, true);
				return;
			}

			auto buffer = allocator_traits::allocate(_allocator, new_cap + 1);
			auto len = get_short_size();

			Traits::copy(buffer, get_short_data(), len);
			Traits::assign(buffer[len], _terminator);

			long_init();
			set_long_data(buffer);
			set_long_size(len);
			set_long_cap(new_cap);
		}

		constexpr void null_terminate() {
			auto buffer = data();
			if (buffer == nullptr) [[unlikely]]
				return;
			Traits::assign(buffer[size()], _terminator);
		}

		constexpr bool addr_in_range(const_pointer ptr) const noexcept {
			if (std::is_constant_evaluated())
				return false;
			else
				return data() <= ptr && ptr <= data() + size();
		}

		template<typename F>
		constexpr void internal_replace_impl(const F& func, size_type pos, size_type oldcount, size_type count) {
			auto cap = capacity();
			auto sz = size();

			auto rsz = sz - oldcount + count;

			if (cap < rsz)
				grow_to(rsz);

			if (oldcount != count)
				Traits::move(data() + pos + count, data() + pos + oldcount, sz - pos - oldcount);

			func();

			set_size(rsz);
			null_terminate();
		}

		constexpr void internal_replace(size_type pos, const_pointer str, size_type oldcount, size_type count) {
			if (addr_in_range(str)) {
				basic_string rstr(str, count);
				internal_replace_impl([&]() { Traits::copy(data() + pos, rstr.data(), count); }, pos, oldcount, count);
			} else
				internal_replace_impl([&]() { Traits::copy(data() + pos, str, count); }, pos, oldcount, count);
		}

		constexpr void internal_replace(size_type pos, value_type ch, size_type oldcount, size_type count) {
			internal_replace_impl([&]() { Traits::assign(data() + pos, count, ch); }, pos, oldcount, count);
		}

		template<typename F>
		constexpr void internal_insert_impl(const F& func, size_type pos, size_type count) {
			if (count == 0) [[unlikely]]
				return;

			auto cap = capacity();
			auto sz = size();
			auto rsz = sz + count;

			if (cap < rsz)
				grow_to(rsz);

			Traits::move(data() + pos + count, data() + pos, sz - pos);
			func();

			set_size(rsz);
			null_terminate();
		}

		constexpr void internal_insert(size_type pos, const_pointer str, size_type count) {
			if (addr_in_range(str)) {
				basic_string rstr(str, count);
				internal_insert_impl([&]() { Traits::copy(data() + pos, rstr.data(), count); }, pos, count);
			} else
				internal_insert_impl([&]() { Traits::copy(data() + pos, str, count); }, pos, count);
		}

		constexpr void internal_insert(size_type pos, value_type ch, size_type count) {
			internal_insert_impl([&]() { Traits::assign(data() + pos, count, ch); }, pos, count);
		}

		template<typename F>
		constexpr void internal_append_impl(const F& func, size_type count) {
			if (count == 0) [[unlikely]]
				return;

			auto cap = capacity();
			auto sz = size();
			auto rsz = sz + count;

			if (cap < rsz)
				grow_to(rsz);

			func(sz);
			set_size(rsz);
			null_terminate();
		}

		constexpr void internal_append(const_pointer str, size_type count) {
			if (addr_in_range(str)) {
				basic_string rstr(str, count);
				internal_append_impl([&](size_type pos) { Traits::copy(data() + pos, rstr.data(), count); }, count);
			} else
				internal_append_impl([&](size_type pos) { Traits::copy(data() + pos, str, count); }, count);
		}

		constexpr void internal_append(value_type ch, size_type count) {
			internal_append_impl([&](size_type pos) { Traits::assign(data() + pos, count, ch); }, count);
		}

		template<typename F>
		constexpr void internal_assign_impl(const F& func, size_type size, bool copy_old) {
			if (fits_in_sso(size)) {
				if (is_long() == true) {
					deallocate();
					short_init();
				}

				set_short_size(size);
				func(get_short_data());
				null_terminate();
			} else {
				if (is_long() == false)
					long_init();
				if (get_long_cap() < size)
					reallocate(size, copy_old);

				func(get_long_data());
				set_long_size(size);
				null_terminate();
			}
		}

		constexpr void internal_assign(const_pointer str, size_type size, bool copy_old = false) {
			if (addr_in_range(str)) {
				basic_string rstr(str, size);
				internal_assign_impl([&](auto data) { Traits::copy(data, rstr.data(), size); }, size, copy_old);
			} else
				internal_assign_impl([&](auto data) { Traits::copy(data, str, size); }, size, copy_old);
		}

		constexpr void internal_assign(value_type ch, size_type count, bool copy_old = false) {
			internal_assign_impl([&](auto data) { Traits::assign(data, count, ch); }, count, copy_old);
		}

	public:
		explicit constexpr basic_string(detail::uninitialized_size_tag, size_type size, const Allocator& allocator)
			: _allocator(allocator) {
			PLUGIFY_ASSERT(size <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			if (fits_in_sso(size))
				short_init();
			else {
				long_init();
				reallocate(size, false);
			}
			set_size(size);
		}

		constexpr basic_string() noexcept(std::is_nothrow_default_constructible<Allocator>::value)
			: basic_string(Allocator()) {}

		explicit constexpr basic_string(const Allocator& allocator) noexcept
			: _allocator(allocator) {
			short_init();
		}

		constexpr basic_string(size_type count, value_type ch, const Allocator& allocator = Allocator())
			: _allocator(allocator) {
			PLUGIFY_ASSERT(count <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(ch, count);
		}

		constexpr basic_string(const basic_string& str, size_type pos, size_type count, const Allocator& allocator = Allocator())
			: _allocator(allocator) {
			PLUGIFY_ASSERT(pos <= str.size(), "plg::basic_string::basic_string(): pos out of range", std::out_of_range);
			auto len = std::min(count, str.size() - pos);
			PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(str.data() + pos, len);
		}
		constexpr basic_string(const basic_string& str, size_type pos, const Allocator& allocator = Allocator())
			: basic_string(str, pos, npos, allocator) {}

		constexpr basic_string(const value_type* str, size_type count, const Allocator& allocator = Allocator())
			: _allocator(allocator) {
			PLUGIFY_ASSERT(count <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(str, count);
		}

		constexpr basic_string(const value_type* str, const Allocator& allocator = Allocator())
			: basic_string(str, Traits::length(str), allocator) {}

		template<std::input_iterator InputIterator>
		constexpr basic_string(InputIterator first, InputIterator last, const Allocator& allocator = Allocator())
			: _allocator(allocator) {
			if constexpr (std::contiguous_iterator<InputIterator>) {
				auto len = size_type(std::distance(first, last));
				PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
				internal_assign(const_pointer(first), len);
			} else {
				if constexpr (std::random_access_iterator<InputIterator>) {
					auto len = size_type(std::distance(first, last));
					PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
					reserve(len);
				}
				for (auto it = first; it != last; ++it) {
					push_back(*it);
				}
			}
		}

		constexpr basic_string(const basic_string& str, const Allocator& allocator)
			: _allocator(allocator) {
			auto len = str.length();
			PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(str.data(), len);
		}
		constexpr basic_string(const basic_string& str)
			: basic_string(str, str.get_allocator()) {}

		constexpr basic_string(basic_string&& str) noexcept(std::is_nothrow_move_constructible<Allocator>::value)
			: _allocator(std::move(str._allocator)), _storage(std::move(str._storage)) {
			str.short_init();
		}

		constexpr basic_string(basic_string&& str, const Allocator& allocator)
			: _allocator(allocator) {
			if constexpr (allocator_traits::is_always_equal::value) {
				std::swap(_storage, str._storage);
			} else {
				if (!str.is_long() || get_allocator() == str.get_allocator()) {
					std::swap(_storage, str._storage);
				} else {
					internal_assign(str.data(), str.size());
					str.deallocate();
				}
			}
			str.short_init();
		}

		constexpr basic_string(std::initializer_list<value_type> list, const Allocator& allocator = Allocator())
			: _allocator(allocator) {
			auto len = list.size();
			PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(const_pointer(list.begin()), len);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type>)
		constexpr basic_string(const Type& t, size_type pos, size_type count, const Allocator& allocator = Allocator())
			: _allocator(allocator) {
			auto sv = sview_type(t);
			PLUGIFY_ASSERT(pos <= sv.length(), "plg::basic_string::basic_string(): pos out of range", std::out_of_range);
			auto ssv = sv.substr(pos, count);
			auto len = ssv.length();
			PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(ssv.data(), len);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string(const Type& t, const Allocator& allocator = Allocator())
			: _allocator(allocator) {
			sview_type sv(t);
			auto len = sv.length();
			PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(sv.data(), len);
		}

		constexpr basic_string(basic_string&& str, size_type pos, size_type count, const Allocator& allocator = Allocator())
			: basic_string(std::move(str), allocator) {
			PLUGIFY_ASSERT(pos <= str.size(), "plg::basic_string::basic_string(): pos out of range", std::out_of_range);
			erase(pos, count);
		}

		constexpr basic_string(basic_string&& str, size_type pos, const Allocator& allocator = Allocator())
			: basic_string(std::move(str), pos, npos, allocator) {}

#if __cplusplus > 202002L
		basic_string(std::nullptr_t) = delete;
#endif

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::string_compatible_range<Char> Range>
		constexpr basic_string(std::from_range_t, Range&& range, const Allocator& allocator = Allocator())
			: basic_string(std::ranges::begin(range), std::ranges::end(range), allocator) {}
#endif // PLUGIFY_STRING_CONTAINERS_RANGES

		constexpr ~basic_string() {
			deallocate();
		}

		constexpr basic_string& operator=(const basic_string& str) {
			return assign(str);
		}

		constexpr basic_string& operator=(basic_string&& str) noexcept(
				allocator_traits::propagate_on_container_move_assignment::value ||
				allocator_traits::is_always_equal::value) {
			return assign(std::move(str));
		}

		constexpr basic_string& operator=(const value_type* str) {
			return assign(str, Traits::length(str));
		}

		constexpr basic_string& operator=(value_type ch) {
			return assign(std::addressof(ch), 1);
		}

		constexpr basic_string& operator=(std::initializer_list<value_type> list) {
			return assign(list.begin(), list.size());
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& operator=(const Type& t) {
			sview_type sv(t);
			return assign(sv);
		}

#if __cplusplus > 202002L
		constexpr basic_string& operator=(std::nullptr_t) = delete;
#endif

		constexpr basic_string& assign(size_type count, value_type ch) {
			PLUGIFY_ASSERT(count <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			internal_assign(ch, count);
			return *this;
		}

		constexpr basic_string& assign(const basic_string& str, size_type pos, size_type count = npos) {
			PLUGIFY_ASSERT(pos <= str.size(), "plg::basic_string::assign(): pos out of range", std::out_of_range);
			internal_assign(str.data(), std::min(count, str.size() - pos));
			return *this;
		}

		constexpr basic_string& assign(const basic_string& str) {
			if (this == &str) [[unlikely]]
				return *this;

			if constexpr (allocator_traits::propagate_on_container_copy_assignment::value) {
				if constexpr (!allocator_traits::is_always_equal::value) {
					if (get_allocator() != str.get_allocator()) {
						deallocate();
						short_init();
					}
				}
				_allocator = str._allocator;
			}

			internal_assign(str.data(), str.size());
			return *this;
		}

		constexpr basic_string& assign(basic_string&& str) noexcept(
				allocator_traits::propagate_on_container_move_assignment::value ||
				allocator_traits::is_always_equal::value) {
			if (this == &str) [[unlikely]]
				return *this;

			if constexpr (allocator_traits::propagate_on_container_move_assignment::value) {
				if constexpr (!allocator_traits::is_always_equal::value) {
					if (get_allocator() != str.get_allocator()) {
						deallocate();
						short_init();
					}
				}
				_allocator = std::move(str._allocator);
			}

			if constexpr (allocator_traits::propagate_on_container_move_assignment::value || allocator_traits::is_always_equal::value) {
				deallocate();
				short_init();
				std::swap(_storage, str._storage);
			} else {
				if (get_allocator() == str.get_allocator()) {
					deallocate();
					short_init();
					std::swap(_storage, str._storage);
				} else {
					internal_assign(str.data(), str.size());
				}
			}

			return *this;
		}

		constexpr basic_string& assign(const value_type* str, size_type count) {
			PLUGIFY_ASSERT(count <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			internal_assign(str, count);
			return *this;
		}

		constexpr basic_string& assign(const value_type* str) {
			return assign(str, Traits::length(str));
		}

		template<std::input_iterator InputIterator>
		constexpr basic_string& assign(InputIterator first, InputIterator last) {
			if constexpr (std::contiguous_iterator<InputIterator>) {
				auto len = size_type(std::distance(first, last));
				PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
				internal_assign(const_pointer(first), len);
			} else {
				if constexpr (std::random_access_iterator<InputIterator>) {
					auto len = size_type(std::distance(first, last));
					PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
					reserve(len);
				}
				for (auto it = first; it != last; ++it) {
					push_back(*it);
				}
			}
			return *this;
		}

		constexpr basic_string& assign(std::initializer_list<value_type> list) {
			auto len = list.size();
			PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			internal_assign(const_pointer(list.begin()), len);
			return *this;
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& assign(const Type& t) {
			sview_type sv(t);
			return assign(sv.data(), sv.length());
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& assign(const Type& t, size_type pos, size_type count = npos) {
			auto sv = sview_type(t).substr(pos, count);
			auto len = sv.length();
			PLUGIFY_ASSERT(len <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			return assign(sv.data(), len);
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::string_compatible_range<Char> Range>
		constexpr basic_string& assign_range(Range&& range) {
			auto str = basic_string(std::from_range, std::forward<Range>(range), _allocator);
			PLUGIFY_ASSERT(str.size() <= max_size(), "plg::basic_string::assign_range(): resulted string size would exceed max_size()", std::length_error);
			return assign(std::move(str));
		}
#endif // PLUGIFY_STRING_CONTAINERS_RANGES

		constexpr allocator_type get_allocator() const noexcept {
			return _allocator;
		}

		constexpr reference operator[](size_type pos) {
			return data()[pos];
		}

		constexpr const_reference operator[](size_type pos) const {
			return data()[pos];
		}

		constexpr reference at(size_type pos) {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::at(): pos out of range", std::out_of_range);
			return data()[pos];
		}

		constexpr const_reference at(size_type pos) const {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::at(): pos out of range", std::out_of_range);
			return data()[pos];
		}

		constexpr reference front() {
			PLUGIFY_ASSERT(!empty(), "plg::basic_string::front(): vector is empty", std::length_error);
			return data()[0];
		}

		constexpr const_reference front() const {
			PLUGIFY_ASSERT(!empty(), "plg::basic_string::front(): vector is empty", std::length_error);
			return data()[0];
		}

		constexpr reference back() {
			PLUGIFY_ASSERT(!empty(), "plg::basic_string::back(): vector is empty", std::length_error);
			return data()[size() - 1];
		}

		constexpr const_reference back() const {
			PLUGIFY_ASSERT(!empty(), "plg::basic_string::back(): vector is empty", std::length_error);
			return data()[size() - 1];
		}

		constexpr const value_type* data() const noexcept {
			return is_long() ? get_long_data() : get_short_data();
		}

		constexpr value_type* data() noexcept {
			return is_long() ? get_long_data() : get_short_data();
		}

		constexpr const value_type* c_str() const noexcept {
			return data();
		}

		constexpr operator sview_type() const noexcept {
			return view();
		}

		constexpr iterator begin() noexcept {
			return data();
		}

		constexpr const_iterator begin() const noexcept {
			return data();
		}

		constexpr const_iterator cbegin() const noexcept {
			return data();
		}

		constexpr iterator end() noexcept {
			return data() + size();
		}

		constexpr const_iterator end() const noexcept {
			return data() + size();
		}

		constexpr const_iterator cend() const noexcept {
			return data() + size();
		}

		constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(cend());
		}

		constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(cbegin());
		}

		constexpr bool empty() const noexcept {
			return size() == 0;
		}

		constexpr size_type size() const noexcept {
			return is_long() ? get_long_size() : get_short_size();
		}

		constexpr size_type length() const noexcept {
			return size();
		}

		constexpr size_type max_size() const noexcept {
			// const size_type alignment = 16;
			// size_type m = allocator_traits::max_size(_allocator);
			// if (m <= std::numeric_limits<size_type>::max() / 2)
			//	 return m - alignment;
			// else
			//=	 return (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) ? m - alignment : (m / 2) - alignment;
			return (allocator_traits::max_size(_allocator) - 1) / 2;
		}

		constexpr size_type capacity() const noexcept {
			return is_long() ? get_long_cap() : min_cap;
		}

		constexpr void reserve(size_type cap) {
			PLUGIFY_ASSERT(cap <= max_size(), "plg::basic_string::reserve(): allocated memory size would exceed max_size()", std::length_error);
			if (cap <= capacity())
				return;

			auto new_cap = std::max(cap, size());
			if (new_cap == capacity())
				return;

			grow_to(new_cap);
		}

		void reserve() {
			shrink_to_fit();
		}

		constexpr void shrink_to_fit() {
			if (is_long() == false)
				return;

			reallocate(size(), true);
		}

		constexpr void clear() noexcept {
			set_size(0);
		}

		constexpr basic_string& insert(size_type pos, size_type count, value_type ch) {
			PLUGIFY_ASSERT(size() + count <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			insert(std::next(cbegin(), pos), count, ch);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const value_type* str) {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			auto len = Traits::length(str);
			PLUGIFY_ASSERT(size() + len <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			internal_insert(pos, str, len);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const value_type* str, size_type count) {
			PLUGIFY_ASSERT(size() + count <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			internal_insert(pos, str, count);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const basic_string& str) {
			PLUGIFY_ASSERT(size() + str.size() <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			internal_insert(pos, const_pointer(str.data()), str.size());
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const basic_string& str, size_type pos_str, size_type count = npos) {
			PLUGIFY_ASSERT(pos <= size() && pos_str <= str.size(), "plg::basic_string::insert(): pos or pos_str out of range", std::out_of_range);
			count = std::min(count, str.length() - pos_str);
			PLUGIFY_ASSERT(size() + count <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			return insert(pos, str.data() + pos_str, count);
		}

		constexpr iterator insert(const_iterator pos, value_type ch) {
			return insert(pos, 1, ch);
		}

		constexpr iterator insert(const_iterator pos, size_type count, value_type ch) {
			PLUGIFY_ASSERT(size() + count <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			auto spos = std::distance(cbegin(), pos);
			internal_insert(spos, ch, count);
			return std::next(begin(), spos);
		}

		template<std::input_iterator InputIterator>
		constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
			auto spos = std::distance(cbegin(), pos);
			auto len = static_cast<size_type>(std::distance(first, last));
			PLUGIFY_ASSERT(size() + len <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			internal_insert(spos, const_pointer(first), len);
			return std::next(begin(), spos);
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> list) {
			PLUGIFY_ASSERT(size() + list.size() <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			auto spos = std::distance(cbegin(), pos);
			internal_insert(spos, const_pointer(list.begin()), list.size());
			return std::next(begin(), spos);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& insert(size_type pos, const Type& t) {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			sview_type sv(t);
			PLUGIFY_ASSERT(size() + sv.length() <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			internal_insert(pos, const_pointer(sv.data()), sv.length());
			return *this;
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& insert(size_type pos, const Type& t, size_type pos_str, size_type count = npos) {
			auto sv = sview_type(t);
			PLUGIFY_ASSERT(pos <= size() && pos_str <= sv.length(), "plg::basic_string::insert(): pos or pos_str out of range", std::out_of_range);
			auto ssv = sv.substr(pos_str, count);
			PLUGIFY_ASSERT(size() + ssv.length() <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			internal_insert(pos, const_pointer(ssv.data()), ssv.length());
			return *this;
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::string_compatible_range<Char> Range>
		constexpr iterator insert_range(const_iterator pos, Range&& range) {
			auto str = basic_string(std::from_range, std::forward<Range>(range), _allocator);
			PLUGIFY_ASSERT(size() + str.size() <= max_size(), "plg::basic_string::insert_range(): resulted string size would exceed max_size()", std::length_error);
			return insert(pos - begin(), str);
		}
#endif // PLUGIFY_STRING_CONTAINERS_RANGES

		constexpr basic_string& erase(size_type pos = 0, size_type count = npos) {
			auto sz = size();
			auto buffer = data();

			PLUGIFY_ASSERT(pos <= sz, "plg::basic_string::erase(): pos out of range", std::out_of_range);

			count = std::min(count, sz - pos);

			auto left = sz - (pos + count);
			if (left != 0)
				Traits::move(buffer + pos, buffer + pos + count, left);

			auto new_size = pos + left;
			set_size(new_size);
			null_terminate();

			return *this;
		}

		constexpr iterator erase(const_iterator position) {
			auto pos = std::distance(cbegin(), position);
			erase(pos, 1);
			return begin() + pos;
		}

		constexpr iterator erase(const_iterator first, const_iterator last) {
			auto pos = std::distance(cbegin(), first);
			auto len = std::distance(first, last);
			erase(pos, len);
			return begin() + pos;
		}

		constexpr void push_back(value_type ch) {
			PLUGIFY_ASSERT(size() + 1 <= max_size(), "plg::basic_string::push_back(): resulted string size would exceed max_size()", std::length_error);
			append(1, ch);
		}

		constexpr void pop_back() {
			erase(end() - 1);
		}

		constexpr basic_string& append(size_type count, value_type ch) {
			PLUGIFY_ASSERT(size() + count <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(ch, count);
			return *this;
		}

		constexpr basic_string& append(const basic_string& str) {
			PLUGIFY_ASSERT(size() + str.size() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(str.data(), str.size());
			return *this;
		}

		constexpr basic_string& append(const basic_string& str, size_type pos, size_type count = npos) {
			PLUGIFY_ASSERT(pos <= str.size(), "plg::basic_string::append(): pos out of range", std::out_of_range);
			auto ssv = sview_type(str).substr(pos, count);
			PLUGIFY_ASSERT(size() + ssv.length() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(ssv.data(), ssv.length());
			return *this;
		}

		constexpr basic_string& append(const value_type* str, size_type count) {
			PLUGIFY_ASSERT(size() + count <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(str, count);
			return *this;
		}

		constexpr basic_string& append(const value_type* str) {
			auto len = Traits::length(str);
			PLUGIFY_ASSERT(size() + len <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			return append(str, len);
		}

		template<std::input_iterator InputIterator>
		constexpr basic_string& append(InputIterator first, InputIterator last) {
			auto len = static_cast<size_type>(std::distance(first, last));
			PLUGIFY_ASSERT(size() + len <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(const_pointer(first), len);
			return *this;
		}

		constexpr basic_string& append(std::initializer_list<value_type> list) {
			PLUGIFY_ASSERT(size() + list.size() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(const_pointer(list.begin()), list.size());
			return *this;
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& append(const Type& t) {
			sview_type sv(t);
			PLUGIFY_ASSERT(size() + sv.length() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(sv.data(), sv.size());
			return *this;
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& append(const Type& t, size_type pos, size_type count = npos) {
			sview_type sv(t);
			PLUGIFY_ASSERT(pos <= sv.length(), "plg::basic_string::append(): pos out of range", std::out_of_range);
			auto ssv = sv.substr(pos, count);
			PLUGIFY_ASSERT(size() + ssv.length() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(ssv.data(), ssv.length());
			return *this;
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::string_compatible_range<Char> Range>
		constexpr basic_string& append_range(Range&& range) {
			auto str = basic_string(std::from_range, std::forward<Range>(range), _allocator);
			PLUGIFY_ASSERT(size() + str.size() <= max_size(), "plg::basic_string::insert_range(): resulted string size would exceed max_size()", std::length_error);
			return append(str);
		}
#endif // PLUGIFY_STRING_CONTAINERS_RANGES

		constexpr basic_string& operator+=(const basic_string& str) {
			return append(str);
		}

		constexpr basic_string& operator+=(value_type ch) {
			push_back(ch);
			return *this;
		}

		constexpr basic_string& operator+=(const value_type* str) {
			return append(str);
		}

		constexpr basic_string& operator+=(std::initializer_list<value_type> list) {
			return append(list);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& operator+=(const Type& t) {
			return append(sview_type(t));
		}

		constexpr int compare(const basic_string& str) const noexcept {
			return view().compare(str.view());
		}

		constexpr int compare(size_type pos1, size_type count1, const basic_string& str) const {
			return view().compare(pos1, count1, str.view());
		}

		constexpr int compare(size_type pos1, size_type count1, const basic_string& str, size_type pos2, size_type count2 = npos) const {
			return view().compare(pos1, count1, str.view(), pos2, count2);
		}

		constexpr int compare(const value_type* str) const {
			return view().compare(str);
		}

		constexpr int compare(size_type pos1, size_type count1, const value_type* str) const {
			return view().compare(pos1, count1, str);
		}

		constexpr int compare(size_type pos1, size_type count1, const value_type* str, size_type count2) const {
			return view().compare(pos1, count1, str, count2);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr int compare(const Type& t) const noexcept(noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>)) {
			return view().compare(sview_type(t));
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr int compare(size_type pos1, size_type count1, const Type& t) const {
			return view().compare(pos1, count1, sview_type(t));
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr int compare(size_type pos1, size_type count1, const Type& t, size_type pos2, size_type count2 = npos) const {
			return view().compare(pos1, count1, sview_type(t), pos2, count2);
		}

		constexpr bool starts_with(sview_type sv) const noexcept {
			return view().starts_with(sv);
		}

		constexpr bool starts_with(Char ch) const noexcept {
			return view().starts_with(ch);
		}

		constexpr bool starts_with(const Char* str) const {
			return view().starts_with(str);
		}

		constexpr bool ends_with(sview_type sv) const noexcept {
			return view().ends_with(sv);
		}

		constexpr bool ends_with(Char ch) const noexcept {
			return view().ends_with(ch);
		}

		constexpr bool ends_with(const Char* str) const {
			return view().ends_with(str);
		}

		constexpr bool contains(sview_type sv) const noexcept {
			return view().contains(sv);
		}

		constexpr bool contains(Char ch) const noexcept {
			return view().contains(ch);
		}

		constexpr bool contains(const Char* str) const {
			return view().contains(str);
		}

		constexpr basic_string& replace(size_type pos, size_type count, const basic_string& str) {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			return replace(pos, count, str, 0, str.length());
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const basic_string& str) {
			auto pos = std::distance(cbegin(), first);
			auto count = std::distance(first, last);
			return replace(pos, count, str, 0, str.length());
		}

		constexpr basic_string& replace(size_type pos, size_type count, const basic_string& str, size_type pos2, size_type count2 = npos) {
			PLUGIFY_ASSERT(pos <= size() && pos2 <= str.size(), "plg::basic_string::replace(): pos or pos_str out of range", std::out_of_range);
			count2 = std::min(count2, str.length() - pos2);
			auto ssv = sview_type(str).substr(pos2, count2);
			return replace(pos, count, ssv.data(), ssv.length());
		}

		template<std::input_iterator InputIterator>
		constexpr basic_string& replace(const_iterator first, const_iterator last, InputIterator first2, InputIterator last2) {
			return replace(first, last, const_pointer(first2), std::distance(first2, last2));
		}

		constexpr basic_string& replace(size_type pos, size_type count, const value_type* str, size_type count2) {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			count = std::min(count, length() - pos);
			PLUGIFY_ASSERT(size() - count + count2 <= max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			internal_replace(pos, const_pointer(str), count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const value_type* str, size_type count2) {
			auto pos = std::distance(cbegin(), first);
			auto count = std::distance(first, last);
			return replace(pos, count, str, count2);
		}

		constexpr basic_string& replace(size_type pos, size_type count, const value_type* str) {
			return replace(pos, count, str, Traits::length(str));
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const value_type* str) {
			return replace(first, last, str, Traits::length(str));
		}

		constexpr basic_string& replace(size_type pos, size_type count, size_type count2, value_type ch) {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			count = std::min(count, length() - pos);
			PLUGIFY_ASSERT(size() - count + count2 <= max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			internal_replace(pos, ch, count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, size_type count2, value_type ch) {
			auto pos = std::distance(cbegin(), first);
			auto count = std::distance(first, last);

			PLUGIFY_ASSERT(size() - count + count2 <= max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			internal_replace(pos, ch, count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, std::initializer_list<value_type> list) {
			return replace(first, last, const_pointer(list.begin()), list.size());
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(size_type pos, size_type count, const Type& t) {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			sview_type sv(t);
			return replace(pos, count, sv.data(), sv.length());
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(const_iterator first, const_iterator last, const Type& t) {
			sview_type sv(t);
			return replace(first, last, sv.data(), sv.length());
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(size_type pos, size_type count, const Type& t, size_type pos2, size_type count2 = npos) {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			auto sv = sview_type(t).substr(pos2, count2);
			return replace(pos, count, sv.data(), sv.length());
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::string_compatible_range<Char> Range>
		constexpr iterator replace_with_range(const_iterator first, const_iterator last, Range&& range) {
			auto str = basic_string(std::from_range, std::forward<Range>(range), _allocator);
			return replace(first, last, str);// replace checks for max_size()
		}
#endif // PLUGIFY_STRING_CONTAINERS_RANGES

		constexpr basic_string substr(size_type pos = 0, size_type count = npos) const {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::substr(): pos out of range", std::out_of_range);
			return basic_string(*this, pos, count);
		}

		constexpr size_type copy(value_type* str, size_type count, size_type pos = 0) const {
			PLUGIFY_ASSERT(pos <= size(), "plg::basic_string::copy(): pos out of range", std::out_of_range);
			return view().copy(str, count, pos);
		}

		constexpr void resize(size_type count, value_type ch) {
			PLUGIFY_ASSERT(size() + count <= max_size(), "plg::basic_string::resize(): resulted string size would exceed max_size()", std::length_error);
			auto cap = capacity();
			auto sz = size();
			auto rsz = count + sz;

			if (sz < rsz) {
				if (cap < rsz)
					grow_to(rsz);
				Traits::assign(data() + sz, count, ch);
			}
			set_size(rsz);
			null_terminate();
		}

		constexpr void resize(size_type count) {
			resize(count, _terminator);
		}

		template<typename Operation>
		constexpr void resize_and_overwrite(size_type, Operation) {
			static_assert(detail::dependent_false<Char>, "plg::basic_string::resize_and_overwrite(count, op) not implemented!");
		}

		constexpr void swap(basic_string& other) noexcept(allocator_traits::propagate_on_container_swap::value || allocator_traits::is_always_equal::value) {
			using std::swap;
			if constexpr (allocator_traits::propagate_on_container_swap::value) {
				swap(_allocator, other._allocator);
			}
			swap(_storage, other._storage);
		}

		constexpr size_type find(const basic_string& str, size_type pos = 0) const noexcept {
			return view().find(sview_type(str), pos);
		}

		constexpr size_type find(const value_type* str, size_type pos, size_type count) const noexcept {
			return view().find(str, pos, count);
		}

		constexpr size_type find(const value_type* str, size_type pos = 0) const noexcept {
			return view().find(str, pos);
		}

		constexpr size_type find(value_type ch, size_type pos = 0) const noexcept {
			return view().find(ch, pos);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return view().find(sview_type(t), pos);
		}

		constexpr size_type rfind(const basic_string& str, size_type pos = npos) const noexcept {
			return view().rfind(sview_type(str), pos);
		}

		constexpr size_type rfind(const value_type* str, size_type pos, size_type count) const noexcept {
			return view().rfind(str, pos, count);
		}

		constexpr size_type rfind(const value_type* str, size_type pos = npos) const noexcept {
			return view().rfind(str, pos);
		}

		constexpr size_type rfind(value_type ch, size_type pos = npos) const noexcept {
			return view().rfind(ch, pos);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type rfind(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return view().rfind(sview_type(t), pos);
		}

		constexpr size_type find_first_of(const basic_string& str, size_type pos = 0) const noexcept {
			return view().find_first_of(sview_type(str), pos);
		}

		constexpr size_type find_first_of(const value_type* str, size_type pos, size_type count) const noexcept {
			return view().find_first_of(str, pos, count);
		}

		constexpr size_type find_first_of(const value_type* str, size_type pos = 0) const noexcept {
			return view().find_first_of(str, pos);
		}

		constexpr size_type find_first_of(value_type ch, size_type pos = 0) const noexcept {
			return view().find_first_of(ch, pos);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find_first_of(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return view().find_first_of(sview_type(t), pos);
		}

		constexpr size_type find_first_not_of(const basic_string& str, size_type pos = 0) const noexcept {
			return view().find_first_not_of(sview_type(str), pos);
		}

		constexpr size_type find_first_not_of(const value_type* str, size_type pos, size_type count) const noexcept {
			return view().find_first_not_of(str, pos, count);
		}

		constexpr size_type find_first_not_of(const value_type* str, size_type pos = 0) const noexcept {
			return view().find_first_not_of(str, pos);
		}

		constexpr size_type find_first_not_of(value_type ch, size_type pos = 0) const noexcept {
			return view().find_first_not_of(ch, pos);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find_first_not_of(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return view().find_first_not_of(sview_type(t), pos);
		}

		constexpr size_type find_last_of(const basic_string& str, size_type pos = npos) const noexcept {
			return view().find_last_of(sview_type(str), pos);
		}

		constexpr size_type find_last_of(const value_type* str, size_type pos, size_type count) const noexcept {
			return view().find_last_of(str, pos, count);
		}

		constexpr size_type find_last_of(const value_type* str, size_type pos = npos) const noexcept {
			return view().find_last_of(str, pos);
		}

		constexpr size_type find_last_of(value_type ch, size_type pos = npos) const noexcept {
			return view().find_last_of(ch, pos);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find_last_of(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return view().find_last_of(sview_type(t), pos);
		}

		constexpr size_type find_last_not_of(const basic_string& str, size_type pos = npos) const noexcept {
			return view().find_last_not_of(sview_type(str), pos);
		}

		constexpr size_type find_last_not_of(const value_type* str, size_type pos, size_type count) const noexcept {
			return view().find_last_not_of(str, pos, count);
		}

		constexpr size_type find_last_not_of(const value_type* str, size_type pos = npos) const noexcept {
			return view().find_last_not_of(str, pos);
		}

		constexpr size_type find_last_not_of(value_type ch, size_type pos = npos) const noexcept {
			return view().find_last_not_of(ch, pos);
		}

		template<typename Type>
			requires (std::is_convertible_v<const Type&, sview_type> &&
					 !std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find_last_not_of(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return view().find_last_not_of(sview_type(t), pos);
		}

		friend constexpr basic_string operator+(const basic_string& lhs, const basic_string& rhs) {
			auto lhs_sz = lhs.size();
			auto rhs_sz = rhs.size();
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::allocator_traits::select_on_container_copy_construction(lhs._allocator));
			auto buffer = ret.data();
			Traits::copy(buffer, lhs.data(), lhs_sz);
			Traits::copy(buffer + lhs_sz, rhs.data(), rhs_sz);
			ret.null_terminate();
			return ret;
		}

		friend constexpr basic_string operator+(basic_string&& lhs, const basic_string& rhs) {
			return std::move(lhs.append(rhs));
		}

		friend constexpr basic_string operator+(const basic_string& lhs, basic_string&& rhs) {
			return std::move(rhs.insert(0, lhs));
		}

		friend constexpr basic_string operator+(basic_string&& lhs, basic_string&& rhs) {
			return std::move(lhs.append(rhs));
		}

		friend constexpr basic_string operator+(const Char* lhs, const basic_string& rhs) {
			auto lhs_sz = Traits::length(lhs);
			auto rhs_sz = rhs.size();
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::allocator_traits::select_on_container_copy_construction(rhs._allocator));
			auto buffer = ret.data();
			Traits::copy(buffer, lhs, lhs_sz);
			Traits::copy(buffer + lhs_sz, rhs.data(), rhs_sz);
			ret.null_terminate();
			return ret;
		}

		friend constexpr basic_string operator+(const Char* lhs, basic_string&& rhs) {
			return std::move(rhs.insert(0, lhs));
		}

		friend constexpr basic_string operator+(Char lhs, const basic_string& rhs) {
			auto rhs_sz = rhs.size();
			basic_string ret(detail::uninitialized_size_tag(), rhs_sz + 1, basic_string::allocator_traits::select_on_container_copy_construction(rhs._allocator));
			auto buffer = ret.data();
			Traits::assign(buffer, 1, lhs);
			Traits::copy(buffer + 1, rhs.data(), rhs_sz);
			ret.null_terminate();
			return ret;
		}

		friend constexpr basic_string operator+(Char lhs, basic_string&& rhs) {
			rhs.insert(rhs.begin(), lhs);
			return std::move(rhs);
		}

		friend constexpr basic_string operator+(const basic_string& lhs, const Char* rhs) {
			auto lhs_sz = lhs.size();
			auto rhs_sz = Traits::length(rhs);
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::allocator_traits::select_on_container_copy_construction(lhs._allocator));
			auto buffer = ret.data();
			Traits::copy(buffer, lhs.data(), lhs_sz);
			Traits::copy(buffer + lhs_sz, rhs, rhs_sz);
			ret.null_terminate();
			return ret;
		}

		friend constexpr basic_string operator+(basic_string&& lhs, const Char* rhs) {
			return std::move(lhs.append(rhs));
		}

		friend constexpr basic_string operator+(const basic_string& lhs, Char rhs) {
			auto lhs_sz = lhs.size();
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + 1, basic_string::allocator_traits::select_on_container_copy_construction(lhs._allocator));
			auto buffer = ret.data();
			Traits::copy(buffer, lhs.data(), lhs_sz);
			Traits::assign(buffer + lhs_sz, 1, rhs);
			ret.null_terminate();
			return ret;
		}

		friend constexpr basic_string operator+(basic_string&& lhs, Char rhs) {
			lhs.push_back(rhs);
			return std::move(lhs);
		}
	};

	template<typename Char, typename Traits, typename Allocator>
	constexpr bool operator==(const basic_string<Char, Traits, Allocator>& lhs, const basic_string<Char, Traits, Allocator>& rhs) noexcept {
		return lhs.compare(rhs) == 0;
	}

	template<typename Char, typename Traits, typename Allocator>
	constexpr bool operator==(const basic_string<Char, Traits, Allocator>& lhs, const Char* rhs) {
		return lhs.compare(rhs) == 0;
	}

	template<typename Char, typename Traits, typename Allocator>
	constexpr std::strong_ordering operator<=>(const basic_string<Char, Traits, Allocator>& lhs, const basic_string<Char, Traits, Allocator>& rhs) noexcept {
		return lhs.compare(rhs) <=> 0;
	}

	template<typename Char, typename Traits, typename Allocator>
	constexpr std::strong_ordering operator<=>(const basic_string<Char, Traits, Allocator>& lhs, const Char* rhs) {
		return lhs.compare(rhs) <=> 0;
	}

	// swap
	template<typename Char, typename Traits, typename Allocator>
	constexpr void swap(basic_string<Char, Traits, Allocator>& lhs, basic_string<Char, Traits, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
		lhs.swap(rhs);
	}

	// erasure
	template<typename Char, typename Traits, typename Allocator, typename U>
	constexpr typename basic_string<Char, Traits, Allocator>::size_type erase(basic_string<Char, Traits, Allocator>& c, const U& value) {
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	template<typename Char, typename Traits, typename Allocator, typename Pred>
	constexpr typename basic_string<Char, Traits, Allocator>::size_type erase_if(basic_string<Char, Traits, Allocator>& c, Pred pred) {
		auto it = std::remove_if(c.begin(), c.end(), pred);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	// deduction guides
	template<typename InputIterator, typename Allocator = plg::allocator<typename std::iterator_traits<InputIterator>::value_type>>
	basic_string(InputIterator, InputIterator, Allocator = Allocator()) -> basic_string<typename std::iterator_traits<InputIterator>::value_type, std::char_traits<typename std::iterator_traits<InputIterator>::value_type>, Allocator>;

	template<typename Char, typename Traits, typename Allocator = plg::allocator<Char>>
	explicit basic_string(std::basic_string_view<Char, Traits>, const Allocator& = Allocator()) -> basic_string<Char, Traits, Allocator>;

	template<typename Char, typename Traits, typename Allocator = plg::allocator<Char>>
	basic_string(std::basic_string_view<Char, Traits>, typename basic_string<Char, Traits, Allocator>::size_type, typename basic_string<Char, Traits, Allocator>::size_type, const Allocator& = Allocator()) -> basic_string<Char, Traits, Allocator>;

#if PLUGIFY_STRING_CONTAINERS_RANGES
	template<std::ranges::input_range Range, typename Allocator = plg::allocator<std::ranges::range_value_t<Range>>>
	basic_string(std::from_range_t, Range&&, Allocator = Allocator()) -> basic_string<std::ranges::range_value_t<Range>, std::char_traits<std::ranges::range_value_t<Range>>, Allocator>;
#endif // PLUGIFY_STRING_CONTAINERS_RANGES

	// basic_string typedef-names
	using string = basic_string<char>;
	using u8string = basic_string<char8_t>;
	using u16string = basic_string<char16_t>;
	using u32string = basic_string<char32_t>;
	using wstring = basic_string<wchar_t>;

#ifndef PLUGIFY_STRING_NO_NUMERIC_CONVERSIONS
	// numeric conversions
	inline int stoi(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtol(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return static_cast<int>(ret);
	}

	inline long stol(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtol(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	inline long long stoll(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoll(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	inline unsigned long stoul(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoul(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	inline unsigned long long stoull(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoull(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	inline float stof(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtof(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	inline double stod(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtod(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	inline long double stold(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtold(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	namespace detail {
		template<typename S, typename V>
		PLUGIFY_FORCE_INLINE constexpr S to_string(V v) {
			//  numeric_limits::digits10 returns value less on 1 than desired for unsigned numbers.
			//  For example, for 1-byte unsigned value digits10 is 2 (999 can not be represented),
			//  so we need +1 here.
			constexpr std::size_t bufSize = std::numeric_limits<V>::digits10 + 2; // +1 for minus, +1 for digits10
			char buf[bufSize];
			const auto res = std::to_chars(buf, buf + bufSize, v);
			return S(buf, res.ptr);
		}

		typedef int (*wide_printf)(wchar_t* __restrict, std::size_t, const wchar_t* __restrict, ...);

#if PLUGIFY_COMPILER_MSVC
		inline int truncate_snwprintf(wchar_t* __restrict buffer, std::size_t count, const wchar_t* __restrict format, ...) {
			int r;
			va_list args;
			va_start(args, format);
			r = _vsnwprintf_s(buffer, count, _TRUNCATE, format, args);
			va_end(args);
			return r;
		}
#endif

		PLUGIFY_FORCE_INLINE constexpr wide_printf get_swprintf() noexcept {
#if PLUGIFY_COMPILER_MSVC
			return static_cast<int(__cdecl*)(wchar_t* __restrict, std::size_t, const wchar_t* __restrict, ...)>(truncate_snwprintf);
#else
			return swprintf;
#endif
		}

		template<typename S, typename P, typename V>
		PLUGIFY_FORCE_INLINE constexpr S as_string(P sprintf_like, const typename S::value_type* fmt, V v) {
			typedef typename S::size_type size_type;
			S s;
			s.resize(s.capacity());
			size_type available = s.size();
			while (true) {
				int status = sprintf_like(&s[0], available + 1, fmt, v);
				if (status >= 0) {
					auto used = static_cast<size_type>(status);
					if (used <= available) {
						s.resize(used);
						break;
					}
					available = used; // Assume this is advice of how much space we need.
				} else {
					available = available * 2 + 1;
				}
				s.resize(available);
			}
			return s;
		}
	}// namespace detail

	inline string to_string(int val) { return detail::to_string<string>(val); }
	inline string to_string(unsigned val) { return detail::to_string<string>(val); }
	inline string to_string(long val) { return detail::to_string<string>(val); }
	inline string to_string(unsigned long val) { return detail::to_string<string>(val); }
	inline string to_string(long long val) { return detail::to_string<string>(val); }
	inline string to_string(unsigned long long val) { return detail::to_string<string>(val); }
	inline string to_string(float val) { return detail::as_string<string>(snprintf, "%f", val); }
	inline string to_string(double val) { return detail::as_string<string>(snprintf, "%f", val); }
	inline string to_string(long double val) { return detail::as_string<string>(snprintf, "%Lf", val); }

	inline wstring to_wstring(int val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(long long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned long long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(float val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%f", val); }
	inline wstring to_wstring(double val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%f", val); }
	inline wstring to_wstring(long double val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%Lf", val); }
#endif // PLUGIFY_STRING_NO_NUMERIC_CONVERSIONS

#ifndef PLUGIFY_STRING_NO_STD_HASH
	// hash support
	namespace detail {
		template<typename Char, typename Allocator, typename String = basic_string<Char, std::char_traits<Char>, Allocator>>
		struct string_hash_base {
			constexpr std::size_t operator()(const String& str) const noexcept {
				return std::hash<typename String::sview_type>{}(typename String::sview_type(str));
			}
		};
	}// namespace detail
#endif // PLUGIFY_STRING_NO_STD_HASH

#ifndef PLUGIFY_STRING_NO_STD_FORMAT
	// format support
	namespace detail {
		template<typename Char>
		static constexpr const Char* format_string() {
			if constexpr (std::is_same_v<Char, char> || std::is_same_v<Char, char8_t>)
				return "{}";
			if constexpr (std::is_same_v<Char, wchar_t>)
				return L"{}";
			if constexpr (std::is_same_v<Char, char16_t>)
				return u"{}";
			if constexpr (std::is_same_v<Char, char32_t>)
				return U"{}";
			return "";
		}

		template<typename Char, typename Allocator, typename String = basic_string<Char, std::char_traits<Char>, Allocator>>
		struct string_formatter_base {
			constexpr auto parse(std::format_parse_context& ctx) {
				return ctx.begin();
			}

			template<class FormatContext>
			auto format(const String& str, FormatContext& ctx) const {
				return std::format_to(ctx.out(), format_string<Char>(), str.c_str());
			}
		};
	}
#endif // PLUGIFY_STRING_NO_STD_FORMAT

	inline namespace literals {
		inline namespace string_literals {
			PLUGIFY_WARN_PUSH()

#if PLUGIFY_COMPILER_CLANG
			PLUGIFY_WARN_IGNORE("-Wuser-defined-literals")
#elif PLUGIFY_COMPILER_GCC
			PLUGIFY_WARN_IGNORE("-Wliteral-suffix")
#elif PLUGIFY_COMPILER_MSVC
			PLUGIFY_WARN_IGNORE(4455)
#endif
			// suffix for basic_string literals
			constexpr string operator""s(const char* str, std::size_t len) { return string{str, len}; }
			constexpr u8string operator""s(const char8_t* str, std::size_t len) { return u8string{str, len}; }
			constexpr u16string operator""s(const char16_t* str, std::size_t len) { return u16string{str, len}; }
			constexpr u32string operator""s(const char32_t* str, std::size_t len) { return u32string{str, len}; }
			constexpr wstring operator""s(const wchar_t* str, std::size_t len) { return wstring{str, len}; }

			PLUGIFY_WARN_POP()
		}// namespace string_literals
	}// namespace literals
}// namespace plg

#ifndef PLUGIFY_STRING_NO_STD_HASH
// hash support
namespace std {
	template<typename Allocator>
	struct hash<plg::basic_string<char, std::char_traits<char>, Allocator>> : plg::detail::string_hash_base<char, Allocator> {};

	template<typename Allocator>
	struct hash<plg::basic_string<char8_t, std::char_traits<char8_t>, Allocator>> : plg::detail::string_hash_base<char8_t, Allocator> {};

	template<typename Allocator>
	struct hash<plg::basic_string<char16_t, std::char_traits<char16_t>, Allocator>> : plg::detail::string_hash_base<char16_t, Allocator> {};

	template<typename Allocator>
	struct hash<plg::basic_string<char32_t, std::char_traits<char32_t>, Allocator>> : plg::detail::string_hash_base<char32_t, Allocator> {};

	template<typename Allocator>
	struct hash<plg::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>> : plg::detail::string_hash_base<wchar_t, Allocator> {};
}// namespace std
#endif // PLUGIFY_STRING_NO_STD_HASH

#ifndef PLUGIFY_STRING_NO_STD_FORMAT
// format support
#ifdef FMT_HEADER_ONLY
namespace fmt {
#else
namespace std {
#endif
	template<typename Allocator>
	struct formatter<plg::basic_string<char, std::char_traits<char>, Allocator>> : plg::detail::string_formatter_base<char, Allocator> {};

	template<typename Allocator>
	struct formatter<plg::basic_string<char8_t, std::char_traits<char8_t>, Allocator>> : plg::detail::string_formatter_base<char8_t, Allocator> {};

	template<typename Allocator>
	struct formatter<plg::basic_string<char16_t, std::char_traits<char16_t>, Allocator>> : plg::detail::string_formatter_base<char16_t, Allocator> {};

	template<typename Allocator>
	struct formatter<plg::basic_string<char32_t, std::char_traits<char32_t>, Allocator>> : plg::detail::string_formatter_base<char32_t, Allocator> {};

	template<typename Allocator>
	struct formatter<plg::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>> : plg::detail::string_formatter_base<wchar_t, Allocator> {};
}// namespace std
#endif // PLUGIFY_STRING_NO_STD_FORMAT

template<typename Char, typename Traits, typename Alloc>
std::ostream& operator<<(std::ostream& os, const plg::basic_string<Char, Traits, Alloc>& str) {
	os << str.c_str();
	return os;
}

#ifndef PLUGIFY_STRING_NO_STD_FORMAT
#include <functional>

namespace plg {
	namespace detail {
		// Concept to match string-like types including char* and const char*
		template<typename T>
		concept is_string_like = requires(T v) {
			{ std::string_view(v) };
		};
	}

	template<typename Range>
	constexpr string join(const Range& range, std::string_view separator) {
		string result;

		auto it = range.cbegin();
		auto end = range.cend();

		if (it == end) return result;

		// First pass: compute total size
		size_t total_size = 0;
		size_t count = 0;

		for (auto tmp = it; tmp != end; ++tmp) {
			using Elem = std::decay_t<decltype(*tmp)>;
			if constexpr (detail::is_string_like<Elem>) {
				total_size += std::string_view(*tmp).size();
			} else {
				total_size += std::formatted_size("{}", *tmp);
			}
			++count;
		}
		if (count > 1) {
			total_size += (count - 1) * separator.size();
		}
		result.reserve(total_size);

		auto in = std::back_inserter(result);

		// Second pass: actual formatting
		/*if (it != end)*/ {
			std::format_to(in, "{}", *it++);
		}
		while (it != end) {
			std::format_to(in, "{}{}", separator, *it++);
		}

		return result;
	}

	template<typename Range, typename Proj>
	constexpr string join(const Range& range, Proj&& proj, std::string_view separator) {
		string result;

		auto it = range.cbegin();
		auto end = range.cend();

		if (it == end) return result;

		// First pass: compute total size
		size_t total_size = 0;
		size_t count = 0;

		for (auto tmp = it; tmp != end; ++tmp) {
			auto&& projected = std::invoke(std::forward<Proj>(proj), *tmp);
			using Elem = std::decay_t<decltype(projected)>;

			if constexpr (detail::is_string_like<Elem>) {
				total_size += std::string_view(*projected).size();
			} else {
				total_size += std::formatted_size("{}", projected);
			}
			++count;
		}
		if (count > 1) {
			total_size += (count - 1) * separator.size();
		}
		result.reserve(total_size);

		auto out = std::back_inserter(result);

		// Second pass: actual formatting
		{
			auto&& projected = std::invoke(std::forward<Proj>(proj), *it++);
			std::format_to(out, "{}", projected);
		}
		while (it != end) {
			auto&& projected = std::invoke(std::forward<Proj>(proj), *it++);
			std::format_to(out, "{}{}", separator, projected);
		}

		return result;
	}
} // namespace plugify
#endif // PLUGIFY_STRING_NO_STD_FORMAT