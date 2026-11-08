#pragma once

/**
 * @file global.h
 * @brief Macros every public header of the library relies on.
 */

/**
 * @def PLUGIFY_ACCESS
 * @brief Access level of the implementation details a PIMPL class carries.
 *
 * Expands to `public` while the library itself is being built, which is what
 * lets the implementation reach into the `Impl` struct and the pointer to it,
 * and to `private` for consumers, which keeps both out of the public API.
 */
#if PLUGIFY_CORE
#define PLUGIFY_ACCESS public
#else
#define PLUGIFY_ACCESS private
#endif

#include "plugify_export.h"

/**
 * @def PLUGIFY_NO_DLL_EXPORT_WARNING
 * @brief Declares a member whose type is not exported from the DLL.
 *
 * MSVC warns (C4251) about a std::unique_ptr member of an exported class, even
 * though the member is never touched from outside the library. This wraps the
 * declaration so the warning is suppressed for it alone, and is a plain
 * pass-through on every other compiler.
 *
 * @param declaration The member declaration to emit.
 */
#ifdef _MSC_VER
#define PLUGIFY_NO_DLL_EXPORT_WARNING(declaration) \
	__pragma(warning(push)) __pragma(warning(disable : 4251)) declaration __pragma(warning(pop))
#else
#define PLUGIFY_NO_DLL_EXPORT_WARNING(declaration) declaration
#endif
