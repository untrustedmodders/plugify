#pragma once

#ifdef PLUGIFY_DEBUG
#if PLUGIFY_PLATFORM_WINDOWS
	#define PL_DEBUGBREAK() __debugbreak()
#elif PLUGIFY_PLATFORM_LINUX || PLUGIFY_PLATFORM_APPLE
	#include <signal.h>
	#define PL_DEBUGBREAK() raise(SIGTRAP)
#else
	#error "Platform doesn't support debugbreak yet!"
#endif
	#define PLUGIFY_ENABLE_ASSERTS
#else
	#define PL_DEBUGBREAK()
#endif

#ifdef PLUGIFY_ENABLE_ASSERTS
	#define PL_EXPAND_MACRO(x) x
	#define PL_STRINGIFY_MACRO(x) #x

	#define PL_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { PL_LOG_##type(msg, __VA_ARGS__); PL_DEBUGBREAK(); } }
	#define PL_INTERNAL_ASSERT_WITH_MSG(type, check, ...) PL_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define PL_INTERNAL_ASSERT_NO_MSG(type, check) PL_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", PL_STRINGIFY_MACRO(check), fs::path{__FILE__}.filename().string(), __LINE__)

	#define PL_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define PL_INTERNAL_ASSERT_GET_MACRO(...) PL_EXPAND_MACRO( PL_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, PL_INTERNAL_ASSERT_WITH_MSG, PL_INTERNAL_ASSERT_NO_MSG) )

	#define PL_ASSERT(...) PL_EXPAND_MACRO( PL_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(ERROR, __VA_ARGS__) )
	#define PL_FATAL_ASSERT(...) PL_EXPAND_MACRO( PL_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(FATAL, __VA_ARGS__) )
#else
	#define PL_ASSERT(...)
	#define PL_FATAL_ASSERT(...)
#endif
