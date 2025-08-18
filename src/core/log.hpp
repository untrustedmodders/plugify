#pragma once

#include <plugify/api/log.hpp>
#include <plg/format.hpp>

namespace plugify {
	class LogSystem {
	public:
		static void SetLogger(std::shared_ptr<ILogger> logger);
		static void Log(std::string_view msg, Severity severity);
		static void Log(std::string_view msg, Color color);

	private:
		static inline std::shared_ptr<ILogger> _logger = nullptr;
	};

	struct ScopeLog {
		std::string entry;
		size_t count{};
		int level{2};
		Severity severity{Severity::Error};

		operator bool() const { return count != 0; }

		template <typename... Args>
		void Log(std::string_view format, Args&&... args) {
			if (0 == count++) {
				LogSystem::Log(entry, severity);
			}
			auto fmt = std::format("{{}} └─ {}", format);
			auto sps = std::format("{:{}}", "", level);
			auto msg = std::format(fmt, std::make_format_args(sps, std::forward<Args>(args)...));
			LogSystem::Log(msg, severity);
		}

	};
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
