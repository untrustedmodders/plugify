#pragma once

#include <plugify/log.h>
#include <filesystem>
#include <vector>

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
		std::vector<std::string> repositories; ///< A collection of repository paths.
	};
} // namespace plugify