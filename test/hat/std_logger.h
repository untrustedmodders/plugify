#pragma once

#include <plugify/log.h>
#include <mutex>
#include <vector>

namespace sorcerer {
    class StdLogger final : public plugify::ILogger {
    public:
        StdLogger() = default;
        ~StdLogger() override = default;

        void Log(const std::string& message, plugify::Severity severity) override;

        void Push(std::string message);
        std::string Pop();

        void SetSeverity(plugify::Severity severity);

    private:
		std::mutex _mutex;
		std::vector<std::string> _log;
        plugify::Severity _severity{ plugify::Severity::None };
    };
}
