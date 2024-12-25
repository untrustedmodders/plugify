#pragma once

#include <cstdint>
#include <string_view>

namespace plugify {
	/**
	 * @enum Severity
	 * @brief Enumerates severity levels for logging messages.
	 */
	enum class Severity : uint8_t {
		None = 0,
		Fatal = 1,
		Error = 2,
		Warning = 3,
		Info = 4,
		Debug = 5,
		Verbose = 6,
	};

	/**
	 * @class ILogger
	 * @brief Interface for logging messages with different severity levels.
	 *
	 * The ILogger interface provides a common interface for loggers to handle
	 * logging messages with various severity levels.
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
	};

	/**
	 * @brief Namespace containing utility functions of Severity enum.
	 */
	namespace SeverityUtils {
		/**
		 * @brief Convert a Severity enum value to its string representation.
		 * @param severity The Severity value to convert.
		 * @return The string representation of the Severity.
		 */
		constexpr std::string_view ToString(Severity severity) noexcept {
			switch (severity) {
				case Severity::Fatal:   return "Fatal";
				case Severity::Error:   return "Error";
				case Severity::Warning: return "Warning";
				case Severity::Info:    return "Info";
				case Severity::Debug:   return "Debug";
				case Severity::Verbose: return "Verbose";
				default:                return "None";
			}
		}

		/**
		 * @brief Convert a string representation to a Severity enum value.
		 * @param severity The string representation of Severity.
		 * @return The corresponding Severity enum value.
		 */
		constexpr Severity FromString(std::string_view severity) noexcept {
			if (severity == "Fatal") {
				return Severity::Fatal;
			} else if (severity == "Error") {
				return Severity::Error;
			} else if (severity == "Warning") {
				return Severity::Warning;
			} else if (severity == "Info") {
				return Severity::Info;
			} else if (severity == "Debug") {
				return Severity::Debug;
			} else if (severity == "Verbose") {
				return Severity::Verbose;
			}
			return Severity::None;
		}
	} // namespace SeverityUtils

} // namespace plugify
