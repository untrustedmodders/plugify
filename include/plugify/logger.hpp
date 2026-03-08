#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <string>

#include "plugify/types.hpp"

namespace plugify {
	/**
	 * @enum Severity
	 * @brief Enumerates severity levels for logging messages.
	 */
	enum class Severity { Unknown, Verbose, Debug, Info, Warning, Error, Fatal };

	/**
	 * @class ILogger
	 * @brief Interface for logging messages with different severity levels.
	 */
	class ILogger {
	public:
		virtual ~ILogger() = default;

		/**
		 * @brief Log a message with the specified severity level.
		 * @param message The log message.
		 * @param severity The severity level of the log message.
		 * @param location The source location where the log message is generated. Defaults to the
		 * current location.
		 */
		virtual void
		Log(std::string_view message,
		    Severity severity,
		    const Location& location = Location::current()
		) = 0;

		/**
		 * @brief Set the minimum severity level for logging messages.
		 * @param minSeverity The minimum severity level to log.
		 */
		virtual void SetLogLevel(Severity minSeverity) = 0;

		/**
		 * @brief Get the minimum severity level for logging messages.
		 * @return The minimum severity level to log.
		 */
		virtual Severity GetLogLevel() = 0;

		/**
		 * @brief Flush any buffered log messages.
		 */
		virtual void Flush() = 0;
	};

}  // namespace plugify
