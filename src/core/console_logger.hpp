#pragma once

#include "plg/enum.hpp"
#include "plugify/core/logger.hpp"

namespace plugify {
    class ConsoleLogger : public ILogger {
    public:
        ConsoleLogger(Severity minSeverity = Severity::Info)
            : _minSeverity(minSeverity) {}

        void Log(std::string_view message, Severity severity,
                 std::source_location loc = std::source_location::current()) override {
            if (severity < _minSeverity) return;

            auto output = FormatMessage(message, severity, loc);

            std::lock_guard lock(_mutex);
            if (severity >= Severity::Error) {
                std::cerr << output << std::endl;
            } else {
                std::cout << output << std::endl;
            }
        }

        void SetLogLevel(Severity minSeverity) override {
            _minSeverity = minSeverity;
        }

        void Flush() override {
            std::cout.flush();
            std::cerr.flush();
        }

    protected:
        std::string FormatMessage(std::string_view message, Severity severity,
                                  const std::source_location& loc) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            std::tm tm{};
    #ifdef _WIN32
            localtime_s(&tm, &time_t);
    #else
            localtime_r(&time_t, &tm);
    #endif

            return std::format("[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}] "
                              "[{}] [{}:{}] {}",
                              tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                              tm.tm_hour, tm.tm_min, tm.tm_sec, ms.count(),
                              plg::enum_to_string(severity),
                              loc.file_name(),
                              loc.line(),
                              message);
        }

    protected:
        std::mutex _mutex;
        std::atomic<Severity> _minSeverity;
    };
}
