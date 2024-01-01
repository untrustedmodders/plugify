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
        Verbose = 6,
    };

    class ILogger {
    public:
        virtual ~ILogger() = default;

        virtual void Log(const std::string& msg, Severity severity) = 0;
    };

    [[maybe_unused]] constexpr std::string_view SeverityToString(Severity severity) {
        using enum Severity;
        switch (severity) {
            case Fatal:   return "Fatal";
            case Error:   return "Error";
            case Warning: return "Warning";
            case Info:    return "Info";
            case Debug:   return "Debug";
            case Verbose: return "Verbose";
            default:                return "None";
        }
    }

    [[maybe_unused]] constexpr Severity SeverityFromString(std::string_view severity) {
        using enum Severity;
        if (severity == "Fatal") {
            return Fatal;
        } else if (severity == "Error") {
            return Error;
        } else if (severity == "Warning") {
            return Warning;
        } else if (severity == "Info") {
            return Info;
        } else if (severity == "Debug") {
            return Debug;
        } else if (severity == "Verbose") {
            return Verbose;
        }
        return None;
    }
}
