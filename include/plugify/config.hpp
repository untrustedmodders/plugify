#pragma once

#include <any>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_set>

#include "plugify/logger.hpp"
#include "plugify/platform_ops.hpp"
#include "plugify/types.hpp"

namespace plugify {
	// Update mode for explicit control
	enum class UpdateMode {
		Manual,            // User calls Update() manually
		BackgroundThread,  // Automatic updates in background thread
		Callback           // User provides update callback
	};

	// Configuration source priority
	enum class ConfigSource {
		Default,  // Default values
		File,     // Loaded from file
		Builder,  // Set via builder methods
		Override  // Explicit overrides (highest priority)
	};

	struct Config {
		// Track source priority for each field group
		struct SourceTracking {
			ConfigSource paths = ConfigSource::Default;
			ConfigSource loading = ConfigSource::Default;
			ConfigSource runtime = ConfigSource::Default;
			ConfigSource security = ConfigSource::Default;
			ConfigSource logging = ConfigSource::Default;
		} _sources{};

		// Paths configuration
		struct Paths {
			std::filesystem::path baseDir;
			std::filesystem::path extensionsDir = "extensions";
			std::filesystem::path configsDir = "configs";
			std::filesystem::path dataDir = "data";
			std::filesystem::path logsDir = "logs";
			std::filesystem::path cacheDir = "cache";

			// Check if paths have non-default values
			bool HasCustomExtensionsDir() const {
				return extensionsDir != "extensions";
			}

			bool HasCustomConfigsDir() const {
				return configsDir != "configs";
			}

			bool HasCustomDataDir() const {
				return dataDir != "data";
			}

			bool HasCustomLogsDir() const {
				return logsDir != "logs";
			}

			bool HasCustomCacheDir() const {
				return cacheDir != "cache";
			}

			void ResolveRelativePaths() {
				auto makeAbsolute = [this](std::filesystem::path& path) {
					if (!path.empty() && path.is_relative() && !baseDir.empty()) {
						path = (baseDir / path).lexically_normal();
					}
				};

				makeAbsolute(extensionsDir);
				makeAbsolute(configsDir);
				makeAbsolute(dataDir);
				makeAbsolute(logsDir);
				makeAbsolute(cacheDir);
			}
		} paths{};

		// Loading configuration
		struct Loading {
			bool preferOwnSymbols = false;
			size_t maxConcurrentLoads = 4;
			std::chrono::milliseconds loadTimeout{ 500 };
			std::chrono::milliseconds exportTimeout{ 100 };
			std::chrono::milliseconds startTimeout{ 250 };

			// Check if values are non-default
			bool HasCustomPreferOwnSymbols() const {
				return preferOwnSymbols != false;
			}

			bool HasCustomMaxConcurrentLoads() const {
				return maxConcurrentLoads != 4;
			}

			bool HasCustomLoadTimeout() const {
				return loadTimeout != std::chrono::milliseconds{ 500 };
			}

			bool HasCustomExportTimeout() const {
				return exportTimeout != std::chrono::milliseconds{ 100 };
			}

			bool HasCustomStartTimeout() const {
				return startTimeout != std::chrono::milliseconds{ 250 };
			}
		} loading{};

		// Runtime configuration
		struct Runtime {
			bool pinToMainThread = true;
			UpdateMode updateMode = UpdateMode::Manual;
			std::chrono::milliseconds updateInterval{ 16 };
			std::function<void(std::chrono::milliseconds)> updateCallback;
			std::optional<size_t> threadPriority;

			// Check if values are non-default
			bool HasCustomPinToMainThread() const {
				return pinToMainThread != true;
			}

			bool HasCustomUpdateMode() const {
				return updateMode != UpdateMode::Manual;
			}

			bool HasCustomUpdateInterval() const {
				return updateInterval != std::chrono::milliseconds{ 16 };
			}

			bool HasThreadPriority() const {
				return threadPriority.has_value();
			}
		} runtime{};

		// Security configuration
		struct Security {
			std::unordered_set<std::string> whitelistedExtensions;
			std::unordered_set<std::string> blacklistedExtensions;
			std::unordered_set<std::filesystem::path> excludedDirs = DefaultExcludedDirs;

			bool HasWhitelist() const {
				return !whitelistedExtensions.empty();
			}

			bool HasBlacklist() const {
				return !blacklistedExtensions.empty();
			}

			bool HasExcluded() const {
				return excludedDirs != DefaultExcludedDirs;
			}
		} security{};

		// Logging configuration
		struct Logging {
			Severity severity{ Severity::Error };
			bool printReport = false;
			bool printLoadOrder = false;
			bool printDependencyGraph = false;
			bool printDigraphDot = false;
			std::filesystem::path exportDigraphDot;

			bool HasCustomSeverity() const {
				return severity != Severity::Error;
			}

			bool HasCustomPrintReport() const {
				return printReport != false;
			}

			bool HasCustomPrintLoadOrder() const {
				return printLoadOrder != false;
			}

			bool HasCustomPrintDependencyGraph() const {
				return printDependencyGraph != false;
			}

			bool HasCustomPrintDigraphDot() const {
				return printDigraphDot != false;
			}

			bool HasExportPath() const {
				return !exportDigraphDot.empty();
			}
		} logging{};

		// Comprehensive merge implementation
		void MergeFrom(const Config& other, ConfigSource source = ConfigSource::Override);

		// Alternative merge for selective field updates
		void MergeField(std::string_view fieldPath, const Config& other, ConfigSource source = ConfigSource::Override);

		// Get effective source for debugging
		std::string GetSourceInfo() const {
		    return std::format(
		        "Config sources - Paths: {}, Loading: {}, Runtime: {}, Security: {}, Logging: {}",
		        plg::enum_to_string(_sources.paths),
		        plg::enum_to_string(_sources.loading),
		        plg::enum_to_string(_sources.runtime),
		        plg::enum_to_string(_sources.security),
		        plg::enum_to_string(_sources.logging)
		    );
		}

		// Reset to defaults
		void Reset() {
			*this = Config{};
		}

		// Validation remains the same
		Result<void> Validate() const;

	private:
		static inline std::unordered_set<std::filesystem::path> DefaultExcludedDirs = {
			PLUGIFY_PATH_LITERAL("disabled"),
			PLUGIFY_PATH_LITERAL(".git"),
			PLUGIFY_PATH_LITERAL(".svn"),
			PLUGIFY_PATH_LITERAL("temp"),
			PLUGIFY_PATH_LITERAL("tmp"),
		};
	};
}  // namespace plugify
