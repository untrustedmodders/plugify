#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "plugify/core/log_system.hpp"
#include "plugify/core/types.hpp"

namespace plugify {
	struct Config {
		std::vector<std::filesystem::path> searchPaths;
		std::filesystem::path configsDir;
		std::filesystem::path dataDir;
		std::filesystem::path logsDir;

		std::optional<Severity> logSeverity;

		//std::optional<bool> preferOwnSymbols;
		//std::optional<bool> strictVersionChecking;
		//std::optional<bool> autoResolveConflicts;
		//std::optional<bool> loadDisabledPackages;

		std::optional<std::vector<PackageId>> whitelistedPackages;
		std::optional<std::vector<PackageId>> blacklistedPackages;
	};
} // namespace plugify
