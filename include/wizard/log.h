#pragma once

#include <string>
#include <cstdint>

namespace wizard {
    enum class Severity : uint8_t {
        None = 0,
        Fatal = 1,
        Error = 2,
        Warning = 3,
        Info = 4,
        Debug = 5,
        Verbose = 6
    };

    class ILogger {
    public:
        virtual ~ILogger() = default;

        virtual void Log(const std::string& msg, Severity severity) = 0;
    };

    [[maybe_unused]] constexpr std::string_view SeverityToString(Severity severity) {
        switch (severity) {
            case Severity::Fatal:   return "Fatal";
            case Severity::Error:   return "Error";
            case Severity::Warning: return "Warning";
            case Severity::Info:    return "Info";
            case Severity::Debug:   return "Debug";
            case Severity::Verbose: return "Verbose";
            default:                return "None";
        }
    }

    [[maybe_unused]] constexpr Severity SeverityFromString(std::string_view severity) {
        if (severity == "Fatal") {
            return Severity::Fatal;
        } else if (severity == "Error") {
            return Severity::Error;
        } else if (severity == "Warning") {
            return Severity::Warning;
        } else if (severity == "Info") {
            return Severity::Info;
        } else if (severity == "Debug") {
            return Severity::Debug;
        } else if (severity == "Verbose") {
            return Severity::Verbose;
        }
        return Severity::None;
    }
}
