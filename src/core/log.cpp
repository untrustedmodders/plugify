#include "plugify/core/log_system.hpp"

using namespace plugify;

void LogSystem::SetLogger(std::shared_ptr<ILogger> logger) {
	_logger = std::move(logger);
}

void LogSystem::Log(std::string_view msg, Severity severity) {
	if (_logger) _logger->Log(msg, severity);
}

void LogSystem::Log(std::string_view msg, Color color) {
	if (_logger) _logger->Log(msg, color);
}
