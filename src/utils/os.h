// This file is used to not include os specific functions that might break other projects
// You should use it in sources

#if PLUGIFY_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif

// require Win10+
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT _WIN32_WINNT_WIN10

#include <windows.h>
#undef ERROR

#elif PLUGIFY_PLATFORM_LINUX

#include <dlfcn.h>

#elif PLUGIFY_PLATFORM_APPLE

#include <dlfcn.h>

#else

#error "Platform is not supported!"

#endif