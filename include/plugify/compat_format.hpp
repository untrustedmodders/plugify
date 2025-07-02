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
	using namespace fmt;
	using namespace fmt::detail;
}

#endif // PLUGIFY_FORMAT_SUPPORT

namespace plg {
	template<typename Range>
	std::string join(const Range& range, std::string_view separator) {
		std::string result;
		auto it = range.begin();
		if (it != range.end()) {
			std::format_to(std::back_inserter(result), "{}", *it++);
		}
		while (it != range.end()) {
			std::format_to(std::back_inserter(result), "{}{}", separator, *it++);
		}
		return result;
	}
}
