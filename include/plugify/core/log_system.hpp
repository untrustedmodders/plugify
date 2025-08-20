#pragma once

#include <atomic>

#include "plg/format.hpp"
#include "plugify/core/logger.hpp"
#include "plugify/core/log_system.hpp"

namespace plugify {
	class LogSystem {
	public:
		static void SetLogger(std::shared_ptr<ILogger> logger);
		static void Log(std::string_view msg, Severity severity);
		static void Log(std::string_view msg, Color color);

	private:
		static inline std::shared_ptr<ILogger> _logger = nullptr;
	};

	/**
	 * @brief Standard implementation using std::iostream
	 */
	class StandardLogger final : public ILogger {
	public:
		void Log(std::string_view message, Severity severity) override;
		void Log(std::string_view msg, Color color) override;

		void SetSeverity(Severity severity);

	private:
		static std::string_view GetAnsiCode(Color color);
		std::atomic<Severity> _severity{ Severity::None };
	};

	/*struct StackLogger {
		std::string entry;
		Severity severity{Severity::Error};

		size_t total{};
		size_t count{};
		size_t level{};

		operator bool() const { return total != 0; }

		void Log(std::string_view message) {
			if (++total == 1) {
				LogSystem::Log(entry, severity);
			}

			auto msg = std::format("{:{}} {}. └─ {}", "", level, ++count, message);
			LogSystem::Log(msg, severity);
		}
	};

	struct ScopeLogger {
		StackLogger& logger;
		size_t step;

		ScopeLogger(StackLogger& logger, size_t step = 2)
			: logger{logger}, step{step}
		{
			logger.count = 0;
			logger.level += step;
		}

		~ScopeLogger() {
			logger.level -= step;
			logger.count = 0;
		}
	};*/
}

#if PLUGIFY_LOGGING
#define PL_LOG(sev, ...)    plugify::LogSystem::Log(std::format(__VA_ARGS__), sev)
#define PL_LOG_VERBOSE(...) PL_LOG(plugify::Severity::Verbose, __VA_ARGS__)
#define PL_LOG_DEBUG(...)   PL_LOG(plugify::Severity::Debug, __VA_ARGS__)
#define PL_LOG_INFO(...)    PL_LOG(plugify::Severity::Info, __VA_ARGS__)
#define PL_LOG_WARNING(...) PL_LOG(plugify::Severity::Warning, __VA_ARGS__)
#define PL_LOG_ERROR(...)   PL_LOG(plugify::Severity::Error, __VA_ARGS__)
#define PL_LOG_FATAL(...)   PL_LOG(plugify::Severity::Fatal, __VA_ARGS__)
#define LOG(fmt, col,  ...) plugify::LogSystem::Log(std::format(fmt, __VA_ARGS__), col)
#else
#define PL_LOG(sev, ...)
#define PL_LOG_VERBOSE(...)
#define PL_LOG_DEBUG(...)
#define PL_LOG_INFO(...)
#define PL_LOG_WARNING(...)
#define PL_LOG_ERROR(...)
#define PL_LOG_FATAL(...)
#define LOG(fmt, col,  ...)
#endif // PLUGIFY_LOGGING
