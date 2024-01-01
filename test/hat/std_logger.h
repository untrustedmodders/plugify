#pragma once

#include <wizard/log.h>

namespace sorcerer {
    class StdLogger final : public wizard::ILogger {
    public:
        StdLogger() = default;
        ~StdLogger() override = default;

        void Log(const std::string& message, wizard::Severity severity) override;

        void Push(std::string message);
        std::string Pop();

        void SetSeverity(wizard::Severity severity);

    private:
        std::vector<std::string> _log;
        wizard::Severity _severity{ wizard::Severity::None };
    };
}
