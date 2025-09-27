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

#ifndef __builtin_constant_p
#  define __builtin_constant_p(x) std::is_constant_evaluated()
#endif

#if __has_include(<version>)
#  include <version>
#endif

#if __has_include(<cassert>)
#  include <cassert>
#  define PLUGIFY_ASSERT(cond, mesg) assert((cond) && (mesg))
#endif

#define PLUGIFY_HAS_EXCEPTIONS  __cpp_exceptions || _CPPUNWIND || __EXCEPTIONS
#if PLUGIFY_HAS_EXCEPTIONS
#  include <stdexcept>
#  include <type_traits>
namespace plg {
	template<typename E, typename... Args>
	[[noreturn]] constexpr void throw_exception(const char* msg, Args...args) {
		if constexpr (std::is_constructible_v<E, const char*>) {
			throw E(msg);
		} else {
			throw E(std::forward<Args>(args)...);
		}
	}
} // namespace plg
#  define PLUGIFY_THROW(str, exp, ...) ::plg::throw_exception<exp>(str, ##__VA_ARGS__);
#else
#  include <cstdlib>
#  include <cstdio>
#  define PLUGIFY_THROW(str, ...) \
	std::fputs(str "\n", stderr); \
	std::abort();
#endif

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#  define PLUGIFY_INSTRUMENTED_WITH_ASAN 1
#  include <sanitizer/asan_interface.h>
#else
#  define PLUGIFY_INSTRUMENTED_WITH_ASAN 0
#endif

#  define PLUGIFY_COMPILER_MAKE_VERSION2(version, sp)         ((version) * 100 + (sp))
#  define PLUGIFY_COMPILER_MAKE_VERSION3(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))

#if defined(__clang__)

#  ifndef PLUGIFY_COMPILER_CLANG
#    define PLUGIFY_COMPILER_CLANG 1
#  endif

#  define PLUGIFY_COMPILER_CLANG_VERSION              PLUGIFY_COMPILER_MAKE_VERSION3(__clang_major__, __clang_minor__, __clang_patchlevel__)
#  define PLUGIFY_CLANG_AT_LEAST(major, minor, patch) (PLUGIFY_COMPILER_CLANG_VERSION >= PLUGIFY_COMPILER_MAKE_VERSION3((major), (minor), (patch)))
#  define PLUGIFY_CLANG_BEFORE(major, minor, patch)   (PLUGIFY_COMPILER_CLANG_VERSION < PLUGIFY_COMPILER_MAKE_VERSION3((major), (minor), (patch)))

#if PLUGIFY_CLANG_BEFORE(15, 0, 0)
#  error "clang version 15 required"
#endif

#elif defined(__GNUC__)

#  ifndef PLUGIFY_COMPILER_GCC
#    define PLUGIFY_COMPILER_GCC 1
#  endif

#  define PLUGIFY_COMPILER_GCC_VERSION              PLUGIFY_COMPILER_MAKE_VERSION3(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#  define PLUGIFY_GCC_AT_LEAST(major, minor, patch) (PLUGIFY_COMPILER_GCC_VERSION >= PLUGIFY_COMPILER_MAKE_VERSION3((major), (minor), (patch)))
#  define PLUGIFY_GCC_BEFORE(major, minor, patch)   (PLUGIFY_COMPILER_GCC_VERSION < PLUGIFY_COMPILER_MAKE_VERSION3((major), (minor), (patch)))

#if PLUGIFY_GCC_BEFORE(11, 1, 0)
#  error "GCC version 11.1 required"
#endif

#elif defined(_MSC_VER)

#  ifndef PLUGIFY_COMPILER_MSVC
#    define PLUGIFY_COMPILER_MSVC 1
#  endif

#if (_MSC_VER >= 1943)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 13)
#elif (_MSC_VER >= 1942)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 12)
#elif (_MSC_VER >= 1941)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 11)
#elif (_MSC_VER >= 1940)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 10)
#elif (_MSC_VER >= 1939)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 9)
#elif (_MSC_VER >= 1938)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 8)
#elif (_MSC_VER >= 1937)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 7)
#elif (_MSC_VER >= 1936)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 6)
#elif (_MSC_VER >= 1935)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 5)
#elif (_MSC_VER >= 1934)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 4)
#elif (_MSC_VER >= 1933)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 3)
#elif (_MSC_VER >= 1932)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 2)
#elif (_MSC_VER >= 1931)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 1)
#elif (_MSC_VER >= 1930)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2022, 0)
#elif (_MSC_VER >= 1929)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 10)
#elif (_MSC_VER >= 1928)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 8)
#elif (_MSC_VER >= 1927)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 7)
#elif (_MSC_VER >= 1926)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 6)
#elif (_MSC_VER >= 1925)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 5)
#elif (_MSC_VER >= 1924)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 4)
#elif (_MSC_VER >= 1923)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 3)
#elif (_MSC_VER >= 1922)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 2)
#elif (_MSC_VER >= 1921)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 1)
#elif (_MSC_VER >= 1920)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2019, 0)
#elif (_MSC_VER >= 1916)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2017, 9)
#elif (_MSC_VER >= 1915)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2017, 8)
#elif (_MSC_VER >= 1914)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2017, 7)
#elif (_MSC_VER >= 1913)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2017, 6)
#elif (_MSC_VER >= 1912)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2017, 5)
#elif (_MSC_VER >= 1911)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2017, 3)
#elif (_MSC_VER >= 1910)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2017, 0)
#elif (_MSC_VER >= 1900) && defined(_MSVC_LANG)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2015, 3)
#elif (_MSC_VER >= 1900)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2015, 0)
#elif (_MSC_VER >= 1800)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2013, 0)
#elif (_MSC_VER >= 1700)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2012, 0)
#elif (_MSC_VER >= 1600)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2010, 0)
#elif (_MSC_VER >= 1500)
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2008, 0)
#else
#  define PLUGIFY_COMPILER_MSVC_VERSION PLUGIFY_COMPILER_MAKE_VERSION2(2005, 0)
#endif
#define PLUGIFY_MSVC_AT_LEAST(version, sp) (PLUGIFY_COMPILER_MSVC_VERSION >= PLUGIFY_COMPILER_MAKE_VERSION2((version), (sp)))
#define PLUGIFY_MSVC_BEFORE(version, sp)   (PLUGIFY_COMPILER_MSVC_VERSION < PLUGIFY_COMPILER_MAKE_VERSION2((version), (sp)))

#if PLUGIFY_MSVC_BEFORE(2022, 9)
#error "MSVC version 2022 17.13.0 required"
#endif

#endif

// Define version detection for MSVC (_MSVC_LANG) and GCC/Clang (__cplusplus)
#if defined(_MSC_VER) && defined(_MSVC_LANG)
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

#define PLUGIFY_PRAGMA_IMPL(x) _Pragma(#x)
#define PLUGIFY_PRAGMA(x) PLUGIFY_PRAGMA_IMPL(x)

#if PLUGIFY_COMPILER_CLANG
#  define PLUGIFY_PRAGMA_DIAG_PREFIX clang
#elif PLUGIFY_COMPILER_GCC
#  define PLUGIFY_PRAGMA_DIAG_PREFIX GCC
#endif

#if PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_CLANG
#  define PLUGIFY_WARN_PUSH() PLUGIFY_PRAGMA(PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic push)
#  define PLUGIFY_WARN_IGNORE(wrn) PLUGIFY_PRAGMA(PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic ignored wrn)
#  define PLUGIFY_WARN_POP() PLUGIFY_PRAGMA(PLUGIFY_PRAGMA_DIAG_PREFIX diagnostic pop)
#elif PLUGIFY_COMPILER_MSVC
#  define PLUGIFY_WARN_PUSH()	__pragma(warning(push))
#  define PLUGIFY_WARN_IGNORE(wrn) __pragma(warning(disable: wrn))
#  define PLUGIFY_WARN_POP() __pragma(warning(pop))
#endif

#if PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_CLANG
#  define PLUGIFY_PACK(decl) decl __attribute__((__packed__))
#elif PLUGIFY_COMPILER_MSVC
#  define PLUGIFY_PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#  define PLUGIFY_PACK(decl) decl
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1929
#  define PLUGIFY_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#  define PLUGIFY_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#if PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_CLANG
#  define PLUGIFY_UNREACHABLE() __builtin_unreachable()
#elif PLUGIFY_COMPILER_MSVC
#  define PLUGIFY_UNREACHABLE() __assume(false)
#else
#  define PLUGIFY_UNREACHABLE()
#endif

#if PLUGIFY_COMPILER_CLANG
#  define PLUGIFY_FORCE_INLINE [[gnu::always_inline]] [[gnu::gnu_inline]] extern inline
#  define PLUGIFY_NOINLINE [[gnu::noinline]]
#elif PLUGIFY_COMPILER_GCC
#  define PLUGIFY_FORCE_INLINE [[gnu::always_inline]] inline
#  define PLUGIFY_NOINLINE [[gnu::noinline]]
#elif PLUGIFY_COMPILER_MSVC
#  pragma warning(error: 4714)
#  define PLUGIFY_FORCE_INLINE [[msvc::forceinline]]
#  define PLUGIFY_NOINLINE [[msvc::noinline]]
#else
#  define PLUGIFY_FORCE_INLINE inline
#  define PLUGIFY_NOINLINE
#endif

#if __has_feature(nullability)
#  define PLUGIFY_NO_NULL _Nonnull
#else
#  define PLUGIFY_NO_NULL
#endif

#if __has_cpp_attribute(__no_sanitize__)
#  define PLUGIFY_NO_CFI __attribute__((__no_sanitize__("cfi")))
#else
#  define PLUGIFY_NO_CFI
#endif

#if PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_CLANG
#  define PLUGIFY_RESTRICT __restrict__
#elif PLUGIFY_COMPILER_MSVC
#  define PLUGIFY_RESTRICT __restrict
#else
#  define PLUGIFY_RESTRICT
#endif

#ifndef PLUGIFY_PLATFORM_WINDOWS
#  if defined(_WIN32) || defined(_WIN64)
#    define PLUGIFY_PLATFORM_WINDOWS 1
#  endif
#endif

#ifndef PLUGIFY_PLATFORM_APPLE
#  if defined(__APPLE__) && defined(__MACH__)
#    define PLUGIFY_PLATFORM_APPLE 1
#  endif
#endif

#ifndef PLUGIFY_PLATFORM_LINUX
#  if defined(__linux__)
#    define PLUGIFY_PLATFORM_LINUX 1
#  endif
#endif

#ifndef PLUGIFY_PLATFORM_ANDROID
#  if defined(__ANDROID__)
#    define PLUGIFY_PLATFORM_ANDROID 1
#  endif
#endif

#ifndef PLUGIFY_PLATFORM_ORBIS
#  if defined(__ORBIS__)
#    define PLUGIFY_PLATFORM_ORBIS 1
#  endif
#endif

#ifndef PLUGIFY_PLATFORM_PROSPERO
#  if defined(__PROSPERO__)
#    define PLUGIFY_PLATFORM_PROSPERO 1
#  endif
#endif

#ifndef PLUGIFY_PLATFORM_SWITCH
#  if defined(__NX__)
#    define PLUGIFY_PLATFORM_SWITCH 1
#  endif
#endif

#ifndef PLUGIFY_PLATFORM_BSD
#  if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#    define PLUGIFY_PLATFORM_BSD 1
#  endif
#endif

#ifndef PLUGIFY_PLATFORM_UNIX
#  if defined(__unix__) || defined(__unix) || defined(unix) || defined(__APPLE__)
#    define PLUGIFY_PLATFORM_UNIX 1
#  endif
#endif

#if !defined(PLUGIFY_PLATFORM_WINDOWS)  && \
	!defined(PLUGIFY_PLATFORM_APPLE)   && \
	!defined(PLUGIFY_PLATFORM_LINUX)   && \
	!defined(PLUGIFY_PLATFORM_ANDROID) && \
	!defined(PLUGIFY_PLATFORM_ORBIS)   && \
	!defined(PLUGIFY_PLATFORM_PROSPERO)&& \
	!defined(PLUGIFY_PLATFORM_SWITCH)  && \
	!defined(PLUGIFY_PLATFORM_BSD)     && \
	!defined(PLUGIFY_PLATFORM_UNIX)
#  error "Unsupported platform! Please extend macro.hpp"
#endif
