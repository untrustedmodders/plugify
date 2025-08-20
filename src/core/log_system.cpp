#include "plugify/core/log_system.hpp"

using namespace plugify;

void StandardLogger::Log(std::string_view message, Severity severity) {
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

void StandardLogger::Log(std::string_view msg, Color color) {
	std::cout << GetAnsiCode(color) << msg << GetAnsiCode(Color::None) << std::endl;
}

void StandardLogger::SetSeverity(Severity severity) {
	_severity = severity;
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
			return "";
	}
}
