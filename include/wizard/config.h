#pragma once

#include <wizard/log.h>
#include <filesystem>
#include <vector>

namespace wizard {
	struct Config {
		std::filesystem::path baseDir;
		Severity logSeverity{ Severity::Verbose };
		bool packageVerification{ true };
		std::string packageVerifyUrl;
		std::vector<std::string> repositories;
	};
}