#pragma once

#ifdef _WIN32
#if WIZARD_BUILD_SHARED_LIB
#define WIZARD_EXPORT __declspec(dllexport)
#define WIZARD_IMPORT __declspec(dllimport)
#else // WIZARD_BUILD_SHARED_LIB
#define WIZARD_EXPORT
#define WIZARD_IMPORT
#endif
#else // _WIN32
#if __GNUC__
#define WIZARD_EXPORT __attribute__((__visibility__("default")))
#else // __GNUC__
#define WIZARD_EXPORT
#endif // __GNUC__
#define WIZARD_IMPORT WIZARD_EXPORT
#endif // _WIN32

#ifdef WIZARD_BUILD_MAIN_LIB
#define WIZARD_API WIZARD_EXPORT
#else
#define WIZARD_API WIZARD_IMPORT
#endif
