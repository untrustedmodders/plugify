#include "std_logger.h"

#include <iostream>

namespace sorcerer {
    void StdLogger::Log(const std::string& message, wizard::Severity severity) {
        if (severity <= _severity) {
            switch (severity) {
                case wizard::Severity::Fatal:
                    std::cerr << "[@] Fatal: " << message << std::endl;
                    break;
                case wizard::Severity::Error:
                    std::cerr << "[#] Error: " << message << std::endl;
                    break;
                case wizard::Severity::Warning:
                    std::cout << "[!] Warning: " << message << std::endl;
                    break;
                case wizard::Severity::Info:
                    std::cout << "[+] Info: " << message << std::endl;
                    break;
                case wizard::Severity::Debug:
                    std::cout << "[~] Debug: " << message << std::endl;
                    break;
                case wizard::Severity::Verbose:
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
        _log.emplace_back(std::move(message));
    }

    std::string StdLogger::Pop() {
        if (!_log.empty()) {
            std::string message = std::move(_log.back());
            _log.pop_back();
            return message;
        }
        return {};
    }

    void StdLogger::SetSeverity(wizard::Severity severity) {
        _severity = severity;
    }
}
