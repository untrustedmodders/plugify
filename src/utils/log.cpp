#include "log.h"

using namespace wizard;

void LogSystem::SetLogger(std::shared_ptr<ILogger> logger) {
    m_logger = std::move(logger);
}

void LogSystem::Log(const std::string& msg, ErrorLevel level) {
    if (m_logger)
        m_logger->Log(msg, level);
}