#pragma once

namespace plugify {
	class String {
	public:
		String() = delete;

		static int Strncasecmp(const char* s1, const char* s2, size_t n) {
#if PLUGIFY_COMPILER_MSVC
			return _strnicmp(s1, s2, n);
#else
			return strncasecmp(s1, s2, n);
#endif // PLUGIFY_COMPILER_MSVC
		}

		static bool IsValidURL(std::string_view url);

		static std::string_view Trim(std::string_view sv) {
			while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t' || sv.front() == '\r' || sv.front() == '\n')) {
				sv.remove_prefix(1);
			}
			while (!sv.empty() && (sv.back() == ' ' || sv.back() == '\t' || sv.back() == '\r' || sv.back() == '\n')) {
				sv.remove_suffix(1);
			}
			return sv;
		}

#if PLUGIFY_PLATFORM_WINDOWS
		/// Converts the specified UTF-8 string to a wide string.
		static std::wstring ConvertUtf8ToWide(std::string_view str);
		static bool ConvertUtf8ToWide(std::wstring& dest, std::string_view str);

		/// Converts the specified wide string to a UTF-8 string.
		static std::string ConvertWideToUtf8(std::wstring_view str);
		static bool ConvertWideToUtf8(std::string& dest, std::wstring_view str);
#endif // PLUGIFY_PLATFORM_WINDOWS
	};
}
