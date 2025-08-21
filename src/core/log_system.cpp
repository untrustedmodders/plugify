#include "plugify/core/log_system.hpp"

using namespace plugify;

void StandardLogger::Log(std::string_view message, Severity severity) {
	if (severity <= _severity) {
		auto color = GetColor(severity);
		std::cout << GetAnsiCode(color) << message << GetAnsiCode(Color::None) << std::endl;
	}
}

void StandardLogger::Log(std::string_view message, Color color) {
	std::cout << GetAnsiCode(color) << message << GetAnsiCode(Color::None) << std::endl;
}

void StandardLogger::SetSeverity(Severity severity) {
	_severity = severity;
}

Color StandardLogger::GetColor(Severity severity) {
	switch (severity) {
		case Severity::Fatal:   return Color::BoldRed;
		case Severity::Error:   return Color::Red;
		case Severity::Warning: return Color::BoldYellow;
		case Severity::Info:    return Color::Green;
		case Severity::Debug:   return Color::Cyan;
		case Severity::Verbose: return Color::Gray;
		default:                return Color::None;
	}
}
std::string_view StandardLogger::GetAnsiCode(Color color) {
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
			return "\033[0m"; // Fallback to reset
	}
}
