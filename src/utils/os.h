// This file is used to not include os specific functions that might break other projects
// You should use it in sources

#if WIZARD_PLATFORM_WINDOWS

#include <windows.h>
#undef ERROR

#elif WIZARD_PLATFORM_LINUX

#include <dlfcn.h>

#elif WIZARD_PLATFORM_APPLE

#include <dlfcn.h>

#else

#error "Platform is not supported!"

#endif