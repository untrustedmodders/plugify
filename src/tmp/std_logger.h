#pragma once

#include <wizard/log.h>

namespace sorcerer {
    struct Error {
        std::string msg;
        wizard::ErrorLevel lvl;
    };

    class StdLogger final : public wizard::ILogger {
    public:
        StdLogger() = default;
        ~StdLogger() override = default;

        void Log(const std::string& message, wizard::ErrorLevel level) override;

        void Push(Error error);
        Error Pop();

        void SetLogLevel(wizard::ErrorLevel level);

    private:
        std::vector<Error> _log;
        wizard::ErrorLevel _level{ wizard::ErrorLevel::INFO };
    };
}
