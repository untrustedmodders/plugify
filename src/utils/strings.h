#pragma once

namespace wizard {
	class String {
	public:
		String() = delete;

		static inline int Strncasecmp(const char* s1, const char* s2, std::size_t n) {
#ifdef _MSC_VER
			return _strnicmp(s1, s2, n);
#else
			return strncasecmp(s1, s2, n);
#endif
		}

#ifdef WIZARD_PLATFORM_WINDOWS
		/// Converts the specified UTF-8 string to a wide string.
		static std::wstring UTF8StringToWideString(std::string_view str);
		static bool UTF8StringToWideString(std::wstring& dest, std::string_view str);

		/// Converts the specified wide string to a UTF-8 string.
		static std::string WideStringToUTF8String(std::wstring_view str);
		static bool WideStringToUTF8String(std::string& dest, std::wstring_view str);
#endif
	};
}