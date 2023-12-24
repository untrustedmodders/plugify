#include "std_logger.h"

namespace sorcerer {
    void StdLogger::Log(const std::string& message, wizard::ErrorLevel level) {
        if (level >= m_level) {
            switch (level) {
            case wizard::ErrorLevel::INFO:
                std::cout << "[+] Info: " << message << std::endl;
                break;
            case wizard::ErrorLevel::WARN:
                std::cout << "[!] Warn: " << message << std::endl;
                break;
            case wizard::ErrorLevel::ERROR:
                std::cout << "[!] Error: " << message << std::endl;
                break;
            default:
                std::cout << "Unsupported error message logged " << message << std::endl;
            }
        }

        Push({ message, level });
    }

    void StdLogger::Push(Error error) {
        m_log.emplace_back(std::move(error));
    }

    Error StdLogger::Pop() {
        if (!m_log.empty()) {
            Error error = std::move(m_log.back());
            m_log.pop_back();
            return error;
        }
        return {};
    }

    void StdLogger::SetLogLevel(wizard::ErrorLevel level) {
        m_level = level;
    }
}
