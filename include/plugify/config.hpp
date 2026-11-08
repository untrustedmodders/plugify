#pragma once

#include <optional>
#include <string>
#include <filesystem>
#include <unordered_set>

#include "plugify/logger.hpp"
#include "plugify/platform_ops.hpp"
#include "plugify/types.hpp"

namespace plugify {
	/**
	 * @enum ConfigSource
	 * @brief Where a group of configuration values came from.
	 *
	 * The values are ordered by priority: a merge only overwrites a group when it
	 * comes from a source at least as strong as the one that set it last, so a
	 * value from a file never silently replaces one the caller set explicitly.
	 *
	 * @see Config::MergeFrom, Config::MergeField
	 */
	enum class ConfigSource {
		Default,  ///< Default values, the weakest source.
		File,     ///< Loaded from a configuration file.
		Builder,  ///< Set through the methods of PlugifyBuilder.
		Override  ///< Explicit overrides, the strongest source.
	};

	/**
	 * @brief The severity a build logs at unless something overrides it.
	 *
	 * @ref Severity::Debug in a debug build, @ref Severity::Error otherwise.
	 */
	constexpr Severity DefaultVerbosity
#ifndef NDEBUG
	= Severity::Debug
#else
	= Severity::Error
#endif
	;

	/**
	 * @struct Config
	 * @brief Everything that steers a Plugify instance.
	 *
	 * The fields are grouped by concern, and each group records the
	 * @ref ConfigSource that set it so that overlapping sources merge by
	 * priority rather than by order.
	 *
	 * @see PlugifyBuilder, Plugify::GetConfig
	 */
	struct Config {
		/**
		 * @struct SourceTracking
		 * @brief The source that last set each group of fields.
		 */
		struct SourceTracking {
			ConfigSource paths = ConfigSource::Default;  ///< Source of @ref Config::paths.
			ConfigSource loading = ConfigSource::Default;  ///< Source of @ref Config::loading.
			ConfigSource security = ConfigSource::Default;  ///< Source of @ref Config::security.
			ConfigSource logging = ConfigSource::Default;  ///< Source of @ref Config::logging.
		} _sources{};  ///< Priority bookkeeping for the groups below.

		/**
		 * @struct Paths
		 * @brief The directories a Plugify instance works in.
		 *
		 * Every directory but @ref baseDir may be relative, in which case it is
		 * taken relative to the base.
		 */
		struct Paths {
			std::filesystem::path baseDir;  ///< Base folder everything else is relative to.
			std::filesystem::path extensionsDir = "extensions";  ///< Folder extensions are discovered in.
			std::filesystem::path configsDir = "configs";  ///< Folder extensions keep their configuration in.
			std::filesystem::path dataDir = "data";  ///< Folder extensions keep their data in.
			std::filesystem::path logsDir = "logs";  ///< Folder logs are written to.
			std::filesystem::path cacheDir = "cache";  ///< Folder cached files are written to.

			/**
			 * @brief Make every path in this group absolute.
			 *
			 * Anchors @ref baseDir against the current working directory, then
			 * resolves the other paths against it.
			 *
			 * @note Safe to call repeatedly: the base is anchored first, so a
			 * relative base cannot be prepended to the paths under it twice.
			 */
			void ResolveRelativePaths() {
				if (baseDir.empty()) {
					return;
				}

				// Anchor the base itself first. This runs after every merge and
				// again from Build(), and while the base stays relative so do the
				// paths under it, so each call would prepend it once more and turn
				// 'res/configs' into 'res/res/configs'.
				std::error_code ec;
				if (auto absolute = std::filesystem::absolute(baseDir, ec); !ec) {
					baseDir = absolute.lexically_normal();
				}

				auto make_absolute = [&](std::filesystem::path& path) {
					if (!path.empty() && path.is_relative()) {
						path = (baseDir / path).lexically_normal();
					}
				};

				make_absolute(extensionsDir);
				make_absolute(configsDir);
				make_absolute(dataDir);
				make_absolute(logsDir);
				make_absolute(cacheDir);
			}
		} paths{};  ///< Directory layout.

		/**
		 * @struct Loading
		 * @brief How extensions are brought up.
		 */
		struct Loading {
			bool preferOwnSymbols = false;  ///< Whether a module prefers its own symbols over shared ones.
			size_t maxConcurrentLoads = 4;  ///< How many extensions may be loaded at the same time.
			std::chrono::milliseconds loadTimeout{ 500 };  ///< Budget for loading one extension.
			std::chrono::milliseconds exportTimeout{ 100 };  ///< Budget for resolving the exports of one extension.
			std::chrono::milliseconds startTimeout{ 250 };  ///< Budget for starting one extension.

		} loading{};  ///< Loading behaviour.

		/**
		 * @struct Security
		 * @brief Which extensions are allowed to load.
		 *
		 * An empty whitelist allows everything that is not blacklisted; a
		 * non-empty one allows nothing else.
		 */
		struct Security {
			std::unordered_set<std::string> whitelistedExtensions;  ///< Extensions explicitly allowed to load.
			std::unordered_set<std::string> blacklistedExtensions;  ///< Extensions explicitly blocked from loading.
			std::unordered_set<std::filesystem::path> excludedDirs = GetDefaultExcludedDirs();  ///< Directories left out of discovery.

			/**
			 * @brief Check whether a whitelist is in effect.
			 * @return True if at least one extension is whitelisted.
			 */
			bool HasWhitelist() const {
				return !whitelistedExtensions.empty();
			}

			/**
			 * @brief Check whether a blacklist is in effect.
			 * @return True if at least one extension is blacklisted.
			 */
			bool HasBlacklist() const {
				return !blacklistedExtensions.empty();
			}

			/**
			 * @brief Check whether the excluded directories were customised.
			 * @return True if @ref excludedDirs differs from the built-in default.
			 */
			bool HasExcluded() const {
				return excludedDirs != GetDefaultExcludedDirs();
			}
		} security{};  ///< Discovery and loading restrictions.

		/**
		 * @struct Logging
		 * @brief What gets logged and which reports get printed.
		 */
		struct Logging {
			Severity severity{ DefaultVerbosity };  ///< Lowest severity that gets logged.
			bool printReport = false;  ///< Print the load report once the pipeline finishes.
			bool printLoadOrder = false;  ///< Print the order extensions were loaded in.
			bool printDependencyGraph = false;  ///< Print the dependency graph.
			bool printDigraphDot = false;  ///< Print the dependency graph in DOT format.
			std::filesystem::path exportDigraphDot;  ///< Path to write the DOT graph to, empty to write none.

			/**
			 * @brief Check whether the DOT graph is written to a file.
			 * @return True if @ref exportDigraphDot names a path.
			 */
			bool HasExportPath() const {
				return !exportDigraphDot.empty();
			}
		} logging{};  ///< Logging and reporting behaviour.

		/**
		 * @brief Merge another configuration into this one.
		 * @param other The configuration to take values from.
		 * @param source The source to attribute those values to.
		 * @note A group is only replaced when @p source is at least as strong as the source that
		 * set it last.
		 */
		void MergeFrom(const Config& other, ConfigSource source = ConfigSource::Override);

		/**
		 * @brief Merge one group of another configuration into this one.
		 * @param fieldPath Name of the group: "paths", "loading", "security" or "logging".
		 * @param other The configuration to take that group from.
		 * @param source The source to attribute the group to.
		 * @note An unknown @p fieldPath is ignored.
		 */
		void MergeField(std::string_view fieldPath, const Config& other, ConfigSource source = ConfigSource::Override);

		/**
		 * @brief Describe where the current values came from.
		 * @return One line naming the @ref ConfigSource of each group, for debugging.
		 */
		std::string GetSourceInfo() const {
		    return std::format(
		        "Config sources - Paths: {}, Loading: {}, Security: {}, Logging: {}",
		        plg::enum_to_string(_sources.paths),
		        plg::enum_to_string(_sources.loading),
		        plg::enum_to_string(_sources.security),
		        plg::enum_to_string(_sources.logging)
		    );
		}

		/**
		 * @brief Return every field to its default value.
		 */
		void Reset() {
			*this = Config{};
		}

		/**
		 * @brief Check that this configuration can be used.
		 * @return Nothing if it is valid, an error message describing the first problem otherwise.
		 */
		Result<void> Validate() const;

	private:
		/**
		 * @brief Get the directories left out of discovery by default.
		 * @return The built-in set of excluded directories.
		 */
		static const std::unordered_set<std::filesystem::path>& GetDefaultExcludedDirs();
	};
}  // namespace plugify
