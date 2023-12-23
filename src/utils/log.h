#pragma once

namespace wizard {
    enum class ErrorLevel : uint8_t {
        INFO,
        WARN,
        ERROR,
    };

    class WIZARD_API Logger {
    public:
        Logger() = default;
        virtual ~Logger() = default;

        virtual void Log(const std::string& msg, ErrorLevel level) = 0;
    };

    class WIZARD_API LogSystem {
    public:
        static void SetLogger(std::shared_ptr<Logger> logger);
        static void Log(const std::string& msg, ErrorLevel level);

    private:
        static inline std::shared_ptr<Logger> m_logger = nullptr;
    };

    struct Error {
        std::string msg;
        ErrorLevel lvl;
    };

    class StdLogger final : public Logger {
    public:
        StdLogger() = default;
        ~StdLogger() override = default;

        void Log(const std::string& msg, ErrorLevel level) override;

        void Push(const Error& err);
        Error Pop();

        static StdLogger& Get();

        void SetLogLevel(ErrorLevel level);

    private:
        std::vector<Error> m_log;
        ErrorLevel m_level{ ErrorLevel::INFO };
    };
}

#if WIZARD_LOGGING
#define WIZARD_LOG(msg, lvl) wizard::LogSystem::Log(msg, lvl)
#else
#define WIZARD_LOG(msg, lvl)
#endif