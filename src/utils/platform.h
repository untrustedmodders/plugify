#pragma once

#include "os.h"

namespace plugify {
	inline std::optional<std::string> getEnvVariable(std::string_view varName) {
#ifdef PLUGIFY_PLATFORM_WINDOWS
		char* buf = nullptr;
		size_t sz = 0;
		if (_dupenv_s(&buf, &sz, varName.data()) == 0 && buf != nullptr) {
			std::optional<std::string> val(buf);
			free(buf);
			return val;
		}
		return {};
#else
		char* val = getenv(varName.data());
		if (!val)
			return {};
		return val;
#endif
	}

	inline bool setEnvVariable(std::string_view varName, std::string_view value) {
#ifdef PLUGIFY_PLATFORM_WINDOWS
		return SetEnvironmentVariable(varName.data(), value.data()) != 0;
#else
		return setenv(varName.data(), value.data(), 1) == 0;
#endif
	}
}