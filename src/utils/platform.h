#pragma once

namespace plugify {
#if PLUGIFY_PLATFORM_WINDOWS
	std::optional<std::string> GetEnvVariable(std::string_view varName);
	bool SetEnvVariable(std::string_view varName, std::string_view value);
#else
	std::optional<std::wstring> GetEnvVariable(std::wstring_view varName);
	bool SetEnvVariable(std::wstring_view varName, std::wstring_view value);
#endif
}