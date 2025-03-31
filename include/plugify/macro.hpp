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
#  define PLUGIFY_ASSERT(x, str, ...) assert((x) && (str))
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

// Define version detection for MSVC (_MSVC_LANG) and GCC/Clang (__cplusplus)
#if defined(_MSC_VER)  &&defined(_MSVC_LANG)
#  define PLUGIFY_CPP_VERSION _MSVC_LANG
#else
#  define PLUGIFY_CPP_VERSION __cplusplus
#endif

// Check for C++ version and define corresponding macros
#if PLUGIFY_CPP_VERSION >= 202602L
#  define PLUGIFY_HAS_CXX26 1
#endif

#if PLUGIFY_CPP_VERSION >= 202302L
#  define PLUGIFY_HAS_CXX23 1
#endif

#if __has_cpp_attribute(likely)
#  define PLUGIFY_LIKELY [[likely]]
#else
#  define PLUGIFY_LIKELY
#endif

#if __has_cpp_attribute(unlikely)
#  define PLUGIFY_UNLIKELY [[unlikely]]
#else
#  define PLUGIFY_UNLIKELY
#endif

#if __has_cpp_attribute(nodiscard)
#  define PLUGIFY_NODISCARD [[nodiscard]]
#else
#  define PLUGIFY_NODISCARD
#endif

#if __has_cpp_attribute(nodiscard)
#  define PLUGIFY_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#else
#  define PLUGIFY_NODISCARD_MSG(msg) PLUGIFY_NODISCARD
#endif

#if __has_cpp_attribute(maybe_unused)
#  define PLUGIFY_MAYBE_UNUSED [[maybe_unused]]
#else
#  define PLUGIFY_MAYBE_UNUSED
#endif

#if __has_cpp_attribute(deprecated)
#  define PLUGIFY_DEPRECATED [[deprecated]]
#  define PLUGIFY_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#else
#  define PLUGIFY_DEPRECATED
#  define PLUGIFY_DEPRECATED_MSG(msg)
#endif

#if __has_cpp_attribute(fallthrough)
#  define PLUGIFY_FALLTHROUGH [[fallthrough]]
#else
#  define PLUGIFY_FALLTHROUGH
#endif

#if __has_cpp_attribute(noreturn)
#  define PLUGIFY_NORETURN [[noreturn]]
#else
#  define PLUGIFY_NORETURN
#endif

#if __has_cpp_attribute(carries_dependency)
#  define PLUGIFY_CARRIES_DEPENDENCY [[carries_dependency]]
#else
#  define PLUGIFY_CARRIES_DEPENDENCY
#endif

#if __has_cpp_attribute(indeterminate)
#  define PLUGIFY_INDETERMINATE [[indeterminate]]
#else
#  define PLUGIFY_INDETERMINATE
#endif

#if __has_cpp_attribute(no_unique_address)
#  define PLUGIFY_NO_UNIQUE_ADDRESS [[no_unique_address]]
#elif defined(_MSC_VER) && _MSC_VER >= 1929
#  define PLUGIFY_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#  error "Compiler not supported, please report an issue."
#endif

#if __has_cpp_attribute(assume)
#  define PLUGIFY_ASSUME(...) [[assume(__VA_ARGS__)]]
#elif defined(_MSC_VER) // Microsoft Visual C++
#  define PLUGIFY_ASSUME(...) __assume(__VA_ARGS__)
#elif defined(__clang__) // LLVMs
#  define PLUGIFY_ASSUME(...) __builtin_assume(__VA_ARGS__)
#elif defined(__GNUC__)
#if __GNUC__ >= 13
#  define PLUGIFY_ASSUME(...) __attribute__((__assume__(__VA_ARGS__)))
#else
#  define PLUGIFY_ASSUME(...) do { if (!(__VA_ARGS__)) __builtin_unreachable(); } while (false);
#endif
#else
#  define PLUGIFY_ASSUME(...) void(0);
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define PLUGIFY_UNREACHABLE() __builtin_unreachable()
#elif defined (_MSC_VER)
#  define PLUGIFY_UNREACHABLE() __assume(false)
#else
#  error "Compiler not supported, please report an issue."
#endif

#if defined(__clang__)
#  define PLUGIFY_FORCE_INLINE [[gnu::always_inline]] [[gnu::gnu_inline]] extern inline
#  define PLUGIFY_NOINLINE [[gnu::noinline]]
#elif defined(__GNUC__)
#  define PLUGIFY_FORCE_INLINE [[gnu::always_inline]] inline
#  define PLUGIFY_NOINLINE [[gnu::noinline]]
#elif defined(_MSC_VER)
#  pragma warning(error: 4714)
#  define PLUGIFY_FORCE_INLINE __forceinline
#  define PLUGIFY_NOINLINE __declspec(noinline)
#else
#  define PLUGIFY_FORCE_INLINE inline
#  define PLUGIFY_NOINLINE
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define PLUGIFY_RESTRICT __restrict__
#elif defined(_MSC_VER)
#  define PLUGIFY_RESTRICT __restrict
#else
#  define PLUGIFY_RESTRICT
#endif
