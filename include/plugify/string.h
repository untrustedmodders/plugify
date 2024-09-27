#pragma once

// Just in case, because we can't ignore some warnings from `-Wpedantic` (about zero size arrays and anonymous structs when gnu extensions are disabled) on gcc
#if defined(__clang__)
#  pragma clang system_header
#elif defined(__GNUC__)
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

#ifndef PLUGIFY_STRING_STD_HASH
#  define PLUGIFY_STRING_STD_HASH 1
#endif

#ifndef PLUGIFY_STRING_STD_FORMAT
#  define PLUGIFY_STRING_STD_FORMAT 1
#  include <plugify/compat_format.h>
#endif

#ifndef PLUGIFY_STRING_NUMERIC_CONVERSIONS
#  define PLUGIFY_STRING_NUMERIC_CONVERSIONS 1
#endif

#if PLUGIFY_STRING_NUMERIC_CONVERSIONS && !__has_include(<cstdlib>)
#  undef PLUGIFY_STRING_NUMERIC_CONVERSIONS
#  define PLUGIFY_STRING_NUMERIC_CONVERSIONS 0
#endif

#if PLUGIFY_STRING_NUMERIC_CONVERSIONS
#  include <cstdlib>
#endif

#ifndef PLUGIFY_STRING_FLOAT
#  define PLUGIFY_STRING_FLOAT 1
#endif

#if PLUGIFY_STRING_FLOAT && !PLUGIFY_STRING_NUMERIC_CONVERSIONS
#  undef PLUGIFY_STRING_FLOAT
#  define PLUGIFY_STRING_FLOAT 0
#endif

#ifndef PLUGIFY_STRING_LONG_DOUBLE
#  define PLUGIFY_STRING_LONG_DOUBLE 1
#endif

#if PLUGIFY_STRING_LONG_DOUBLE && !PLUGIFY_STRING_FLOAT
#  undef PLUGIFY_STRING_LONG_DOUBLE
#  define PLUGIFY_STRING_LONG_DOUBLE 0
#endif

#ifndef PLUGIFY_STRING_CONTAINERS_RANGES
#  define PLUGIFY_STRING_CONTAINERS_RANGES 1
#endif

#if PLUGIFY_STRING_CONTAINERS_RANGES && (__cplusplus <= 202002L || !__has_include(<ranges>) || !defined(__cpp_lib_containers_ranges))
#  undef PLUGIFY_STRING_CONTAINERS_RANGES
#  define PLUGIFY_STRING_CONTAINERS_RANGES 0
#endif

#if PLUGIFY_STRING_CONTAINERS_RANGES
#  include <ranges>
#endif

#ifndef PLUGIFY_STRING_EXCEPTIONS
#  if __cpp_exceptions
#    define PLUGIFY_STRING_EXCEPTIONS 1
#  else
#    define PLUGIFY_STRING_EXCEPTIONS 0
#  endif
#endif

#if PLUGIFY_STRING_EXCEPTIONS && (!__cpp_exceptions || !__has_include(<stdexcept>))
#  undef PLUGIFY_STRING_EXCEPTIONS
#  define PLUGIFY_STRING_EXCEPTIONS 0
#endif

#ifndef PLUGIFY_STRING_FALLBACK_ASSERT
#  define PLUGIFY_STRING_FALLBACK_ASSERT 1
#endif

#if PLUGIFY_STRING_FALLBACK_ASSERT && !__has_include(<cassert>)
#  undef PLUGIFY_STRING_FALLBACK_ASSERT
#  define PLUGIFY_STRING_FALLBACK_ASSERT 0
#endif

#ifndef PLUGIFY_STRING_FALLBACK_ABORT
#  define PLUGIFY_STRING_FALLBACK_ABORT 1
#endif

#if PLUGIFY_STRING_FALLBACK_ABORT && !__has_include(<cstdlib>)
#  undef PLUGIFY_STRING_FALLBACK_ABORT
#  define PLUGIFY_STRING_FALLBACK_ABORT 0
#endif

#ifndef PLUGIFY_STRING_FALLBACK_ABORT_FUNCTION
#  define PLUGIFY_STRING_FALLBACK_ABORT_FUNCTION [] (auto) { }
#endif

#if PLUGIFY_STRING_EXCEPTIONS
#  include <stdexcept>
#  define _PLUGIFY_STRING_ASSERT(x, str, e) do { if (!(x)) throw e(str); } while (0)
#elif PLUGIFY_STRING_FALLBACK_ASSERT
#  include <cassert>
#  define _PLUGIFY_STRING_ASSERT(x, str, ...) assert(x && str)
#elif PLUGIFY_STRING_FALLBACK_ABORT
#  if !PLUGIFY_STRING_NUMERIC_CONVERSIONS
#    include <cstdlib>
#  endif
#  define _PLUGIFY_STRING_ASSERT(x, ...) do { if (!(x)) { std::abort(); } } while (0)
#else
#  define _PLUGIFY_STRING_ASSERT(x, str, ...) do { if (!(x)) { PLUGIFY_STRING_FALLBACK_ABORT_FUNCTION (str); { while (true) { [] { } (); } } } } while (0)
#endif

#define _PLUGIFY_STRING_PRAGMA_IMPL(x) _Pragma(#x)
#define _PLUGIFY_STRING_PRAGMA(x) _PLUGIFY_STRING_PRAGMA_IMPL(x)

#if defined(__clang__)
#  define _PLUGIFY_STRING_PRAGMA_DIAG_PREFIX clang
#elif defined(__GNUC__)
#  define _PLUGIFY_STRING_PRAGMA_DIAG_PREFIX GCC
#endif

#if defined(__GNUC__) || defined(__clang__)
#define _PLUGIFY_STRING_DIAG_PUSH() _PLUGIFY_STRING_PRAGMA(_PLUGIFY_STRING_PRAGMA_DIAG_PREFIX diagnostic push)
#define _PLUGIFY_STRING_DIAG_IGN(wrn) _PLUGIFY_STRING_PRAGMA(_PLUGIFY_STRING_PRAGMA_DIAG_PREFIX diagnostic ignored wrn)
#define _PLUGIFY_STRING_DIAG_POP() _PLUGIFY_STRING_PRAGMA(_PLUGIFY_STRING_PRAGMA_DIAG_PREFIX diagnostic pop)
#elif defined(_MSC_VER)
#define _PLUGIFY_STRING_DIAG_PUSH()	__pragma(warning(push))
#define _PLUGIFY_STRING_DIAG_IGN(wrn) __pragma(warning(disable: wrn))
#define _PLUGIFY_STRING_DIAG_POP() __pragma(warning(pop))
#endif

#if defined(__GNUC__) || defined(__clang__)
#define _PLUGIFY_STRING_PACK(decl) decl __attribute__((__packed__))
#elif defined(_MSC_VER)
#define _PLUGIFY_STRING_PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#define _PLUGIFY_STRING_PACK(decl) decl
#endif

#if defined(__GNUC__) || defined(__clang__)
#define _PLUGIFY_ALWAYS_INLINE __attribute__((always_inline)) inline
#define _PLUGIFY_ALWAYS_RESTRICT __restrict__
#elif defined(_MSC_VER)
#define _PLUGIFY_ALWAYS_INLINE __forceinline
#define _PLUGIFY_ALWAYS_RESTRICT __restrict
#else
#define _PLUGIFY_ALWAYS_INLINE inline
#define _PLUGIFY_ALWAYS_RESTRICT
#endif

namespace plg {
	namespace detail {
		template<typename Allocator, typename = void>
		struct is_allocator : std::false_type {};

		template<typename Allocator>
		struct is_allocator<Allocator, std::void_t<typename Allocator::value_type, decltype(std::declval<Allocator&>().allocate(std::size_t{}))>> : std::true_type {};

		template<typename Allocator>
		constexpr inline bool is_allocator_v = is_allocator<Allocator>::value;

		struct uninitialized_size_tag {};

		template<typename>
		constexpr bool dependent_false = false;

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<typename Range, typename Type>
		concept container_compatible_range = std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, Type>;
#endif
	}// namespace detail

	// basic_string
	// based on implementations from libc++, libstdc++ and Microsoft STL
	template<typename Char, typename Traits = std::char_traits<Char>, typename Allocator = std::allocator<Char>>
	class basic_string {
		using alloc_traits = std::allocator_traits<Allocator>;
	public:
		using traits_type = Traits;
		using value_type = typename traits_type::char_type;
		using allocator_type = Allocator;
		using size_type = typename alloc_traits::size_type;
		using difference_type = typename alloc_traits::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = typename alloc_traits::pointer;
		using const_pointer = typename alloc_traits::const_pointer;
		using iterator = value_type*;
		using const_iterator = const value_type*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using sview_type = std::basic_string_view<Char, Traits>;

		constexpr static size_type npos = static_cast<size_t>(-1);

	private:
		constexpr static auto _terminator = value_type();

#if _MSC_VER
#define no_unique_address msvc::no_unique_address
#endif
		[[no_unique_address]] Allocator _allocator;

		_PLUGIFY_STRING_DIAG_PUSH()

#if defined(__clang__)
		_PLUGIFY_STRING_DIAG_IGN("-Wgnu-anonymous-struct")
		_PLUGIFY_STRING_DIAG_IGN("-Wzero-length-array")
#elif defined(__GNUC__)
		_PLUGIFY_STRING_DIAG_IGN("-Wpedantic")// this doesn't work
#elif defined(_MSC_VER)
		_PLUGIFY_STRING_DIAG_IGN(4201)
		_PLUGIFY_STRING_DIAG_IGN(4200)
#endif

		template <typename CharT, size_t = sizeof(CharT)>
		struct padding {
			[[maybe_unused]] char padding[sizeof(CharT) - sizeof(char)];
		};

		template <typename CharT>
		struct padding<CharT, 1> {
			// template specialization to remove the padding structure to avoid warnings on zero length arrays
			// also, this allows us to take advantage of the empty-base-class optimization.
		};

		// size must correspond to the last byte of long_data.cap, so we don't want the compiler to insert
		// padding after size if sizeof(value_type) != 1; Also ensures both layouts are the same size.
		struct sso_size : padding<value_type> {
			_PLUGIFY_STRING_PACK(struct {
				unsigned char spare_size : 7;
				unsigned char is_long : 1;
			});
		};

		static constexpr int char_bit = std::numeric_limits<char>::digits + std::numeric_limits<char>::is_signed;

		static_assert(char_bit == 8, "assumes an 8 bit byte.");

		struct long_data {
			pointer data;
			size_type size;
			_PLUGIFY_STRING_PACK(struct {
				 size_type cap : sizeof(std::size_t) * char_bit - 1;
				 size_type is_long : 1;
			});
		};

		static constexpr size_type min_cap = (sizeof(long_data) - sizeof(char)) / sizeof(value_type) > 2 ? (sizeof(long_data) - sizeof(char)) / sizeof(value_type) : 2;

		struct short_data {
			value_type data[min_cap];
			sso_size size;
		};

		_PLUGIFY_STRING_DIAG_POP()

		static_assert(sizeof(short_data) == (sizeof(value_type) * (min_cap + sizeof(char))), "short has an unexpected size.");
		static_assert(sizeof(short_data) == sizeof(long_data), "short and long layout structures must be the same size");

		union storage_t {
			long_data _long;
			short_data _short{};
		} storage;

		constexpr static bool fits_in_sso(size_type size) {
			return size < min_cap;
		}

		constexpr void long_init() {
			this->is_long(true);
			this->set_long_data(nullptr);
			this->set_long_size(0);
			this->set_long_cap(0);
		}

		constexpr void short_init() {
			if (auto& buffer = this->get_long_data(); this->is_long() && buffer != nullptr) {
				this->_allocator.deallocate(buffer, this->get_long_cap() + 1);
				buffer = nullptr;
			}

			this->is_long(false);
			this->set_short_size(0);
		}

		constexpr void default_init(size_type size) {
			if (fits_in_sso(size))
				this->short_init();
			else
				this->long_init();
		}

		constexpr auto& get_long_data() noexcept {
			return this->storage._long.data;
		}

		constexpr const auto& get_long_data() const noexcept {
			return this->storage._long.data;
		}

		constexpr auto& get_short_data() noexcept {
			return this->storage._short.data;
		}

		constexpr const auto& get_short_data() const noexcept {
			return this->storage._short.data;
		}

		constexpr void set_short_size(size_type size) noexcept {
			this->storage._short.size.spare_size = min_cap - (size & 0x7F);
		}

		constexpr size_type get_short_size() const noexcept {
			return min_cap - this->storage._short.size.spare_size;
		}

		constexpr void set_long_size(size_type size) noexcept {
			this->storage._long.size = size;
		}

		constexpr size_type get_long_size() const noexcept {
			return this->storage._long.size;
		}

		constexpr void set_long_cap(size_type cap) noexcept {
			this->storage._long.cap = cap;
		}

		constexpr size_type get_long_cap() const noexcept {
			return this->storage._long.cap;
		}

		constexpr void set_long_data(value_type* data) {
			this->storage._long.data = data;
		}

		constexpr void is_long(bool l)  noexcept {
			this->storage._long.is_long = l;
		}

		constexpr bool is_long() const noexcept {
			return this->storage._long.is_long == true;
		}

		constexpr pointer get_data() noexcept {
			return this->is_long() ? this->get_long_data() : this->get_short_data();
		}

		constexpr const_pointer get_data() const noexcept {
			return this->is_long() ? this->get_long_data() : this->get_short_data();
		}

		constexpr size_type get_size() const noexcept {
			return this->is_long() ? this->get_long_size() : this->get_short_size();
		}

		constexpr void set_size(size_type size) noexcept {
			if (this->is_long())
				this->set_long_size(size);
			else
				this->set_short_size(size);
		}

		constexpr size_type get_cap() const noexcept {
			if (this->is_long())
				return this->get_long_cap();
			else
				return min_cap;
		}

		constexpr sview_type get_view() const noexcept {
			return sview_type(this->get_data(), this->get_size());
		}

		constexpr void reallocate(std::size_t new_cap, bool copy_old) {
			if (new_cap == this->get_long_cap())
				return;

			auto old_len = this->get_long_size();
			auto old_cap = this->get_long_cap();
			auto& old_buffer = this->get_long_data();

			auto new_len = std::min(new_cap, old_len);
			auto new_buffer = this->_allocator.allocate(new_cap + 1);

			if (old_buffer != nullptr) {
				if (old_len != 0 && copy_old)
					Traits::copy(new_buffer, old_buffer, new_len);
				this->_allocator.deallocate(old_buffer, old_cap + 1);
			}

			this->set_long_data(new_buffer);
			this->set_long_size(new_len);
			this->set_long_cap(new_cap);
		}

		constexpr void grow_to(size_type new_cap) {
			if (this->is_long() == true) {
				this->reallocate(new_cap, true);
				return;
			}

			auto buffer = this->_allocator.allocate(new_cap + 1);
			auto len = this->get_short_size();

			Traits::copy(buffer, this->get_short_data(), len);
			Traits::assign(buffer[len], _terminator);

			this->long_init();
			this->set_long_data(buffer);
			this->set_long_size(len);
			this->set_long_cap(new_cap);
		}

		constexpr void null_terminate() noexcept {
			auto buffer = this->get_data();
			if (buffer == nullptr)
				return;
			Traits::assign(buffer[this->get_size()], _terminator);
		}

		constexpr bool addr_in_range(const_pointer ptr) const {
			if (std::is_constant_evaluated())
				return false;
			return this->get_data() <= ptr && ptr <= this->get_data() + this->get_size();
		}

		constexpr void internal_replace_impl(auto func, size_type pos, size_type oldcount, size_type count) {
			auto cap = this->get_cap();
			auto sz = this->get_size();

			auto rsz = sz - oldcount + count;

			if (cap < rsz)
				this->grow_to(rsz);

			if (oldcount != count)
				Traits::move(this->get_data() + pos + count, this->get_data() + pos + oldcount, sz - pos - oldcount);

			func();

			this->set_size(rsz);
			this->null_terminate();
		}

		constexpr void internal_replace(size_type pos, const_pointer str, size_type oldcount, size_type count) {
			if (this->addr_in_range(str)) {
				basic_string rstr(str, count);
				this->internal_replace_impl([&]() { Traits::copy(this->get_data() + pos, rstr.data(), count); }, pos, oldcount, count);
			} else
				this->internal_replace_impl([&]() { Traits::copy(this->get_data() + pos, str, count); }, pos, oldcount, count);
		}

		constexpr void internal_replace(size_type pos, value_type ch, size_type oldcount, size_type count) {
			this->internal_replace_impl([&]() { Traits::assign(this->get_data() + pos, count, ch); }, pos, oldcount, count);
		}

		constexpr void internal_insert_impl(auto func, size_type pos, size_type size) {
			if (size != 0) {
				auto cap = this->get_cap();
				auto sz = this->get_size();
				auto rsz = sz + size;

				if (cap < rsz)
					this->grow_to(rsz);

				Traits::move(this->get_data() + pos + size, this->get_data() + pos, sz - pos);
				func();

				this->set_size(rsz);
				this->null_terminate();
			}
		}

		constexpr void internal_insert(size_type pos, const_pointer str, size_type count) {
			if (this->addr_in_range(str)) {
				basic_string rstr(str, count);
				this->internal_insert_impl([&]() { Traits::copy(this->get_data() + pos, rstr.data(), count); }, pos, count);
			} else
				this->internal_insert_impl([&]() { Traits::copy(this->get_data() + pos, str, count); }, pos, count);
		}

		constexpr void internal_insert(size_type pos, value_type ch, size_type count) {
			this->internal_insert_impl([&]() { Traits::assign(this->get_data() + pos, count, ch); }, pos, count);
		}

		constexpr void internal_append_impl(auto func, size_type size) {
			if (size != 0) {
				auto cap = this->get_cap();
				auto sz = this->get_size();
				auto rsz = sz + size;

				if (cap < rsz)
					this->grow_to(rsz);

				func(sz);
				this->set_size(rsz);
				this->null_terminate();
			}
		}

		constexpr void internal_append(const_pointer str, size_type count) {
			if (this->addr_in_range(str)) {
				basic_string rstr(str, count);
				this->internal_append_impl([&](size_type pos) { Traits::copy(this->get_data() + pos, rstr.data(), count); }, count);
			} else
				this->internal_append_impl([&](size_type pos) { Traits::copy(this->get_data() + pos, str, count); }, count);
		}

		constexpr void internal_append(value_type ch, size_type count) {
			this->internal_append_impl([&](size_type pos) { Traits::assign(this->get_data() + pos, count, ch); }, count);
		}

		constexpr void internal_assign_impl(auto func, size_type size, bool copy_old) {
			if (fits_in_sso(size)) {
				if (this->is_long() == true)
					this->short_init();

				this->set_short_size(size);
				func(this->get_short_data());
				this->null_terminate();
			} else {
				if (this->is_long() == false)
					this->long_init();
				if (this->get_long_cap() < size)
					this->reallocate(size, copy_old);

				func(this->get_long_data());
				this->set_long_size(size);
				this->null_terminate();
			}
		}

		constexpr void internal_assign(const_pointer str, size_type size, bool copy_old = false) {
			if (this->addr_in_range(str)) {
				basic_string rstr(str, size);
				this->internal_assign_impl([&](auto data) { Traits::copy(data, rstr.data(), size); }, size, copy_old);
			} else
				this->internal_assign_impl([&](auto data) { Traits::copy(data, str, size); }, size, copy_old);
		}

		constexpr void internal_assign(value_type ch, size_type count, bool copy_old = false) {
			this->internal_assign_impl([&](auto data) { Traits::assign(data, count, ch); }, count, copy_old);
		}

	public:
		explicit constexpr basic_string(detail::uninitialized_size_tag, size_type size, const allocator_type& a) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			if (fits_in_sso(size))
				this->short_init();
			else {
				this->long_init();
				this->reallocate(size, false);
			}
			this->set_size(size);
		}

		constexpr basic_string() noexcept(std::is_nothrow_default_constructible<allocator_type>::value) requires(detail::is_allocator_v<Allocator>) : basic_string(allocator_type()) {}
		explicit constexpr basic_string(const allocator_type& a) noexcept requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			this->short_init();
		}

		constexpr basic_string(size_type count, value_type ch, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			_PLUGIFY_STRING_ASSERT(count <= this->max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			this->internal_assign(ch, count);
		}

		constexpr basic_string(const basic_string& str, size_type pos, size_type count, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			_PLUGIFY_STRING_ASSERT(pos <= str.get_size(), "plg::basic_string::basic_string(): pos out of range", std::out_of_range);
			auto len = std::min(count, str.get_size() - pos);
			this->internal_assign(str.data(), len);
		}
		constexpr basic_string(const basic_string& str, size_type pos, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>) : basic_string(str, pos, npos, a) {}

		constexpr basic_string(const value_type* str, size_type count, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			_PLUGIFY_STRING_ASSERT(count <= this->max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			this->internal_assign(str, count);
		}
		constexpr basic_string(const value_type* str, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>)
			: basic_string(str, Traits::length(str), a) {}

		template<std::input_iterator InputIterator>
		constexpr basic_string(InputIterator first, InputIterator last, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			auto len = std::distance(first, last);
			_PLUGIFY_STRING_ASSERT((size_t) len <= this->max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			this->internal_assign(const_pointer(first), len);
		}

		constexpr basic_string(const basic_string& str, const allocator_type& a) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			auto len = str.length();
			this->internal_assign(str.data(), len);
		}
		constexpr basic_string(const basic_string& str) : basic_string(str, allocator_type()) {}

		constexpr basic_string(basic_string&& str, const allocator_type& a) requires(detail::is_allocator_v<Allocator>) : _allocator(a), storage(std::move(str.storage)) {
			if (str.is_long() && a != str._allocator) {
				auto len = str.get_long_size();
				this->internal_assign(str.get_long_data(), len);
			} else {
				this->storage = str.storage;
				str.short_init();
			}
		}
		constexpr basic_string(basic_string&& str) noexcept(std::is_nothrow_move_constructible<allocator_type>::value) : basic_string(str, std::move(str._allocator)) {}

		constexpr basic_string(std::initializer_list<value_type> ilist, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			auto len = ilist.size();
			_PLUGIFY_STRING_ASSERT(len <= this->max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			this->internal_assign(const_pointer(ilist.begin()), len);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type>)
		constexpr basic_string(const Type& t, size_type pos, size_type count, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			auto sv = sview_type(t);
			_PLUGIFY_STRING_ASSERT(pos <= sv.length(), "plg::basic_string::basic_string(): pos out of range", std::out_of_range);

			auto ssv = sv.substr(pos, count);
			auto len = ssv.length();
			_PLUGIFY_STRING_ASSERT(len <= this->max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			this->internal_assign(ssv.data(), len);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string(const Type& t, const allocator_type& a = allocator_type()) requires(detail::is_allocator_v<Allocator>) : _allocator(a) {
			sview_type sv(t);
			auto len = sv.length();
			_PLUGIFY_STRING_ASSERT(len <= this->max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			this->internal_assign(sv.data(), len);
		}

		/*constexpr basic_string(basic_string &&str, size_type pos, size_type count, const allocator_type &a = allocator_type()) requires(detail::is_allocator_v<Allocator>) {

        }
		constexpr basic_string(basic_string &&str, size_type pos, const allocator_type &a = allocator_type()) requires(detail::is_allocator_v<Allocator>) {

        }*/

#if __cplusplus > 202002L
		basic_string(std::nullptr_t) = delete;
#endif

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr basic_string(std::from_range_t, Range&& range, const Allocator& a = Allocator()) : basic_string(std::ranges::begin(range), std::ranges::end(range), a) {}
#endif

		constexpr ~basic_string() {
			if (this->is_long())
				if (auto& buffer = this->get_long_data(); buffer != nullptr)
					this->_allocator.deallocate(buffer, this->get_long_cap() + 1);
		}

		constexpr basic_string& operator=(const basic_string& str) {
			return this->assign(str);
		}

		constexpr basic_string& operator=(basic_string&& str) noexcept(
				alloc_traits::propagate_on_container_move_assignment::value ||
				alloc_traits::is_always_equal::value) {
			return this->assign(str);
		}

		constexpr basic_string& operator=(const value_type* str) {
			return this->assign(str, Traits::length(str));
		}

		constexpr basic_string& operator=(value_type ch) {
			return this->assign(addressof(ch), 1);
		}

		constexpr basic_string& operator=(std::initializer_list<value_type> ilist) {
			return this->assign(ilist.begin(), ilist.size());
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& operator=(const Type& t) {
			sview_type sv(t);
			return this->assign(sv);
		}

#if __cplusplus > 202002L
		constexpr basic_string& operator=(std::nullptr_t) = delete;
#endif

		constexpr basic_string& assign(size_type count, value_type ch) {
			_PLUGIFY_STRING_ASSERT(count <= this->max_size(), "plg::basic_string::basic_string(): resulted string size would exceed max_size()", std::length_error);
			this->internal_assign(ch, count);
			return *this;
		}

		constexpr basic_string& assign(const basic_string& str) {
			this->internal_assign(str.data(), str.size());
			return *this;
		}

		constexpr basic_string& assign(const basic_string& str, size_type pos, size_type count = npos) {
			_PLUGIFY_STRING_ASSERT(pos <= str.get_size(), "plg::basic_string::assign(): pos out of range", std::out_of_range);
			this->internal_assign(str.data(), std::min(count, str.size() - pos));
			return *this;
		}

		constexpr basic_string& assign(basic_string&& str) noexcept(
				alloc_traits::propagate_on_container_move_assignment::value ||
				alloc_traits::is_always_equal::value) {
			if (this->_allocator == str._allocator)
				this->swap(str);
			else
				this->internal_assign(str.data(), str.size());

			return *this;
		}

		constexpr basic_string& assign(const value_type* str, size_type count) {
			_PLUGIFY_STRING_ASSERT(count <= this->max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			this->internal_assign(str, count);
			return *this;
		}

		constexpr basic_string& assign(const value_type* str) {
			return this->assign(str, Traits::length(str));
		}

		template<typename InputIterator>
		constexpr basic_string& assign(InputIterator first, InputIterator last) {
			auto len = std::distance(first, last);
			_PLUGIFY_STRING_ASSERT((size_t) len <= this->max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			this->internal_assign(const_pointer(first), len);
			return *this;
		}

		constexpr basic_string& assign(std::initializer_list<value_type> ilist) {
			auto len = ilist.size();
			_PLUGIFY_STRING_ASSERT(len <= this->max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			this->internal_assign(const_pointer(ilist.begin()), len);
			return *this;
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& assign(const Type& t) {
			sview_type sv(t);
			return this->assign(sv.data(), sv.length());
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& assign(const Type& t, size_type pos, size_type count = npos) {
			auto sv = sview_type(t).substr(pos, count);
			auto len = sv.length();
			_PLUGIFY_STRING_ASSERT(len <= this->max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			return this->assign(sv.data(), len);
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr basic_string& assign_range(Range&& range) {
			auto str = basic_string(std::from_range, std::forward<Range>(range), this->_allocator);
			_PLUGIFY_STRING_ASSERT(str.get_size() <= this->max_size(), "plg::basic_string::assign_range(): resulted string size would exceed max_size()", std::length_error);
			return this->assign();
		}
#endif

		constexpr allocator_type get_allocator() const noexcept {
			return this->_allocator;
		}

		constexpr reference operator[](size_type pos) {
			return this->get_data()[pos];
		}

		constexpr const_reference operator[](size_type pos) const {
			return this->get_data()[pos];
		}

		constexpr reference at(size_type pos) {
			_PLUGIFY_STRING_ASSERT(pos < this->get_size(), "plg::basic_string::at(): pos out of range", std::out_of_range);
			return this->get_data()[pos];
		}

		constexpr const_reference at(size_type pos) const {
			_PLUGIFY_STRING_ASSERT(pos < this->get_size(), "plg::basic_string::at(): pos out of range", std::out_of_range);
			return this->get_data()[pos];
		}

		constexpr reference front() {
			return this->get_data()[0];
		}

		constexpr const_reference front() const {
			return this->get_data()[0];
		}

		constexpr reference back() {
			return this->get_data()[this->get_size() - 1];
		}

		constexpr const_reference back() const {
			return this->get_data()[this->get_size() - 1];
		}

		constexpr const value_type* data() const noexcept {
			return this->get_data();
		}

		constexpr value_type* data() noexcept {
			return this->get_data();
		}

		constexpr const value_type* c_str() const noexcept {
			return this->get_data();
		}

		constexpr operator sview_type() const noexcept {
			return this->get_view();
		}

		constexpr iterator begin() noexcept {
			return this->get_data();
		}

		constexpr const_iterator begin() const noexcept {
			return this->get_data();
		}

		constexpr const_iterator cbegin() const noexcept {
			return this->get_data();
		}

		constexpr iterator end() noexcept {
			return this->get_data() + this->get_size();
		}

		constexpr const_iterator end() const noexcept {
			return this->get_data() + this->get_size();
		}

		constexpr const_iterator cend() const noexcept {
			return this->get_data() + this->get_size();
		}

		constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(this->end());
		}

		constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(this->end());
		}

		constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(this->cend());
		}

		constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(this->begin());
		}

		constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(this->begin());
		}

		constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(this->cbegin());
		}

		constexpr bool empty() const noexcept {
			return this->get_size() == 0;
		}

		constexpr size_type size() const noexcept {
			return this->get_size();
		}

		constexpr size_type length() const noexcept {
			return this->get_size();
		}

		constexpr size_type max_size() const noexcept {
			// size_type m = alloc_traits::max_size(this->_allocator);

			// if (m <= numeric_limits<size_type>::max() / 2)
			//     return m - alignment;
			// else
			//     return (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) ? m - alignment : (m / 2) - alignment;
			return (alloc_traits::max_size(allocator_type()) - 1) / 2;
		}

		constexpr void reserve(size_type cap) {
			_PLUGIFY_STRING_ASSERT(cap <= this->max_size(), "plg::basic_string::reserve(): allocated memory size would exceed max_size()", std::length_error);
			if (cap <= this->get_cap())
				return;

			auto new_cap = std::max(cap, this->get_size());
			if (new_cap == this->get_cap())
				return;

			this->grow_to(new_cap);
		}

		[[deprecated]] void reserve() {
			this->shrink_to_fit();
		}

		constexpr size_type capacity() const noexcept {
			return this->get_cap();
		}

		constexpr void shrink_to_fit() {
			if (this->is_long() == false)
				return;

			this->reallocate(this->get_size(), true);
		}

		constexpr void clear() noexcept {
			this->set_size(0);
		}

		constexpr basic_string& insert(size_type pos, size_type count, value_type ch) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + count <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			this->insert(std::next(this->cbegin(), pos), count, ch);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const value_type* str) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			auto len = Traits::length(str);
			_PLUGIFY_STRING_ASSERT(this->get_size() + len <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			this->internal_insert(pos, str, len);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const value_type* str, size_type count) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + count <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			this->internal_insert(pos, str, count);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const basic_string& str) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + str.get_size() <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			this->internal_insert(pos, const_pointer(str.get_data()), str.get_size());
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const basic_string& str, size_type pos_str, size_type count = npos) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size() && pos_str <= str.get_size(), "plg::basic_string::insert(): pos or pos_str out of range", std::out_of_range);
			count = std::min(count, str.length() - pos_str);
			_PLUGIFY_STRING_ASSERT(this->get_size() + count <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			return this->insert(pos, str.data() + pos_str, count);
		}

		constexpr iterator insert(const_iterator pos, value_type ch) {
			return this->insert(pos, 1, ch);
		}

		constexpr iterator insert(const_iterator pos, size_type count, value_type ch) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + count <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			auto spos = std::distance(this->cbegin(), pos);
			this->internal_insert(spos, ch, count);
			return std::next(this->begin(), spos);
		}

		template<typename InputIterator>
		constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
			auto spos = std::distance(this->cbegin(), pos);
			auto len = std::distance(first, last);
			_PLUGIFY_STRING_ASSERT(this->get_size() + len <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			this->internal_insert(spos, const_pointer(first), len);
			return std::next(this->begin(), spos);
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + ilist.size() <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			auto spos = std::distance(this->cbegin(), pos);
			this->internal_insert(spos, const_pointer(ilist.begin()), ilist.size());
			return std::next(this->begin(), spos);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& insert(size_type pos, const Type& t) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			sview_type sv(t);
			_PLUGIFY_STRING_ASSERT(this->get_size() + sv.length() <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			this->internal_insert(pos, const_pointer(sv.data()), sv.length());
			return *this;
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& insert(size_type pos, const Type& t, size_type pos_str, size_type count = npos) {
			auto sv = sview_type(t);
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size() && pos_str <= sv.length(), "plg::basic_string::insert(): pos or pos_str out of range", std::out_of_range);
			auto ssv = sv.substr(pos_str, count);
			_PLUGIFY_STRING_ASSERT(this->get_size() + ssv.length() <= this->max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			this->internal_insert(pos, const_pointer(ssv.data()), ssv.length());
			return *this;
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr iterator insert_range(const_iterator pos, Range&& range) {
			auto str = basic_string(std::from_range, std::forward<Range>(range), this->_allocator);
			_PLUGIFY_STRING_ASSERT(this->get_size() + str.get_size() <= this->max_size(), "plg::basic_string::insert_range(): resulted string size would exceed max_size()", std::length_error);
			return this->insert(pos - this->begin(), str);
		}
#endif

		constexpr basic_string& erase(size_type pos = 0, size_type count = npos) {
			auto sz = this->get_size();
			auto buffer = this->get_data();

			_PLUGIFY_STRING_ASSERT(pos <= sz, "plg::basic_string::erase(): pos out of range", std::out_of_range);

			count = std::min(count, sz - pos);

			auto left = sz - (pos + count);
			if (left != 0)
				Traits::move(buffer + pos, buffer + pos + count, left);

			auto new_size = pos + left;
			this->set_size(new_size);
			this->null_terminate();

			return *this;
		}

		constexpr iterator erase(const_iterator position) {
			auto pos = std::distance(this->cbegin(), position);
			this->erase(pos, 1);
			return this->begin() + pos;
		}

		constexpr iterator erase(const_iterator first, const_iterator last) {
			auto pos = std::distance(this->cbegin(), first);
			auto len = std::distance(first, last);
			this->erase(pos, len);
			return this->begin() + pos;
		}

		constexpr void push_back(value_type ch) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + 1 <= this->max_size(), "plg::basic_string::push_back(): resulted string size would exceed max_size()", std::length_error);
			this->append(1, ch);
		}

		constexpr void pop_back() {
			this->erase(this->end() - 1);
		}

		constexpr basic_string& append(size_type count, value_type ch) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + count <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			this->internal_append(ch, count);
			return *this;
		}

		constexpr basic_string& append(const basic_string& str) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + str.get_size() <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			this->internal_append(str.get_data(), str.get_size());
			return *this;
		}

		constexpr basic_string& append(const basic_string& str, size_type pos, size_type count = npos) {
			_PLUGIFY_STRING_ASSERT(pos <= str.get_size(), "plg::basic_string::append(): pos out of range", std::out_of_range);
			auto ssv = sview_type(str).substr(pos, count);
			_PLUGIFY_STRING_ASSERT(this->get_size() + ssv.length() <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			this->internal_append(ssv.data(), ssv.length());
			return *this;
		}

		constexpr basic_string& append(const value_type* str, size_type count) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + count <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			this->internal_append(str, count);
			return *this;
		}

		constexpr basic_string& append(const value_type* str) {
			auto len = Traits::length(str);
			_PLUGIFY_STRING_ASSERT(this->get_size() + len <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			return this->append(str, len);
		}

		template<typename InputIterator>
		constexpr basic_string& append(InputIterator first, InputIterator last) {
			auto len = std::distance(first, last);
			_PLUGIFY_STRING_ASSERT(this->get_size() + len <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			this->internal_append(const_pointer(first), len);
			return *this;
		}

		constexpr basic_string& append(std::initializer_list<value_type> ilist) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + ilist.size() <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			this->internal_append(const_pointer(ilist.begin()), ilist.size());
			return *this;
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& append(const Type& t) {
			sview_type sv(t);
			_PLUGIFY_STRING_ASSERT(this->get_size() + sv.length() <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			this->internal_append(sv.data(), sv.size());
			return *this;
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& append(const Type& t, size_type pos, size_type count = npos) {
			sview_type sv(t);
			_PLUGIFY_STRING_ASSERT(pos < sv.length(), "plg::basic_string::append(): pos out of range", std::out_of_range);
			auto ssv = sv.substr(pos, count);
			_PLUGIFY_STRING_ASSERT(this->get_size() + ssv.length() <= this->max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			this->internal_append(ssv.data(), ssv.length());
			return *this;
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr basic_string& append_range(Range&& range) {
			auto str = basic_string(std::from_range, std::forward<Range>(range), this->_allocator);
			_PLUGIFY_STRING_ASSERT(this->get_size() + str.get_size() <= this->max_size(), "plg::basic_string::insert_range(): resulted string size would exceed max_size()", std::length_error);
			return this->append(str);
		}
#endif

		constexpr basic_string& operator+=(const basic_string& str) {
			return this->append(str);
		}

		constexpr basic_string& operator+=(value_type ch) {
			this->push_back(ch);
			return *this;
		}

		constexpr basic_string& operator+=(const value_type* str) {
			return this->append(str);
		}

		constexpr basic_string& operator+=(std::initializer_list<value_type> ilist) {
			return this->append(ilist);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& operator+=(const Type& t) {
			return this->append(sview_type(t));
		}

		constexpr int compare(const basic_string& str) const noexcept {
			return this->get_view().compare(str.get_view());
		}

		constexpr int compare(size_type pos1, size_type count1, const basic_string& str) const {
			return this->get_view().compare(pos1, count1, str.get_view());
		}

		constexpr int compare(size_type pos1, size_type count1, const basic_string& str, size_type pos2, size_type count2 = npos) const {
			return this->get_view().compare(pos1, count1, str.get_view(), pos2, count2);
		}

		constexpr int compare(const value_type* str) const {
			return this->get_view().compare(str);
		}

		constexpr int compare(size_type pos1, size_type count1, const value_type* str) const {
			return this->get_view().compare(pos1, count1, str);
		}

		constexpr int compare(size_type pos1, size_type count1, const value_type* str, size_type count2) const {
			return this->get_view().compare(pos1, count1, str, count2);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr int compare(const Type& t) const noexcept(noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>)) {
			return this->get_view().compare(sview_type(t));
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr int compare(size_type pos1, size_type count1, const Type& t) const {
			return this->get_view().compare(pos1, count1, sview_type(t));
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr int compare(size_type pos1, size_type count1, const Type& t, size_type pos2, size_type count2 = npos) const {
			return this->get_view().compare(pos1, count1, sview_type(t), pos2, count2);
		}

		constexpr bool starts_with(sview_type sv) const noexcept {
			return this->get_view().starts_with(sv);
		}

		constexpr bool starts_with(Char ch) const noexcept {
			return this->get_view().starts_with(ch);
		}

		constexpr bool starts_with(const Char* str) const {
			return this->get_view().starts_with(str);
		}

		constexpr bool ends_with(sview_type sv) const noexcept {
			return this->get_view().ends_with(sv);
		}

		constexpr bool ends_with(Char ch) const noexcept {
			return this->get_view().ends_with(ch);
		}

		constexpr bool ends_with(const Char* str) const {
			return this->get_view().ends_with(str);
		}

		constexpr bool contains(sview_type sv) const noexcept {
			return this->get_view().contains(sv);
		}

		constexpr bool contains(Char ch) const noexcept {
			return this->get_view().contains(ch);
		}

		constexpr bool contains(const Char* str) const {
			return this->get_view().contains(str);
		}

		constexpr basic_string& replace(size_type pos, size_type count, const basic_string& str) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			return this->replace(pos, count, str, 0, str.length());
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const basic_string& str) {
			auto pos = std::distance(this->cbegin(), first);
			auto count = std::distance(first, last);
			return this->replace(pos, count, str, 0, str.length());
		}

		constexpr basic_string& replace(size_type pos, size_type count, const basic_string& str, size_type pos2, size_type count2 = npos) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size() && pos2 <= str.get_size(), "plg::basic_string::replace(): pos or pos_str out of range", std::out_of_range);
			count2 = std::min(count2, str.length() - pos2);
			auto ssv = sview_type(str).substr(pos2, count2);
			return this->replace(pos, count, ssv.data(), ssv.length());
		}

		template<typename InputIterator>
		constexpr basic_string& replace(const_iterator first, const_iterator last, InputIterator first2, InputIterator last2) {
			return this->replace(first, last, const_pointer(first2), std::distance(first2, last2));
		}

		constexpr basic_string& replace(size_type pos, size_type count, const value_type* str, size_type count2) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			count = std::min(count, this->length() - pos);
			_PLUGIFY_STRING_ASSERT(this->get_size() - count + count2 <= this->max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			this->internal_replace(pos, const_pointer(str), count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const value_type* str, size_type count2) {
			size_type pos = std::distance(this->cbegin(), first);
			size_type count = std::distance(first, last);

			return this->replace(pos, count, str, count2);
		}

		constexpr basic_string& replace(size_type pos, size_type count, const value_type* str) {
			return this->replace(pos, count, str, Traits::length(str));
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const value_type* str) {
			return this->replace(first, last, str, Traits::length(str));
		}

		constexpr basic_string& replace(size_type pos, size_type count, size_type count2, value_type ch) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			count = std::min(count, this->length() - pos);
			_PLUGIFY_STRING_ASSERT(this->get_size() - count + count2 <= this->max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			this->internal_replace(pos, ch, count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, size_type count2, value_type ch) {
			auto pos = std::distance(this->cbegin(), first);
			auto count = std::distance(first, last);

			_PLUGIFY_STRING_ASSERT(this->get_size() - count + count2 <= this->max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			this->internal_replace(pos, ch, count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, std::initializer_list<value_type> ilist) {
			return this->replace(first, last, const_pointer(ilist.begin()), ilist.size());
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(size_type pos, size_type count, const Type& t) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			sview_type sv(t);
			return this->replace(pos, count, sv.data(), sv.length());
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(const_iterator first, const_iterator last, const Type& t) {
			sview_type sv(t);
			return this->replace(first, last, sv.data(), sv.length());
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(size_type pos, size_type count, const Type& t, size_type pos2, size_type count2 = npos) {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			auto sv = sview_type(t).substr(pos2, count2);
			return this->replace(pos, count, sv.data(), sv.length());
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr iterator replace_with_range(const_iterator first, const_iterator last, Range&& range) {
			auto str = basic_string(std::from_range, std::forward<Range>(range), this->_allocator);
			return this->replace(first, last, str);// replace checks for max_size()
		}
#endif

		constexpr basic_string substr(size_type pos = 0, size_type count = npos) const {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::substr(): pos out of range", std::out_of_range);
			return basic_string(*this, pos, count);
		}

		constexpr size_type copy(value_type* str, size_type count, size_type pos = 0) const {
			_PLUGIFY_STRING_ASSERT(pos <= this->get_size(), "plg::basic_string::copy(): pos out of range", std::out_of_range);
			return this->get_view().copy(str, count, pos);
		}

		constexpr void resize(size_type count, value_type ch) {
			_PLUGIFY_STRING_ASSERT(this->get_size() + count <= this->max_size(), "plg::basic_string::resize(): resulted string size would exceed max_size()", std::length_error);
			auto cap = this->get_cap();
			auto sz = this->get_size();
			auto rsz = count + sz;

			if (sz < rsz) {
				if (cap < rsz)
					this->grow_to(rsz);
				Traits::assign(this->get_data() + sz, count, ch);
			}
			this->set_size(rsz);
			this->null_terminate();
		}

		constexpr void resize(size_type count) {
			this->resize(count, _terminator);
		}

		template<typename Operation>
		constexpr void resize_and_overwrite(size_type, Operation) {
			static_assert(detail::dependent_false<Char>, "plg::basic_string::resize_and_overwrite(count, op) not implemented!");
		}

		constexpr void swap(basic_string& str) noexcept(alloc_traits::propagate_on_container_swap::value || alloc_traits::is_always_equal::value) {
			using std::swap;
			swap(this->storage, str.storage);
			swap(this->_allocator, str._allocator);
		}

		constexpr size_type find(const basic_string& str, size_type pos = 0) const noexcept {
			return this->get_view().find(sview_type(str), pos);
		}

		constexpr size_type find(const value_type* str, size_type pos, size_type count) const noexcept {
			return this->get_view().find(str, pos, count);
		}

		constexpr size_type find(const value_type* str, size_type pos = 0) const noexcept {
			return this->get_view().find(str, pos);
		}

		constexpr size_type find(value_type ch, size_type pos = 0) const noexcept {
			return this->get_view().find(ch, pos);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return this->get_view().find(sview_type(t), pos);
		}

		constexpr size_type rfind(const basic_string& str, size_type pos = npos) const noexcept {
			return this->get_view().rfind(sview_type(str), pos);
		}

		constexpr size_type rfind(const value_type* str, size_type pos, size_type count) const noexcept {
			return this->get_view().rfind(str, pos, count);
		}

		constexpr size_type rfind(const value_type* str, size_type pos = npos) const noexcept {
			return this->get_view().rfind(str, pos);
		}

		constexpr size_type rfind(value_type ch, size_type pos = npos) const noexcept {
			return this->get_view().rfind(ch, pos);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type rfind(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return this->get_view().rfind(sview_type(t), pos);
		}

		constexpr size_type find_first_of(const basic_string& str, size_type pos = 0) const noexcept {
			return this->get_view().find_first_of(sview_type(str), pos);
		}

		constexpr size_type find_first_of(const value_type* str, size_type pos, size_type count) const noexcept {
			return this->get_view().find_first_of(str, pos, count);
		}

		constexpr size_type find_first_of(const value_type* str, size_type pos = 0) const noexcept {
			return this->get_view().find_first_of(str, pos);
		}

		constexpr size_type find_first_of(value_type ch, size_type pos = 0) const noexcept {
			return this->get_view().find_first_of(ch, pos);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find_first_of(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return this->get_view().find_first_of(sview_type(t), pos);
		}

		constexpr size_type find_first_not_of(const basic_string& str, size_type pos = 0) const noexcept {
			return this->get_view().find_last_not_of(sview_type(str), pos);
		}

		constexpr size_type find_first_not_of(const value_type* str, size_type pos, size_type count) const noexcept {
			return this->get_view().find_last_not_of(str, pos, count);
		}

		constexpr size_type find_first_not_of(const value_type* str, size_type pos = 0) const noexcept {
			return this->get_view().find_last_not_of(str, pos);
		}

		constexpr size_type find_first_not_of(value_type ch, size_type pos = 0) const noexcept {
			return this->get_view().find_first_not_of(ch, pos);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find_first_not_of(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return this->get_view().find_first_not_of(sview_type(t), pos);
		}

		constexpr size_type find_last_of(const basic_string& str, size_type pos = npos) const noexcept {
			return this->get_view().find_last_of(sview_type(str), pos);
		}

		constexpr size_type find_last_of(const value_type* str, size_type pos, size_type count) const noexcept {
			return this->get_view().find_last_of(str, pos, count);
		}

		constexpr size_type find_last_of(const value_type* str, size_type pos = npos) const noexcept {
			return this->get_view().find_last_of(str, pos);
		}

		constexpr size_type find_last_of(value_type ch, size_type pos = npos) const noexcept {
			return this->get_view().find_last_of(ch, pos);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find_last_of(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return this->get_view().find_last_of(sview_type(t), pos);
		}

		constexpr size_type find_last_not_of(const basic_string& str, size_type pos = npos) const noexcept {
			return this->get_view().find_last_not_of(sview_type(str), pos);
		}

		constexpr size_type find_last_not_of(const value_type* str, size_type pos, size_type count) const noexcept {
			return this->get_view().find_last_not_of(str, pos, count);
		}

		constexpr size_type find_last_not_of(const value_type* str, size_type pos = npos) const noexcept {
			return this->get_view().find_last_not_of(str, pos);
		}

		constexpr size_type find_last_not_of(value_type ch, size_type pos = npos) const noexcept {
			return this->get_view().find_last_not_of(ch, pos);
		}

		template<typename Type>
			requires(
					std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr size_type find_last_not_of(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) {
			return this->get_view().find_last_not_of(sview_type(t), pos);
		}

		friend constexpr basic_string operator+(const basic_string& lhs, const basic_string& rhs) {
			auto lhs_sz = lhs.size();
			auto rhs_sz = rhs.size();
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::alloc_traits::select_on_container_copy_construction(lhs.get_allocator()));
			auto buffer = ret.get_data();
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
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::alloc_traits::select_on_container_copy_construction(rhs.get_allocator()));
			auto buffer = ret.get_data();
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
			basic_string ret(detail::uninitialized_size_tag(), rhs_sz + 1, basic_string::alloc_traits::select_on_container_copy_construction(rhs.get_allocator()));
			auto buffer = ret.get_data();
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
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::alloc_traits::select_on_container_copy_construction(lhs.get_allocator()));
			auto buffer = ret.get_data();
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
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + 1, basic_string::alloc_traits::select_on_container_copy_construction(lhs.get_allocator()));
			auto buffer = ret.get_data();
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
	template<typename InputIt, typename Allocator = std::allocator<typename std::iterator_traits<InputIt>::value_type>>
	basic_string(InputIt, InputIt, Allocator = Allocator()) -> basic_string<typename std::iterator_traits<InputIt>::value_type, std::char_traits<typename std::iterator_traits<InputIt>::value_type>, Allocator>;

	template<typename Char, typename Traits, typename Allocator = std::allocator<Char>>
	explicit basic_string(std::basic_string_view<Char, Traits>, const Allocator& = Allocator()) -> basic_string<Char, Traits, Allocator>;

	template<typename Char, typename Traits, typename Allocator = std::allocator<Char>>
	basic_string(std::basic_string_view<Char, Traits>, typename basic_string<Char, Traits, Allocator>::size_type, typename basic_string<Char, Traits, Allocator>::size_type, const Allocator& = Allocator()) -> basic_string<Char, Traits, Allocator>;

#if PLUGIFY_STRING_CONTAINERS_RANGES
	template<std::ranges::input_range Range, typename Allocator = std::allocator<std::ranges::range_value_t<Range>>>
	basic_string(std::from_range_t, Range&&, Allocator = Allocator()) -> basic_string<std::ranges::range_value_t<Range>, std::char_traits<std::ranges::range_value_t<Range>>, Allocator>;
#endif

	// basic_string typedef-names
	using string = basic_string<char>;
	using u8string = basic_string<char8_t>;
	using u16string = basic_string<char16_t>;
	using u32string = basic_string<char32_t>;
	using wstring = basic_string<wchar_t>;

#if PLUGIFY_STRING_NUMERIC_CONVERSIONS
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

#if PLUGIFY_STRING_FLOAT
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

#if PLUGIFY_STRING_LONG_DOUBLE
	inline long double stold(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtold(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}
#endif
#endif

	namespace detail {
		template<typename S, typename V>
		_PLUGIFY_ALWAYS_INLINE S to_string(V v) {
			//  numeric_limits::digits10 returns value less on 1 than desired for unsigned numbers.
			//  For example, for 1-byte unsigned value digits10 is 2 (999 can not be represented),
			//  so we need +1 here.
			constexpr size_t bufSize = std::numeric_limits<V>::digits10 + 2; // +1 for minus, +1 for digits10
			char buf[bufSize];
			const auto res = std::to_chars(buf, buf + bufSize, v);
			return S(buf, res.ptr);
		}

		typedef int (*wide_printf)(wchar_t* __restrict, size_t, const wchar_t* __restrict, ...);

#if defined(_MSC_VER)
		inline int truncate_snwprintf(wchar_t* __restrict buffer, size_t count, const wchar_t* __restrict format, ...) {
			int r;
			va_list args;
			va_start(args, format);
			r = _vsnwprintf_s(buffer, count, _TRUNCATE, format, args);
			va_end(args);
			return r;
		}
#endif

		constexpr wide_printf get_swprintf() {
#if defined(_MSC_VER)
			return static_cast<int(__cdecl*)(wchar_t* __restrict, size_t, const wchar_t* __restrict, ...)>(truncate_snwprintf);
#else
			return swprintf;
#endif
		}

		template<typename S, typename P, typename V>
		_PLUGIFY_ALWAYS_INLINE S as_string(P sprintf_like, const typename S::value_type* fmt, V v) {
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

#if PLUGIFY_STRING_FLOAT
	inline string to_string(float val) { return detail::as_string<string>(snprintf, "%f", val); }
	inline string to_string(double val) { return detail::as_string<string>(snprintf, "%f", val); }

#if PLUGIFY_STRING_LONG_DOUBLE
	inline string to_string(long double val) { return detail::as_string<string>(snprintf, "%Lf", val); }
#endif
#endif

	inline wstring to_wstring(int val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(long long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned long long val) { return detail::to_string<wstring>(val); }

#if PLUGIFY_STRING_FLOAT
	inline wstring to_wstring(float val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%f", val); }
	inline wstring to_wstring(double val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%f", val); }

#if PLUGIFY_STRING_LONG_DOUBLE
	inline wstring to_wstring(long double val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%Lf", val); }
#endif
#endif
#endif

#if PLUGIFY_STRING_STD_HASH
	// hash support
	namespace detail {
		template<typename Char, typename Allocator, typename String = basic_string<Char, std::char_traits<Char>, Allocator>>
		struct string_hash_base {
			[[nodiscard]] constexpr std::size_t operator()(const String& str) const noexcept {
				return std::hash<typename String::sview_type>{}(typename String::sview_type(str));
			}
		};
	}// namespace detail
#endif

#if PLUGIFY_STRING_STD_FORMAT
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
		}

		template<typename Char, typename Allocator, typename String = basic_string<Char, std::char_traits<Char>, Allocator>>
		struct string_formatter_base {
			constexpr auto parse(std::format_parse_context& ctx) {
				return ctx.begin();
			}

			auto format(const String& str, std::format_context& ctx) const {
				return std::format_to(ctx.out(), format_string<Char>(), str.c_str());
			}
		};
	}
#endif

	inline namespace literals {
		inline namespace string_literals {
			_PLUGIFY_STRING_DIAG_PUSH()

#if defined(__clang__)
			_PLUGIFY_STRING_DIAG_IGN("-Wuser-defined-literals")
#elif defined(__GNUC__)
			_PLUGIFY_STRING_DIAG_IGN("-Wliteral-suffix")
#elif defined(_MSC_VER)
			_PLUGIFY_STRING_DIAG_IGN(4455)
#endif
			// suffix for basic_string literals
			constexpr string operator""s(const char* str, std::size_t len) { return string{str, len}; }
			constexpr u8string operator""s(const char8_t* str, std::size_t len) { return u8string{str, len}; }
			constexpr u16string operator""s(const char16_t* str, std::size_t len) { return u16string{str, len}; }
			constexpr u32string operator""s(const char32_t* str, std::size_t len) { return u32string{str, len}; }
			constexpr wstring operator""s(const wchar_t* str, std::size_t len) { return wstring{str, len}; }

			_PLUGIFY_STRING_DIAG_POP()
		}// namespace string_literals
	}// namespace literals
}// namespace plg

#if PLUGIFY_STRING_STD_HASH
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
#endif

#if PLUGIFY_STRING_STD_FORMAT
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
#endif