#pragma once

#ifdef WIZARD_DEBUG
#if WIZARD_PLATFORM_WINDOWS
	#define WZ_DEBUGBREAK() __debugbreak()
#elif WIZARD_PLATFORM_LINUX
	#include <signal.h>
	#define WZ_DEBUGBREAK() raise(SIGTRAP)
#else
	#error "Platform doesn't support debugbreak yet!"
#endif
	#define WIZARD_ENABLE_ASSERTS
#else
	#define WZ_DEBUGBREAK()
#endif

#ifdef WIZARD_ENABLE_ASSERTS
	#define WZ_EXPAND_MACRO(x) x
	#define WZ_STRINGIFY_MACRO(x) #x

	#define WZ_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { WZ_LOG_##type(msg, __VA_ARGS__); WZ_DEBUGBREAK(); } }
	#define WZ_INTERNAL_ASSERT_WITH_MSG(type, check, ...) WZ_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define WZ_INTERNAL_ASSERT_NO_MSG(type, check) WZ_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", WZ_STRINGIFY_MACRO(check), fs::path{__FILE__}.filename().string(), __LINE__)

	#define WZ_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define WZ_INTERNAL_ASSERT_GET_MACRO(...) WZ_EXPAND_MACRO( WZ_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, WZ_INTERNAL_ASSERT_WITH_MSG, WZ_INTERNAL_ASSERT_NO_MSG) )

	#define WZ_ASSERT(...) WZ_EXPAND_MACRO( WZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(ERROR, __VA_ARGS__) )
	#define WZ_FATAL_ASSERT(...) WZ_EXPAND_MACRO( WZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(FATAL, __VA_ARGS__) )
#else
	#define WZ_ASSERT(...)
	#define WZ_FATAL_ASSERT(...)
#endif