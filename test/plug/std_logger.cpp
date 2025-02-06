#include "std_logger.hpp"

#include <iostream>
#include <syncstream>

namespace plug {
    void StdLogger::Log(std::string_view message, plugify::Severity severity) {
		if (severity <= _severity) {
			switch (severity) {
				case plugify::Severity::None:
					std::osyncstream{std::cout} << message << std::endl;
					break;
				case plugify::Severity::Fatal:
					std::osyncstream{std::cerr} << "[@] Fatal: " << message << std::endl;
					break;
				case plugify::Severity::Error:
					std::osyncstream{std::cerr} << "[#] Error: " << message << std::endl;
					break;
				case plugify::Severity::Warning:
					std::osyncstream{std::cout} << "[!] Warning: " << message << std::endl;
					break;
				case plugify::Severity::Info:
					std::osyncstream{std::cout} << "[+] Info: " << message << std::endl;
					break;
				case plugify::Severity::Debug:
					std::osyncstream{std::cout} << "[~] Debug: " << message << std::endl;
					break;
				case plugify::Severity::Verbose:
					std::osyncstream{std::cout} << "[*] Verbose: " << message << std::endl;
					break;
				default:
					std::osyncstream{std::cerr} << "Unsupported error message logged " << message << std::endl;
					break;
			}
		}
    }

    void StdLogger::SetSeverity(plugify::Severity severity) {
        _severity = severity;
    }
}
