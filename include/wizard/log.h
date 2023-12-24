#pragma once

namespace wizard {
    enum class ErrorLevel : uint8_t {
        INFO,
        WARN,
        ERROR,
    };

    class ILogger {
    public:
        virtual ~ILogger() = default;

        virtual void Log(const std::string& msg, ErrorLevel level) = 0;
    };
}
