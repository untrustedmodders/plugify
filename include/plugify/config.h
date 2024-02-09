#pragma once

#include <plugify/log.h>
#include <filesystem>
#include <vector>

namespace plugify {
	struct Config {
		std::filesystem::path baseDir;
		Severity logSeverity{ Severity::Verbose };
		std::vector<std::string> repositories;
	};
} // namespace plugify