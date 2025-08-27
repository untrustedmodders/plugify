#pragma once

#include "core/console_logger.hpp"

namespace plugify {
   class FileLogger : public ConsoleLogger {
    public:
        FileLogger(const std::filesystem::path& logFile,
                   Severity minSeverity = Severity::Info,
                   size_t maxFileSize = 10 * 1024 * 1024) // 10MB default
            : ConsoleLogger(minSeverity)
            , _logPath(logFile)
            , _maxFileSize(maxFileSize) {

            // Create log directory if needed
            auto parent = _logPath.parent_path();
            if (!parent.empty() && !std::filesystem::exists(parent)) {
                std::filesystem::create_directories(parent);
            }

            _logFile.open(_logPath, std::ios::app);
            if (!_logFile) {
                throw std::runtime_error("Failed to open log file: " + _logPath.string());
            }
        }

        ~FileLogger() {
            if (_logFile.is_open()) {
                _logFile.close();
            }
        }

        void Log(std::string_view message, Severity severity,
                 std::source_location loc = std::source_location::current()) override {
            if (severity < _minSeverity) return;

            auto output = FormatMessage(message, severity, loc);

            std::lock_guard lock(_mutex);

            // Check for rotation
            if (ShouldRotate()) {
                RotateLog();
            }

            _logFile << output << std::endl;

            // Also log to console for warnings and errors
            if (severity >= Severity::Warning) {
                if (severity >= Severity::Error) {
                    std::cerr << output << std::endl;
                } else {
                    std::cout << output << std::endl;
                }
            }
        }

        void Flush() override {
            std::lock_guard lock(_mutex);
            _logFile.flush();
            ConsoleLogger::Flush();
        }

    private:
        std::filesystem::path _logPath;
        std::ofstream _logFile;
        size_t _maxFileSize;

        bool ShouldRotate() {
            auto pos = _logFile.tellp();
            return pos >= static_cast<std::streamoff>(_maxFileSize);
        }

        void RotateLog() {
            _logFile.close();

            // Generate timestamp for rotated file
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tm{};
    #if PLUGIFY_PLATFORM_WINDOWS
            localtime_s(&tm, &time_t);
    #else
            localtime_r(&time_t, &tm);
    #endif

            auto rotatedPath = _logPath.parent_path() /
                std::format("{}.{:04d}{:02d}{:02d}_{:02d}{:02d}{:02d}{}",
                           _logPath.stem().string(),
                           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                           tm.tm_hour, tm.tm_min, tm.tm_sec,
                           _logPath.extension().string());

            std::filesystem::rename(_logPath, rotatedPath);

            _logFile.open(_logPath, std::ios::app);
        }
    };
}
