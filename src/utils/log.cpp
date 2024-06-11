#include "log.h"

using namespace plugify;

void LogSystem::SetLogger(std::shared_ptr<ILogger> logger) {
	_logger = std::move(logger);
}

void LogSystem::Log(const std::string& msg, Severity severity) {
	if (_logger)
		_logger->Log(msg, severity);
}