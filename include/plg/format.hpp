#pragma once

#include "plg/config.hpp"

#if __has_include(<format>)
#include <format>
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
#define PLUGIFY_HAS_STD_FORMAT 1
#else
#define PLUGIFY_HAS_STD_FORMAT 0
#endif
#else
#define PLUGIFY_HAS_STD_FORMAT 0
#endif

#if !PLUGIFY_HAS_STD_FORMAT

// Define FMT_FORMAT_H externally to force a difference location for {fmt}
#ifndef FMT_FORMAT_H
#define FMT_FORMAT_H <fmt/format.h>
#endif

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include FMT_FORMAT_H

namespace std {
	using namespace fmt;
	using namespace fmt::detail;
} // namespace std

#endif // !PLUGIFY_HAS_STD_FORMAT
