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
	};

	/**
	 * @brief Namespace containing utility functions of Severity enum.
	 */
	struct SeverityUtils {
		/**
		 * @brief Convert a Severity enum value to its string representation.
		 * @param severity The Severity value to convert.
		 * @return The string representation of the Severity.
		 */
		static constexpr std::string_view ToString(Severity severity) noexcept {
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
		static constexpr Severity FromString(std::string_view severity) noexcept {
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
	};

	// ============================================================================
	// Standard Logger Implementation
	// ============================================================================

	/**
	 * @brief Standard implementation using std::iostream
	 */
	class Logger final : public ILogger {
	public:
		void Log(std::string_view message, Severity severity) override {
			if (severity <= _severity) {
				switch (severity) {
					case Severity::None:
						std::cout << message << std::endl;
						break;
					case Severity::Fatal:
						std::cerr << "[@] Fatal: " << message << std::endl;
						break;
					case Severity::Error:
						std::cerr << "[#] Error: " << message << std::endl;
						break;
					case Severity::Warning:
						std::cout << "[!] Warning: " << message << std::endl;
						break;
					case Severity::Info:
						std::cout << "[+] Info: " << message << std::endl;
						break;
					case Severity::Debug:
						std::cout << "[~] Debug: " << message << std::endl;
						break;
					case Severity::Verbose:
						std::cout << "[*] Verbose: " << message << std::endl;
						break;
					default:
						std::cerr << "Unsupported error message logged " << message << std::endl;
						break;
				}
			}
		}

		void SetSeverity(Severity severity) {
			_severity = severity;
		}

	private:
		std::atomic<Severity> _severity{ Severity::None };
	};

} // namespace plugify
