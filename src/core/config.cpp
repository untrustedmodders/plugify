#include "plugify/config.hpp"

using namespace plugify;

namespace {
	// A merge has to tell "this source asked for it" from "this source left it
	// alone", and the fields are plain values rather than optionals - so anything
	// still equal to a freshly built Config counts as untouched. Comparing against
	// one instance of it keeps every default written down once, in the member
	// initializers, rather than a second time per field here.
	const Config& Defaults() {
		static const Config defaults;
		return defaults;
	}
}

void Config::MergeFrom(const Config& other, ConfigSource source) {
	const Config& defaults = Defaults();

	// Only merge if the new source has equal or higher priority

	if (source >= _sources.paths) {
		bool changed = false;
		auto take = [&](auto field) {
			if (other.paths.*field != defaults.paths.*field) {
				paths.*field = other.paths.*field;
				changed = true;
			}
		};

		take(&Paths::baseDir);
		take(&Paths::extensionsDir);
		take(&Paths::configsDir);
		take(&Paths::dataDir);
		take(&Paths::logsDir);
		take(&Paths::cacheDir);

		if (changed) {
			_sources.paths = source;
		}
	}

	if (source >= _sources.loading) {
		bool changed = false;
		auto take = [&](auto field) {
			if (other.loading.*field != defaults.loading.*field) {
				loading.*field = other.loading.*field;
				changed = true;
			}
		};

		take(&Loading::preferOwnSymbols);
		take(&Loading::maxConcurrentLoads);
		take(&Loading::loadTimeout);
		take(&Loading::exportTimeout);
		take(&Loading::startTimeout);

		if (changed) {
			_sources.loading = source;
		}
	}

	if (source >= _sources.security) {
		bool changed = false;
		// An override replaces a set outright; anything below it adds to one.
		bool replace = source == ConfigSource::Override;
		auto take = [&](auto field) {
			const auto& from = other.security.*field;
			if (from == defaults.security.*field) {
				return;
			}
			if (replace) {
				security.*field = from;
			} else {
				(security.*field).insert(from.begin(), from.end());
			}
			changed = true;
		};

		take(&Security::whitelistedExtensions);
		take(&Security::blacklistedExtensions);
		take(&Security::excludedDirs);

		if (changed) {
			_sources.security = source;
		}
	}

	if (source >= _sources.logging) {
		bool changed = false;
		auto take = [&](auto field) {
			if (other.logging.*field != defaults.logging.*field) {
				logging.*field = other.logging.*field;
				changed = true;
			}
		};

		take(&Logging::severity);
		take(&Logging::printReport);
		take(&Logging::printLoadOrder);
		take(&Logging::printDependencyGraph);
		take(&Logging::printDigraphDot);
		take(&Logging::exportDigraphDot);

		if (changed) {
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