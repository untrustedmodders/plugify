#pragma once

#include "plg/macro.hpp"

#ifdef __cpp_lib_format

#include <format>

#else // __cpp_lib_format

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
}

#endif // __cpp_lib_format
