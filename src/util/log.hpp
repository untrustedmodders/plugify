#pragma once

#include <plugify/api/log.hpp>
#include <plg/format.hpp>

namespace plugify {
	class LogSystem {
	public:
		static void SetLogger(std::shared_ptr<ILogger> logger);
		static void Log(std::string_view msg, Severity severity);

	private:
		static inline std::shared_ptr<ILogger> _logger = nullptr;
	};
}

#if PLUGIFY_LOGGING
#define PL_LOG(sev, ...)    plugify::LogSystem::Log(std::format(__VA_ARGS__), sev)
#define PL_LOG_VERBOSE(...) PL_LOG(plugify::Severity::Verbose, __VA_ARGS__)
#define PL_LOG_DEBUG(...)   PL_LOG(plugify::Severity::Debug, __VA_ARGS__)
#define PL_LOG_INFO(...)    PL_LOG(plugify::Severity::Info, __VA_ARGS__)
#define PL_LOG_WARNING(...) PL_LOG(plugify::Severity::Warning, __VA_ARGS__)
#define PL_LOG_ERROR(...)   PL_LOG(plugify::Severity::Error, __VA_ARGS__)
#define PL_LOG_FATAL(...)   PL_LOG(plugify::Severity::Fatal, __VA_ARGS__)
#else
#define PL_LOG(sev, ...)
#define PL_LOG_VERBOSE(...)
#define PL_LOG_DEBUG(...)
#define PL_LOG_INFO(...)
#define PL_LOG_WARNING(...)
#define PL_LOG_ERROR(...)
#define PL_LOG_FATAL(...)
#endif // PLUGIFY_LOGGING
