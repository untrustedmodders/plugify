#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <plugify/api/log.hpp>
#include <plugify/core/types.hpp>

namespace plugify {
	struct Config {
		std::filesystem::path baseDir;
		std::filesystem::path configsDir;
		std::filesystem::path dataDir;
		std::filesystem::path logsDir;
		std::optional<Severity> logSeverity;
		std::optional<bool> preferOwnSymbols;

		std::optional<std::vector<PackageId>> whitelistedPackages;
		std::optional<std::vector<PackageId>> blacklistedPackages;
	};
} // namespace plugify
