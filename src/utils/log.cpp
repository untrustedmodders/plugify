#include "log.h"

using namespace wizard;

void LogSystem::SetLogger(std::shared_ptr<ILogger> logger) {
    _logger = std::move(logger);
}

void LogSystem::Log(const std::string& msg, ErrorLevel level) {
    if (_logger)
        _logger->Log(msg, level);
}