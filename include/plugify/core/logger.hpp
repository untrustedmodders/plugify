#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <atomic>

namespace plugify {
	/**
	 * @enum Severity
	 * @brief Enumerates severity levels for logging messages.
	 */
	enum class Severity {
		None,
		Fatal,
		Error,
		Warning,
		Info,
		Debug,
		Verbose,
		Unknown
	};

	/**
	 * @brief Enumeration of available text colors and styles for console output.
	 *
	 * This enum defines both normal and bold variations of commonly used colors.
	 * It is typically used in conjunction with ANSI escape codes to provide
	 * colored or emphasized text output in supported terminals.
	 *
	 * @note Not all terminals support ANSI escape sequences. If unsupported,
	 *       the raw escape codes may be displayed instead of colored text.
	 */
	enum class Color {
		None,       ///< No color / reset to default.
		Red,        ///< Standard red text.
		Green,      ///< Standard green text.
		Yellow,     ///< Standard yellow text.
		Blue,       ///< Standard blue text.
		Cyan,       ///< Standard cyan text.
		Gray,       ///< Standard gray text.
		Bold,       ///< Bold text (no color).
		BoldRed,    ///< Bold red text.
		BoldGreen,  ///< Bold green text.
		BoldYellow, ///< Bold yellow text.
		BoldBlue,   ///< Bold blue text.
		BoldCyan    ///< Bold cyan text.
	};

	/**
	 * @class ILogger
	 * @brief Interface for logging messages with different severity levels.
	 */
	class ILogger {
	public:
		virtual ~ILogger() = default;

		/**
		 * @brief Log a message with the specified severity level.
		 * @param msg The log message.
		 * @param severity The severity level of the log message.
		 */
		virtual void Log(std::string_view msg, Severity severity) = 0;

		/**
		 * @brief Log a message with the specified color.
		 * @param msg The log message.
		 * @param color The color of the log message.
		 */
		virtual void Log(std::string_view msg, Color color) = 0;
	};

} // namespace plugify
