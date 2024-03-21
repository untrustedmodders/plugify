#include "std_logger.h"

#include <iostream>

namespace plug {
    void StdLogger::Log(const std::string& message, plugify::Severity severity) {
        if (severity <= _severity) {
            switch (severity) {
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

        Push(message);
    }

    void StdLogger::Push(std::string message) {
		std::lock_guard<std::mutex> lock{_mutex};
        _log.emplace_back(std::move(message));
    }

    std::string StdLogger::Pop() {
		std::lock_guard<std::mutex> lock{_mutex};
        if (!_log.empty()) {
            std::string message = std::move(_log.back());
            _log.pop_back();
            return message;
        }
        return {};
    }

    void StdLogger::SetSeverity(plugify::Severity severity) {
		std::lock_guard<std::mutex> lock{_mutex};
        _severity = severity;
    }
}
