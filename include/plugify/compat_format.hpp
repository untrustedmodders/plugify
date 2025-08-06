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

#include <type_traits>
#include <string>

namespace plg {
	namespace detail {
		// Concept to match string-like types including char* and const char*
		template<typename T>
		concept is_string_like =
			std::same_as<std::remove_cvref_t<T>, std::string> ||
			std::convertible_to<T, std::string_view> ||
			std::is_same_v<std::remove_cvref_t<T>, char*> ||
			std::is_same_v<std::remove_cvref_t<T>, const char*>;
	}

	template<typename Range>
	constexpr std::string join(const Range& range, std::string_view separator) {
		std::string result;

		auto it = range.begin();
		auto end = range.end();

		if (it == end) return result;

		// First pass: compute total size
		size_t total_size = 0;
		size_t count = 0;

		for (auto tmp = it; tmp != end; ++tmp) {
			using Elem = std::decay_t<decltype(*tmp)>;
			if constexpr (detail::is_string_like<Elem>) {
				total_size += std::string_view(*tmp).size();
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
