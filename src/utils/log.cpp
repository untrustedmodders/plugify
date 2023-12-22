#include "log.h"

using namespace wizard;

void LogSystem::SetLogger(std::shared_ptr<Logger> logger) {
    m_logger = std::move(logger);
}

void LogSystem::Log(const std::string& msg, ErrorLevel level) {
    if (m_logger)
        m_logger->Log(msg, level);
}

void StdLogger::Log(const std::string& msg, ErrorLevel level) {
    Push({ msg, level });
}

void StdLogger::Push(const Error& err) {
    if (err.lvl >= m_level) {
        switch (err.lvl) {
            case ErrorLevel::INFO:
                std::cout << "[+] Info: " << err.msg << std::endl;
                break;
            case ErrorLevel::WARN:
                std::cout << "[!] Warn: " << err.msg << std::endl;
                break;
            case ErrorLevel::ERROR:
                std::cout << "[!] Error: " << err.msg << std::endl;
                break;
            default:
                std::cout << "Unsupported error message logged " << err.msg << std::endl;
        }
    }

    m_log.push_back(err);
}

Error StdLogger::Pop() {
    Error err{};
    if (!m_log.empty()) {
        err = m_log.back();
        m_log.pop_back();
    }
    return err;
}


void StdLogger::SetLogLevel(ErrorLevel level) {
    m_level = level;
}

StdLogger& StdLogger::Get() {
    static StdLogger logger;
    return logger;
}