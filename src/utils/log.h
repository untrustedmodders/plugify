#pragma once

#include <wizard/log.h>

namespace wizard {
    class LogSystem {
    public:
        static void SetLogger(std::shared_ptr<ILogger> logger);
        static void Log(const std::string& msg, ErrorLevel level);

    private:
        static inline std::shared_ptr<ILogger> m_logger = nullptr;
    };
}

#if WIZARD_LOGGING
#define WIZARD_LOG(msg, lvl) wizard::LogSystem::Log(msg, lvl)
#else
#define WIZARD_LOG(msg, lvl)
#endif
