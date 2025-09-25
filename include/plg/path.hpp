#pragma once

#include <cwctype>
#include <filesystem>
#include <string>

#include "plg/config.hpp"

namespace plg {
	using path_view = std::basic_string_view<std::filesystem::path::value_type>;
	using path_string = std::filesystem::path::string_type;
	using path_char = path_string::value_type;
	using path_diff_t = path_string::difference_type;

#if PLUGIFY_PLATFORM_WINDOWS
#define PLUGIFY_PATH_LITERAL(x) L##x
#else
#define PLUGIFY_PATH_LITERAL(x) x
#endif

	template <typename char_type>
	bool insensitive_equals(const path_char lhs, const path_char rhs) {
		if constexpr (std::is_same_v<char_type, wchar_t>) {
			return std::towlower(static_cast<std::wint_t>(lhs)) == std::towlower(static_cast<std::wint_t>(rhs));
		} else {
			return std::tolower(static_cast<unsigned char>(lhs)) == std::tolower(static_cast<unsigned char>(rhs));
		}
	}

	inline bool insensitive_equals(const path_view lhs, const path_view rhs) {
		return lhs.size() == rhs.size() && std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), insensitive_equals<path_char>);
	}

	inline path_view extension_view(const std::filesystem::path& path, size_t extension_size) {
		if (!path.has_extension()) {
			return {};
		}
		const path_string& path_str = path.native();
		const auto offset = static_cast<path_diff_t>(path_str.size() - extension_size);
		if (offset <= 0) {
			return {};
		}
		return { path_str.cbegin() + offset, path_str.cend() };  // NOLINT(*-dangling-handle)
	}

	inline bool has_extension(const std::filesystem::path& path, const path_view extension) {
		return insensitive_equals(extension_view(path, extension.size()), extension);
	}

	inline auto as_string(const std::filesystem::path& p) {
#if PLUGIFY_PLATFORM_WINDOWS
		return p.string();  // returns std::string by value
#else
		return p.native();  // returns const std::string&
#endif
	}
}
