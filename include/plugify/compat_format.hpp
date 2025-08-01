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
		auto end = range.end();

		// First pass: compute total size
		size_t total_size = 0;
		size_t count = 0;

		for (auto tmp = it; tmp != end; ++tmp) {
			using Elem = std::decay_t<decltype(*tmp)>;
			if constexpr (std::is_convertible_v<Elem, std::string_view>) {
				total_size += tmp->size();
			} else {
				total_size += std::formatted_size("{}", *tmp);
			}
			++count;
		}
		if (count > 1) {
			total_size += (count - 1) * separator.size();
		}
		result.reserve(total_size);

		// Second pass: actual formatting
		if (it != end) {
			std::format_to(std::back_inserter(result), "{}", *it++);
		}
		while (it != end) {
			std::format_to(std::back_inserter(result), "{}{}", separator, *it++);
		}

		return result;
	}
}
