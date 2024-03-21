#pragma once

#include <plugify/log.h>
#include <filesystem>
#include <set>

namespace plugify {
	/**
	 * @struct Config
	 * @brief Represents configuration settings for a program.
	 *
	 * This struct encapsulates various configuration parameters.
	 */
	struct Config final {
		std::filesystem::path baseDir; ///< The base directory for the program.
		Severity logSeverity{ Severity::Verbose }; ///< The severity level for logging.
		std::set<std::string> repositories; ///< A collection of repository paths.
	};
} // namespace plugify