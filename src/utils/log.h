#pragma once

#include <wizard/log.h>
#include <wizard/compat_format.h>

namespace wizard {
    class LogSystem {
    public:
        static void SetLogger(std::shared_ptr<ILogger> logger);
        static void Log(const std::string& msg, Severity severity);

    private:
        static inline std::shared_ptr<ILogger> _logger = nullptr;
    };
}

#if WIZARD_LOGGING
#define WZ_LOG(sev, ...)    wizard::LogSystem::Log(std::format(__VA_ARGS__), sev)
#define WZ_LOG_VERBOSE(...) WZ_LOG(wizard::Severity::Verbose, __VA_ARGS__)
#define WZ_LOG_DEBUG(...)   WZ_LOG(wizard::Severity::Debug, __VA_ARGS__)
#define WZ_LOG_INFO(...)    WZ_LOG(wizard::Severity::Info, __VA_ARGS__)
#define WZ_LOG_WARNING(...) WZ_LOG(wizard::Severity::Warning, __VA_ARGS__)
#define WZ_LOG_ERROR(...)   WZ_LOG(wizard::Severity::Error, __VA_ARGS__)
#define WZ_LOG_FATAL(...)   WZ_LOG(wizard::Severity::Fatal, __VA_ARGS__)
#else
#define WZ_LOG(sev, ...)
#define WZ_LOG_VERBOSE(...)
#define WZ_LOG_DEBUG(...)
#define WZ_LOG_INFO(...)
#define WZ_LOG_WARNING(...)
#define WZ_LOG_ERROR(...)
#define WZ_LOG_FATAL(...)
#endif
