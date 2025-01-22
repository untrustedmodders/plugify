#include "std_logger.hpp"

#include <iostream>

namespace plug {
    void StdLogger::Log(std::string_view message, plugify::Severity severity) {
        if (severity <= _severity) {
            switch (severity) {
                case plugify::Severity::None:
                    std::cout << message << std::endl;
                    break;
                case plugify::Severity::Fatal:
                    std::cerr << "[@] Fatal: " << message << std::endl;
                    break;
                case plugify::Severity::Error:
                    std::cerr << "[#] Error: " << message << std::endl;
                    break;
                case plugify::Severity::Warning:
                    std::cout << "[!] Warning: " << message << std::endl;
                    break;
                case plugify::Severity::Info:
                    std::cout << "[+] Info: " << message << std::endl;
                    break;
                case plugify::Severity::Debug:
                    std::cout << "[~] Debug: " << message << std::endl;
                    break;
                case plugify::Severity::Verbose:
                    std::cout << "[*] Verbose: " << message << std::endl;
                    break;
                default:
                    std::cout << "Unsupported error message logged " << message << std::endl;
                    break;
            }
        }
    }

    void StdLogger::SetSeverity(plugify::Severity severity) {
		std::lock_guard<std::mutex> lock(_mutex);
        _severity = severity;
    }
}
