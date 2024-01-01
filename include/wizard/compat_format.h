#pragma once

#if __has_include(<format>)

#include <format>

#else // __has_include(<format>)

// Define FMT_FORMAT_H externally to force a difference location for {fmt}
#ifndef FMT_FORMAT_H
#define FMT_FORMAT_H <fmt/format.h>
#endif

#define FMT_HEADER_ONLY
#include FMT_FORMAT_H

namespace std {
	using fmt::format;
	using fmt::format_to;
	using fmt::format_to_n;
}

#endif // __has_include(<format>)