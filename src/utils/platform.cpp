#include "platform.hpp"
#include "os.h"

#if PLUGIFY_PLATFORM_WINDOWS
std::optional<std::wstring> plugify::GetEnvVariable(const wchar_t* varName) {
	DWORD size = GetEnvironmentVariableW(varName, NULL, 0);
	std::wstring buffer(size, 0);
	GetEnvironmentVariableW(varName, buffer.data(), size);
	return { std::move(buffer) };
}

bool plugify::SetEnvVariable(const wchar_t* varName, const wchar_t* value) {
	return SetEnvironmentVariableW(varName, value) != 0;
}
#else
std::optional<std::string> plugify::GetEnvVariable(const char* varName) {
	char* val = getenv(varName);
	if (!val)
		return {};
	return val;
}

bool plugify::SetEnvVariable(const char* varName, const char* value) {
	return setenv(varName, value, 1) == 0;
}

bool plugify::UnsetEnvVariable(const char* varName) {
	return unsetenv(varName) == 0;
}
#endif // PLUGIFY_PLATFORM_WINDOWS
