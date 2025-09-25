#pragma once

#include <concepts>
#include <memory>
#include <type_traits>

#if PLUGIFY_HAS_CXX23
#  include <ranges>
#endif

namespace plg {
#if PLUGIFY_HAS_CXX23
	template<typename Range, typename Type>
	concept container_compatible_range = std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, Type>;
#endif

	template <typename Alloc>
	concept is_allocator =
	// basic nested types (via allocator_traits)
	requires {
		typename std::allocator_traits<Alloc>::value_type;
		typename std::allocator_traits<Alloc>::pointer;
		typename std::allocator_traits<Alloc>::const_pointer;
		typename std::allocator_traits<Alloc>::void_pointer;
		typename std::allocator_traits<Alloc>::const_void_pointer;
		typename std::allocator_traits<Alloc>::size_type;
		typename std::allocator_traits<Alloc>::difference_type;
	} &&
	// required expressions / member calls (use allocator_traits helpers where appropriate)
	requires(
		Alloc& a,
		typename std::allocator_traits<Alloc>::size_type n,
		typename std::allocator_traits<Alloc>::pointer p,
		typename std::allocator_traits<Alloc>::value_type& v,
		const typename std::allocator_traits<Alloc>::value_type& cv
	) {
		// allocation / deallocation
		{ a.allocate(n) } -> std::same_as<typename std::allocator_traits<Alloc>::pointer>;
		{ a.deallocate(p, n) } -> std::same_as<void>;

		// max_size: prefer allocator_traits::max_size (calls member or fallback)
		{ std::allocator_traits<Alloc>::max_size(a) } -> std::convertible_to<typename std::allocator_traits<Alloc>::size_type>;

		// construct / destroy (via allocator_traits helpers; these must be well-formed)
		{ std::allocator_traits<Alloc>::construct(a, p, cv) } -> std::same_as<void>;
		{ std::allocator_traits<Alloc>::destroy(a, p) } -> std::same_as<void>;

		// optional helpful factory used by containers when copying them
		{ std::allocator_traits<Alloc>::select_on_container_copy_construction(a) } -> std::convertible_to<Alloc>;
	};

	template <typename Traits>
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

	// A type is trivially relocatable if a move construct + destroy of the original object is equivalent to
	// `memcpy(dst, src, sizeof(T))`.
	//
	// Note that we don't use the __cpp_lib_trivially_relocatable Clang builtin right now because it does not
	// implement the semantics of any current or future trivial relocation proposal and it can lead to
	// incorrect optimizations on some platforms (Windows) and supported compilers (AppleClang).
#if __has_builtin(__cpp_lib_trivially_relocatable)
	template <class T, class = void>
	struct is_trivially_relocatable : std::integral_constant<bool, std::is_trivially_relocatable(T)> {};
#else
	template <class T, class = void>
	struct is_trivially_relocatable : std::is_trivially_copyable<T> {};
#endif

	template <class T> requires(std::is_same_v<T, typename T::trivially_relocatable>)
	struct is_trivially_relocatable<T> : std::true_type {};
}
