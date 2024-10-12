#pragma once

namespace plugify {
	class String {
	public:
		String() = delete;

		static inline int Strncasecmp(const char* s1, const char* s2, size_t n) {
#if PLUGIFY_COMPILER_MSVC
			return _strnicmp(s1, s2, n);
#else
			return strncasecmp(s1, s2, n);
#endif // PLUGIFY_COMPILER_MSVC
		}

#if PLUGIFY_PLATFORM_WINDOWS
		/// Converts the specified UTF-8 string to a wide string.
		static std::wstring ConvertUtf8ToWide(std::string_view str);
		static bool ConvertUtf8ToWide(std::wstring& dest, std::string_view str);

		/// Converts the specified wide string to a UTF-8 string.
		static std::string ConvertWideToUtf8(std::wstring_view str);
		static bool ConvertWideToUtf8(std::string& dest, std::wstring_view str);
#endif
	};
}
