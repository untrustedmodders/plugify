#include "log.h"

using namespace plugify;

void LogSystem::SetLogger(std::shared_ptr<ILogger> logger) {
	_logger = std::move(logger);
}

void LogSystem::Log(std::string_view msg, Severity severity) {
	if (_logger)
		_logger->Log(msg, severity);
}
