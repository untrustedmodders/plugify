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

#if PLUGIFY_PLATFORM_WINDOWS
		/// Converts the specified UTF-8 string to a wide string.
		static std::wstring ConvertUtf8ToWide(std::string_view str);
		static bool ConvertUtf8ToWide(std::wstring& dest, std::string_view str);

		/// Converts the specified wide string to a UTF-8 string.
		static std::string ConvertWideToUtf8(std::wstring_view str);
		static bool ConvertWideToUtf8(std::string& dest, std::wstring_view str);
#endif // PLUGIFY_PLATFORM_WINDOWS

		template<typename Cnt, typename Pr = std::equal_to<typename Cnt::value_type>>
		static bool RemoveDuplicates(Cnt &cnt, Pr cmp = Pr()) {
			auto size = std::size(cnt);
			Cnt result;
			result.reserve(size);

			std::copy_if(
				std::make_move_iterator(std::begin(cnt)),
				std::make_move_iterator(std::end(cnt)),
				std::back_inserter(result),
				[&](const typename Cnt::value_type& what) {
					return std::find_if(std::begin(result), std::end(result), [&](const typename Cnt::value_type& existing) {
						return cmp(what, existing);
					}) == std::end(result);
				}
			);

			cnt = std::move(result);
			return std::size(cnt) != size;
		}
	};
}
