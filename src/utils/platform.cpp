#include "platform.h"
#include "os.h"

using namespace plugify;

#if PLUGIFY_PLATFORM_WINDOWS
std::optional<std::wstring> GetEnvVariable(std::wstring_view varName) {
	DWORD size = GetEnvironmentVariableW(varName.data(), NULL, 0);
	std::wstring buffer(size, 0);
	GetEnvironmentVariableW(varName.data(), buffer.data(), size);
	return { std::move(buffer) };
}

bool SetEnvVariable(std::wstring_view varName, std::wstring_view value) {
	return SetEnvironmentVariableW(varName.data(), value.data()) != 0;
}
#else
std::optional<std::string> GetEnvVariable(std::string_view varName) {
	char* val = getenv(varName.data());
	if (!val)
		return {};
	return val;
}

bool SetEnvVariable(std::string_view varName, std::string_view value) {
	return setenv(varName.data(), value.data(), 1) == 0;
}
#endif