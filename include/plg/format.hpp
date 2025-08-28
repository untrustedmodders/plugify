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

#include <type_traits>
#include <string>

namespace plg {
	namespace detail {
		// Concept to match string-like types including char* and const char*
		template<typename T>
		concept is_string_like = requires(T v) {
			{ std::string_view(v) };
		};
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

		auto in = std::back_inserter(result);

		// Second pass: actual formatting
		/*if (it != end)*/ {
			std::format_to(in, "{}", *it++);
		}
		while (it != end) {
			std::format_to(in, "{}{}", separator, *it++);
		}

		return result;
	}

	template<typename Range, typename Proj>
	constexpr std::string join(const Range& range, Proj&& proj, std::string_view separator) {
		std::string result;

		auto it = range.begin();
		auto end = range.end();

		if (it == end) return result;

		// First pass: compute total size
		size_t total_size = 0;
		size_t count = 0;

		for (auto tmp = it; tmp != end; ++tmp) {
			auto&& projected = std::invoke(std::forward<Proj>(proj), *tmp);
			using Elem = std::decay_t<decltype(projected)>;

			if constexpr (detail::is_string_like<Elem>) {
				total_size += std::string_view(*projected).size();
			} else {
				total_size += std::formatted_size("{}", projected);
			}
			++count;
		}
		if (count > 1) {
			total_size += (count - 1) * separator.size();
		}
		result.reserve(total_size);

		auto out = std::back_inserter(result);

		// Second pass: actual formatting
		{
			auto&& projected = std::invoke(std::forward<Proj>(proj), *it++);
			std::format_to(out, "{}", projected);
		}
		while (it != end) {
			auto&& projected = std::invoke(std::forward<Proj>(proj), *it++);
			std::format_to(out, "{}{}", separator, projected);
		}

		return result;
	}
}
