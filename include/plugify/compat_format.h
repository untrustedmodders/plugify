#pragma once

#if PLUGIFY_FORMAT_SUPPORT

#include <format>

#else // PLUGIFY_FORMAT_SUPPORT

// Define FMT_FORMAT_H externally to force a difference location for {fmt}
#ifndef FMT_FORMAT_H
#define FMT_FORMAT_H <fmt/format.h>
#endif

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include FMT_FORMAT_H

namespace std {
	using fmt::format;
	using fmt::format_to;
	using fmt::format_to_n;
}

#endif // PLUGIFY_FORMAT_SUPPORT