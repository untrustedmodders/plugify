#pragma once

#if PLUGIFY_CORE
#  define PLUGIFY_ACCESS public
#else
#  define PLUGIFY_ACCESS private
#endif

#include "plugify_export.h"

#ifdef _MSC_VER
    #define PLUGIFY_NO_DLL_EXPORT_WARNING(declaration) \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4251)) \
    declaration \
    __pragma(warning(pop))
#else
    #define PLUGIFY_NO_DLL_EXPORT_WARNING(declaration) \
    declaration
#endif
