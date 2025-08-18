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

	/**
	 * @brief Namespace containing utility functions of Color enum.
	 */
	struct ColorUtils {
		/**
		 * @brief Convert a Color enum value to its string representation.
		 * @param color The Color value to convert.
		 * @return The string representation of the Color.
		 */
		static constexpr std::string_view ToString(Color color) noexcept {
			switch (color) {
				case Color::Red:        return "Red";
				case Color::Green:      return "Green";
				case Color::Yellow:     return "Yellow";
				case Color::Blue:       return "Blue";
				case Color::Cyan:       return "Cyan";
				case Color::Gray:       return "Gray";
				case Color::Bold:       return "Bold";
				case Color::BoldRed:    return "BoldRed";
				case Color::BoldGreen:  return "BoldGreen";
				case Color::BoldYellow: return "BoldYellow";
				case Color::BoldBlue:   return "BoldBlue";
				case Color::BoldCyan:   return "BoldCyan";
				default:                return "None";
			}
		}

		/**
	 * @brief Convert a string representation to a Color enum value.
	 * @param color The string representation of Color.
	 * @return The corresponding Color enum value.
	 */
		static constexpr Color FromString(std::string_view color) noexcept {
			if (color == "Red") {
				return Color::Red;
			} else if (color == "Green") {
				return Color::Green;
			} else if (color == "Yellow") {
				return Color::Yellow;
			} else if (color == "Blue") {
				return Color::Blue;
			} else if (color == "Cyan") {
				return Color::Cyan;
			} else if (color == "Gray") {
				return Color::Gray;
			} else if (color == "Bold") {
				return Color::Bold;
			} else if (color == "BoldRed") {
				return Color::BoldRed;
			} else if (color == "BoldGreen") {
				return Color::BoldGreen;
			} else if (color == "BoldYellow") {
				return Color::BoldYellow;
			} else if (color == "BoldBlue") {
				return Color::BoldBlue;
			} else if (color == "BoldCyan") {
				return Color::BoldCyan;
			}
			return Color::None;
		}
	};

	// ============================================================================
	// Standard Logger Implementation
	// ============================================================================

	/**
	 * @brief Standard implementation using std::iostream
	 */
	class StandartLogger final : public ILogger {
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

		void Log(std::string_view msg, Color color) override {
			std::cout << GetAnsiCode(color) << msg << GetAnsiCode(Color::None) << std::endl;
		}

		void SetSeverity(Severity severity) {
			_severity = severity;
		}

	private:
		static std::string_view GetAnsiCode(Color color) {
			switch (color) {
				case Color::None:
					return "\033[0m";
				case Color::Red:
					return "\033[31m";
				case Color::Green:
					return "\033[32m";
				case Color::Yellow:
					return "\033[33m";
				case Color::Blue:
					return "\033[34m";
				case Color::Cyan:
					return "\033[36m";
				case Color::Gray:
					return "\033[90m";
				case Color::Bold:
					return "\033[1m";
				case Color::BoldRed:
					return "\033[1;31m";
				case Color::BoldGreen:
					return "\033[1;32m";
				case Color::BoldYellow:
					return "\033[1;33m";
				case Color::BoldBlue:
					return "\033[1;34m";
				case Color::BoldCyan:
					return "\033[1;36m";
				default:
					return "";
			}
		}

		std::atomic<Severity> _severity{ Severity::None };
	};

} // namespace plugify
