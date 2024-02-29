#include "platform.h"
#include "os.h"

#if PLUGIFY_PLATFORM_WINDOWS
std::optional<std::wstring> plugify::GetEnvVariable(std::wstring_view varName) {
	DWORD size = GetEnvironmentVariableW(varName.data(), NULL, 0);
	std::wstring buffer(size, 0);
	GetEnvironmentVariableW(varName.data(), buffer.data(), size);
	return { std::move(buffer) };
}

bool plugify::SetEnvVariable(std::wstring_view varName, std::wstring_view value) {
	return SetEnvironmentVariableW(varName.data(), value.data()) != 0;
}
#else
std::optional<std::string> plugify::GetEnvVariable(std::string_view varName) {
	char* val = getenv(varName.data());
	if (!val)
		return {};
	return val;
}

bool plugify::SetEnvVariable(std::string_view varName, std::string_view value) {
	return setenv(varName.data(), value.data(), 1) == 0;
}
#endif