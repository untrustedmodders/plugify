#pragma once

#include <optional>
#include <string>
#include <filesystem>
#include <unordered_set>

#include "plugify/logger.hpp"
#include "plugify/platform_ops.hpp"
#include "plugify/types.hpp"

namespace plugify {
	// Configuration source priority
	enum class ConfigSource {
		Default,  // Default values
		File,     // Loaded from file
		Builder,  // Set via builder methods
		Override  // Explicit overrides (highest priority)
	};

	constexpr Severity DefaultVerbosity
#ifndef NDEBUG
	= Severity::Debug
#else
	= Severity::Error
#endif
	;

	struct Config {
		// Track source priority for each field group
		struct SourceTracking {
			ConfigSource paths = ConfigSource::Default;
			ConfigSource loading = ConfigSource::Default;
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
				auto make_absolute = [&](std::filesystem::path& path) {
					if (!path.empty() && path.is_relative() && !baseDir.empty()) {
						path = (baseDir / path).lexically_normal();
					}
				};

				make_absolute(extensionsDir);
				make_absolute(configsDir);
				make_absolute(dataDir);
				make_absolute(logsDir);
				make_absolute(cacheDir);
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

		// Security configuration
		struct Security {
			std::unordered_set<std::string> whitelistedExtensions;
			std::unordered_set<std::string> blacklistedExtensions;
			std::unordered_set<std::filesystem::path> excludedDirs = GetDefaultExcludedDirs();

			bool HasWhitelist() const {
				return !whitelistedExtensions.empty();
			}

			bool HasBlacklist() const {
				return !blacklistedExtensions.empty();
			}

			bool HasExcluded() const {
				return excludedDirs != GetDefaultExcludedDirs();
			}
		} security{};

		// Logging configuration
		struct Logging {
			Severity severity{ DefaultVerbosity };
			bool printReport = false;
			bool printLoadOrder = false;
			bool printDependencyGraph = false;
			bool printDigraphDot = false;
			std::filesystem::path exportDigraphDot;

			bool HasCustomSeverity() const {
				return severity != DefaultVerbosity;
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
		        "Config sources - Paths: {}, Loading: {}, Security: {}, Logging: {}",
		        plg::enum_to_string(_sources.paths),
		        plg::enum_to_string(_sources.loading),
		        plg::enum_to_string(_sources.security),
		        plg::enum_to_string(_sources.logging)
		    );
		}

		// Reset to defaults
		void Reset() {
			*this = Config{};
		}

		// Validation of fields
		Result<void> Validate() const;

	private:
		static const std::unordered_set<std::filesystem::path>& GetDefaultExcludedDirs();
	};
}  // namespace plugify
