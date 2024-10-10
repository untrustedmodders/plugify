// This file is used to not include os specific functions that might break other projects
// You should use it in sources
#pragma once

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

#include <Windows.h>
#undef ERROR

#elif PLUGIFY_PLATFORM_LINUX

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <link.h>
#include <sys/mman.h>
#include <sys/stat.h>

#elif PLUGIFY_PLATFORM_APPLE

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/nlist.h>
#include <sys/mman.h>
#include <sys/stat.h>

#elif PLUGIFY_PLATFORM_ORBIS || PLUGIFY_PLATFORM_PROSPERO

#include <kernel.h>

#elif PLUGIFY_PLATFORM_SWITCH

#include <nn/ro.h>
#include <nn/fs.h>
#include <nn/os.h>

#else

#error "Platform is not supported!"

#endif
