#pragma once

namespace plugify {
#if PLUGIFY_PLATFORM_WINDOWS
	std::optional<std::wstring> GetEnvVariable(const wchar_t* varName);
	bool SetEnvVariable(const wchar_t* varName, const wchar_t* value);
#else
	std::optional<std::string> GetEnvVariable(const char* varName);
	bool SetEnvVariable(const char* varName, const char* value);
	bool UnsetEnvVariable(const char* varName);
#endif // PLUGIFY_PLATFORM_WINDOWS
}
