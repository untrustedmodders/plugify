#include "strings.h"
#include "os.h"

using namespace plugify;

#if PLUGIFY_PLATFORM_WINDOWS
std::wstring String::UTF8StringToWideString(std::string_view str){
	std::wstring ret;
	if (!UTF8StringToWideString(ret, str))
		return {};
	return ret;
}

bool String::UTF8StringToWideString(std::wstring& dest, std::string_view str) {
	int wlen = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0);
	if (wlen < 0)
		return false;

	dest.resize(static_cast<size_t>(wlen));
	if (wlen > 0 && MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), dest.data(), wlen) < 0)
		return false;

	return true;
}

std::string String::WideStringToUTF8String(std::wstring_view str) {
	std::string ret;
	if (!WideStringToUTF8String(ret, str))
		return {};
	return ret;
}

bool String::WideStringToUTF8String(std::string& dest, std::wstring_view str) {
	int mblen = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0, nullptr, nullptr);
	if (mblen < 0)
		return false;

	dest.resize(static_cast<size_t>(mblen));
	if (mblen > 0 && WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), dest.data(), mblen, nullptr, nullptr) < 0)
		return false;

	return true;
}
#endif