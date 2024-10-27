#pragma once

#include <mutex>
#include <plugify/log.hpp>

namespace plug {
    class StdLogger final : public plugify::ILogger {
    public:
        StdLogger() = default;
        ~StdLogger() override = default;

        void Log(std::string_view message, plugify::Severity severity) override;

        void SetSeverity(plugify::Severity severity);

    private:
		std::mutex _mutex;
        plugify::Severity _severity{ plugify::Severity::None };
    };
}
