#pragma once

#ifndef __has_cpp_attribute
#  define __has_cpp_attribute(x) 0
#endif
#ifndef __has_extension
#  define __has_extension(x) 0
#endif
#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#ifndef __has_include
#  define __has_include(x) 0
#endif
#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif

#define PLUGIFY_HAS_EXCEPTIONS (__cpp_exceptions || __EXCEPTIONS || _HAS_EXCEPTIONS)

#ifndef PLUGIFY_EXCEPTIONS
#  if PLUGIFY_HAS_EXCEPTIONS
#    define PLUGIFY_EXCEPTIONS 1
#  else
#    define PLUGIFY_EXCEPTIONS 0
#  endif
#endif

#if PLUGIFY_EXCEPTIONS && (!PLUGIFY_HAS_EXCEPTIONS || !__has_include(<stdexcept>))
#  undef PLUGIFY_EXCEPTIONS
#  define PLUGIFY_EXCEPTIONS 0
#endif

#ifndef PLUGIFY_FALLBACK_ASSERT
#  define PLUGIFY_FALLBACK_ASSERT 1
#endif

#if PLUGIFY_FALLBACK_ASSERT && !__has_include(<cassert>)
#  undef PLUGIFY_FALLBACK_ASSERT
#  define PLUGIFY_FALLBACK_ASSERT 0
#endif

#ifndef PLUGIFY_FALLBACK_ABORT
#  define PLUGIFY_FALLBACK_ABORT 1
#endif

#if PLUGIFY_FALLBACK_ABORT && !__has_include(<cstdlib>)
#  undef PLUGIFY_FALLBACK_ABORT
#  define PLUGIFY_FALLBACK_ABORT 0
#endif

#ifndef PLUGIFY_FALLBACK_ABORT_FUNCTION
#  define PLUGIFY_FALLBACK_ABORT_FUNCTION [] (auto) { }
#endif

#if PLUGIFY_EXCEPTIONS
#  include <stdexcept>
#  define PLUGIFY_ASSERT(x, str, e) do { if (!(x)) [[unlikely]] throw e(str); } while (0)
#elif PLUGIFY_FALLBACK_ASSERT
#  include <cassert>
#  define PLUGIFY_ASSERT(x, str, ...) assert(x && str)
#elif PLUGIFY_FALLBACK_ABORT
#  include <cstdlib>
#  define PLUGIFY_ASSERT(x, ...) do { if (!(x)) [[unlikely]] { std::abort(); } } while (0)
#else
#  define PLUGIFY_ASSERT(x, str, ...) do { if (!(x)) [[unlikely]] { PLUGIFY_FALLBACK_ABORT_FUNCTION (str); { while (true) { [] { } (); } } } } while (0)
#endif

#define PLUGIFY_PRAGMA_IMPL(x) _Pragma(#x)
#define PLUGIFY_PRAGMA(x) PLUGIFY_PRAGMA_IMPL(x)

#if defined(__clang__)
#  define PLUGIFY_PRAGMA_DIAG_PREFIX clang
#elif defined(__GNUC__)
#  define PLUGIFY_PRAGMA_DIAG_PREFIX GCC
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define PLUGIFY_WARN_PUSH() PLUGIFY_PRAGMA(PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic push)
#  define PLUGIFY_WARN_IGNORE(wrn) PLUGIFY_PRAGMA(PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic ignored wrn)
#  define PLUGIFY_WARN_POP() PLUGIFY_PRAGMA(PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic pop)
#elif defined(_MSC_VER)
#  define PLUGIFY_WARN_PUSH()	__pragma(warning(push))
#  define PLUGIFY_WARN_IGNORE(wrn) __pragma(warning(disable: wrn))
#  define PLUGIFY_WARN_POP() __pragma(warning(pop))
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define PLUGIFY_PACK(decl) decl __attribute__((__packed__))
#elif defined(_MSC_VER)
#  define PLUGIFY_PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#  define PLUGIFY_PACK(decl) decl
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define PLUGIFY_UNREACHABLE() __builtin_unreachable()
#elif defined (_MSC_VER)
#  define PLUGIFY_UNREACHABLE() __assume(false)
#else
#  error "Compiler not supported, please report an issue."
#endif

#if defined(_MSC_VER)
#  define PLUGIFY_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#  define PLUGIFY_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#if defined(__clang__)
#  define PLUGIFY_FORCE_INLINE [[gnu::always_inline]] [[gnu::gnu_inline]] extern inline
#elif defined(__GNUC__)
#  define PLUGIFY_FORCE_INLINE [[gnu::always_inline]] inline
#elif defined(_MSC_VER)
#  pragma warning(error: 4714)
#  define PLUGIFY_FORCE_INLINE __forceinline
#else
#  define PLUGIFY_FORCE_INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define PLUGIFY_RESTRICT __restrict__
#elif defined(_MSC_VER)
#  define PLUGIFY_RESTRICT __restrict
#else
#  define PLUGIFY_RESTRICT
#endif

#if __cplusplus >= 202002L
// Include it in your implimentation
// #  include <concepts>
// #  include <memory>

#  define PLUGIFY_INPUT_ITERATOR std::input_iterator
#  define PLUGIFY_CONSTRUCT_AT(ptr, ...) std::construct_at(ptr, __VA_ARGS__)
#else // !(__cplusplus >= 202002L)
#  define PLUGIFY_INPUT_ITERATOR typename
#  define PLUGIFY_CONSTRUCT_AT(ptr, ...) new (ptr) std::remove_reference_t<decltype(*ptr)>(__VA_ARGS__)
#endif // __cplusplus >= 202002L

#ifndef __cpp_char8_t
enum char8_t : unsigned char {};
#endif // __cpp_char8_t

#ifdef __cpp_lib_is_nothrow_convertible
#  define PLUGIFY_NOTTHROW_CONVERTIBLE(T, S) std::is_nothrow_convertible_v<T, S>
#else // !__cpp_lib_is_nothrow_convertible
#  define PLUGIFY_NOTTHROW_CONVERTIBLE(T, S) 0
#endif // __cpp_lib_is_nothrow_convertible

