#pragma once

#include "plugify/logger.hpp"

#include "plg/enum.hpp"

namespace plugify {
	class ConsoleLogger final : public ILogger {
	public:
		ConsoleLogger(Severity minSeverity = Severity::Info)
			: _minSeverity(minSeverity) {
		}

		~ConsoleLogger() override = default;

		void Log(
			std::string_view message,
			Severity severity,
			Location loc = Location::current()
		) override {
			if (severity == Severity::Unknown) {
				std::cout << message << std::endl;
				return;
			} else if (severity < _minSeverity) {
				return;
			}

			auto output = FormatMessage(message, severity, loc);

			std::lock_guard lock(_mutex);
			if (severity >= Severity::Error) {
				std::cerr << output << std::endl;
			} else {
				std::cout << output << std::endl;
			}
		}

		void SetLogLevel(Severity minSeverity) override {
			_minSeverity = minSeverity;
		}

		Severity GetLogLevel() override {
			return _minSeverity;
		}

		void Flush() override {
			std::cout.flush();
			std::cerr.flush();
		}

	protected:
		static std::string
		FormatMessage(std::string_view message, Severity severity, Location location) {
			using namespace std::chrono;

			auto now = system_clock::now();

			// Split into seconds + milliseconds
			auto seconds = floor<std::chrono::seconds>(now);
			auto ms = duration_cast<milliseconds>(now - seconds);

			return std::format(
				"[{:%F %T}.{:03d}] [{}] [{} => {}:({}:{}): {}] {}",
				seconds,  // %F = YYYY-MM-DD, %T = HH:MM:SS
				static_cast<int>(ms.count()),
				plg::enum_to_string(severity),
				location.module_name(),
				location.file_name(),
				location.line(),
				location.column(),
				location.function_name(),
				message
			);
		}

	protected:
		std::mutex _mutex;
		std::atomic<Severity> _minSeverity;
	};
}
