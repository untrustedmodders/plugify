#pragma once

#include <android/log.h>
#include <plugify/log.h>
#include <mutex>

namespace plug {
    class AndroidLogger final : public plugify::ILogger {
    public:
        AndroidLogger() = default;
        ~AndroidLogger() override = default;

        void Log(std::string_view message, plugify::Severity severity) override;

        void SetSeverity(plugify::Severity severity);

    private:
		std::mutex _mutex;
        plugify::Severity _severity{ plugify::Severity::None };
    };
}
