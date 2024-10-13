#include "strings.h"
#include "os.h"

#include <regex>

using namespace plugify;

#if PLUGIFY_PLATFORM_WINDOWS
std::wstring String::ConvertUtf8ToWide(std::string_view str){
	std::wstring ret;
	if (!ConvertUtf8ToWide(ret, str))
		return {};
	return ret;
}

bool String::ConvertUtf8ToWide(std::wstring& dest, std::string_view str) {
	int wlen = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0);
	if (wlen < 0)
		return false;

	dest.resize(static_cast<size_t>(wlen));
	if (wlen > 0 && MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), dest.data(), wlen) < 0)
		return false;

	return true;
}

std::string String::ConvertWideToUtf8(std::wstring_view str) {
	std::string ret;
	if (!ConvertWideToUtf8(ret, str))
		return {};
	return ret;
}

bool String::ConvertWideToUtf8(std::string& dest, std::wstring_view str) {
	int mblen = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0, nullptr, nullptr);
	if (mblen < 0)
		return false;

	dest.resize(static_cast<size_t>(mblen));
	if (mblen > 0 && WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), dest.data(), mblen, nullptr, nullptr) < 0)
		return false;

	return true;
}
#endif // PLUGIFY_PLATFORM_WINDOWS

bool String::IsValidURL(std::string_view url) {
	static std::regex regex(R"(^((http[s]?|ftp):\/)?\/?([^:\/\s]+)((\/\w+)*\/)([\w\-\.]+[^#?\s]+)(.*)?(#[\w\-]+)?$)");
	return !url.empty() && std::regex_match(url.begin(), url.end(), regex);
}