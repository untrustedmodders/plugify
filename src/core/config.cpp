#include "plugify/config.hpp"

using namespace plugify;

void Config::MergeFrom(const Config& other, ConfigSource source) {
	// Only merge if the new source has equal or higher priority

	// Merge Paths
	if (source >= _sources.paths) {
		bool pathsChanged = false;

		// Always take baseDir if provided
		if (!other.paths.baseDir.empty()) {
			paths.baseDir = other.paths.baseDir;
			pathsChanged = true;
		}

		// Take other paths if they're customized
		if (other.paths.HasCustomExtensionsDir()) {
			paths.extensionsDir = other.paths.extensionsDir;
			pathsChanged = true;
		}
		if (other.paths.HasCustomConfigsDir()) {
			paths.configsDir = other.paths.configsDir;
			pathsChanged = true;
		}
		if (other.paths.HasCustomDataDir()) {
			paths.dataDir = other.paths.dataDir;
			pathsChanged = true;
		}
		if (other.paths.HasCustomLogsDir()) {
			paths.logsDir = other.paths.logsDir;
			pathsChanged = true;
		}
		if (other.paths.HasCustomCacheDir()) {
			paths.cacheDir = other.paths.cacheDir;
			pathsChanged = true;
		}

		if (pathsChanged) {
			_sources.paths = source;
		}
	}

	// Merge Loading
	if (source >= _sources.loading) {
		bool loadingChanged = false;

		if (other.loading.HasCustomPreferOwnSymbols()) {
			loading.preferOwnSymbols = other.loading.preferOwnSymbols;
			loadingChanged = true;
		}
		if (other.loading.HasCustomMaxConcurrentLoads()) {
			loading.maxConcurrentLoads = other.loading.maxConcurrentLoads;
			loadingChanged = true;
		}
		if (other.loading.HasCustomLoadTimeout()) {
			loading.loadTimeout = other.loading.loadTimeout;
			loadingChanged = true;
		}
		if (other.loading.HasCustomExportTimeout()) {
			loading.exportTimeout = other.loading.exportTimeout;
			loadingChanged = true;
		}
		if (other.loading.HasCustomStartTimeout()) {
			loading.startTimeout = other.loading.startTimeout;
			loadingChanged = true;
		}

		if (loadingChanged) {
			_sources.loading = source;
		}
	}

	// Merge Security
	if (source >= _sources.security) {
		bool securityChanged = false;

		// For sets, we have different merge strategies
		if (source == ConfigSource::Override) {
			// Override replaces entirely if non-empty
			if (other.security.HasWhitelist()) {
				security.whitelistedExtensions = other.security.whitelistedExtensions;
				securityChanged = true;
			}
			if (other.security.HasBlacklist()) {
				security.blacklistedExtensions = other.security.blacklistedExtensions;
				securityChanged = true;
			}
			if (other.security.HasExcluded()) {
				security.excludedDirs = other.security.excludedDirs;
				securityChanged = true;
			}
		} else {
			// Other sources merge/append
			if (other.security.HasWhitelist()) {
				security.whitelistedExtensions.insert(
					other.security.whitelistedExtensions.begin(),
					other.security.whitelistedExtensions.end()
				);
				securityChanged = true;
			}
			if (other.security.HasBlacklist()) {
				security.blacklistedExtensions.insert(
					other.security.blacklistedExtensions.begin(),
					other.security.blacklistedExtensions.end()
				);
				securityChanged = true;
			}
			if (other.security.HasExcluded()) {
				security.excludedDirs.insert(
					other.security.excludedDirs.begin(),
					other.security.excludedDirs.end()
				);
				securityChanged = true;
			}
		}

		if (securityChanged) {
			_sources.security = source;
		}
	}

	// Merge Logging
	if (source >= _sources.logging) {
		bool loggingChanged = false;

		if (other.logging.HasCustomSeverity()) {
			logging.severity = other.logging.severity;
			loggingChanged = true;
		}
		if (other.logging.HasCustomPrintReport()) {
			logging.printReport = other.logging.printReport;
			loggingChanged = true;
		}
		if (other.logging.HasCustomPrintLoadOrder()) {
			logging.printLoadOrder = other.logging.printLoadOrder;
			loggingChanged = true;
		}
		if (other.logging.HasCustomPrintDependencyGraph()) {
			logging.printDependencyGraph = other.logging.printDependencyGraph;
			loggingChanged = true;
		}
		if (other.logging.HasCustomPrintDigraphDot()) {
			logging.printDigraphDot = other.logging.printDigraphDot;
			loggingChanged = true;
		}
		if (other.logging.HasExportPath()) {
			logging.exportDigraphDot = other.logging.exportDigraphDot;
			loggingChanged = true;
		}

		if (loggingChanged) {
			_sources.logging = source;
		}
	}

	// Always resolve paths after merge
	paths.ResolveRelativePaths();
}

void Config::MergeField(std::string_view fieldPath, const Config& other, ConfigSource source) {
	// This allows merging specific fields only
	if (fieldPath == "paths") {
		if (source >= _sources.paths) {
			paths = other.paths;
			_sources.paths = source;
			paths.ResolveRelativePaths();
		}
	} else if (fieldPath == "loading") {
		if (source >= _sources.loading) {
			loading = other.loading;
			_sources.loading = source;
		}
	} else if (fieldPath == "security") {
		if (source >= _sources.security) {
			security = other.security;
			_sources.security = source;
		}
	} else if (fieldPath == "logging") {
		if (source >= _sources.logging) {
			logging = other.logging;
			_sources.logging = source;
		}
	}
}

Result<void> Config::Validate() const{
	if (paths.baseDir.empty()) {
		return MakeError("Base directory not set");
	}

	// Check for path collisions
	using pair = std::pair<std::string_view, const std::filesystem::path*>;
	std::array pathList = {
		pair{ "extensions", &paths.extensionsDir },
		pair{ "configs", &paths.configsDir },
		pair{ "data", &paths.dataDir },
		pair{ "logs", &paths.logsDir },
		pair{ "cache", &paths.cacheDir },
	};

	for (size_t i = 0; i < pathList.size(); ++i) {
		for (size_t j = i + 1; j < pathList.size(); ++j) {
			const auto& [iname, ipath] = pathList[i];
			const auto& [jname, jpath] = pathList[j];
			if (*ipath == *jpath) {
				return MakeError(
					"{} and {} directories are the same: {}",
					iname,
					jname,
					plg::as_string(*ipath)
				);
			}
		}
	}

	return {};
}

const std::unordered_set<std::filesystem::path>& Config::GetDefaultExcludedDirs() {
	static const std::unordered_set<std::filesystem::path> dirs = {
		PLUGIFY_PATH_LITERAL("disabled"),
		PLUGIFY_PATH_LITERAL(".git"),
		PLUGIFY_PATH_LITERAL(".svn"),
		PLUGIFY_PATH_LITERAL("temp"),
		PLUGIFY_PATH_LITERAL("tmp"),
	};
	return dirs;
}