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

#define _PLUGIFY_HAS_EXCEPTIONS (__cpp_exceptions || __EXCEPTIONS || _HAS_EXCEPTIONS)

#ifndef PLUGIFY_EXCEPTIONS
#  if _PLUGIFY_HAS_EXCEPTIONS
#    define PLUGIFY_EXCEPTIONS 1
#  else
#    define PLUGIFY_EXCEPTIONS 0
#  endif
#endif

#if PLUGIFY_EXCEPTIONS && (!_PLUGIFY_HAS_EXCEPTIONS || !__has_include(<stdexcept>))
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
#  define _PLUGIFY_ASSERT(x, str, e) do { if (!(x)) [[unlikely]] throw e(str); } while (0)
#elif PLUGIFY_FALLBACK_ASSERT
#  include <cassert>
#  define _PLUGIFY_ASSERT(x, str, ...) assert(x && str)
#elif PLUGIFY_FALLBACK_ABORT
#  include <cstdlib>
#  define _PLUGIFY_ASSERT(x, ...) do { if (!(x)) [[unlikely]] { std::abort(); } } while (0)
#else
#  define _PLUGIFY_ASSERT(x, str, ...) do { if (!(x)) [[unlikely]] { PLUGIFY_FALLBACK_ABORT_FUNCTION (str); { while (true) { [] { } (); } } } } while (0)
#endif

#define _PLUGIFY_PRAGMA_IMPL(x) _Pragma(#x)
#define _PLUGIFY_PRAGMA(x) _PLUGIFY_PRAGMA_IMPL(x)

#if defined(__clang__)
#  define _PLUGIFY_PRAGMA_DIAG_PREFIX clang
#elif defined(__GNUC__)
#  define _PLUGIFY_PRAGMA_DIAG_PREFIX GCC
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_WARN_PUSH() _PLUGIFY_PRAGMA(_PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic push)
#  define _PLUGIFY_WARN_IGNORE(wrn) _PLUGIFY_PRAGMA(_PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic ignored wrn)
#  define _PLUGIFY_WARN_POP() _PLUGIFY_PRAGMA(_PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic pop)
#elif defined(_MSC_VER)
#  define _PLUGIFY_WARN_PUSH()	__pragma(warning(push))
#  define _PLUGIFY_WARN_IGNORE(wrn) __pragma(warning(disable: wrn))
#  define _PLUGIFY_WARN_POP() __pragma(warning(pop))
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_PACK(decl) decl __attribute__((__packed__))
#elif defined(_MSC_VER)
#  define _PLUGIFY_PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#  define _PLUGIFY_PACK(decl) decl
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_UNREACHABLE() __builtin_unreachable()
#elif defined (_MSC_VER)
#  define _PLUGIFY_UNREACHABLE() __assume(false)
#else
#  error "Compiler not supported, please report an issue."
#endif

#if defined(_MSC_VER)
#  define _PLUGIFY_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#  define _PLUGIFY_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#if defined(__clang__)
#  define _PLUGIFY_FORCE_INLINE [[gnu::always_inline]] [[gnu::gnu_inline]] extern inline
#elif defined(__GNUC__)
#  define _PLUGIFY_FORCE_INLINE [[gnu::always_inline]] inline
#elif defined(_MSC_VER)
#  pragma warning(error: 4714)
#  define _PLUGIFY_FORCE_INLINE __forceinline
#else
#  define _PLUGIFY_FORCE_INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_RESTRICT __restrict__
#elif defined(_MSC_VER)
#  define _PLUGIFY_RESTRICT __restrict
#else
#  define _PLUGIFY_RESTRICT
#endif
