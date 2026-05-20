#pragma once

#include "core/console_logger.hpp"

namespace plugify {
	class FileLogger final : public ConsoleLogger {
	public:
		FileLogger(
			std::filesystem::path logFile,
			Severity minSeverity = Severity::Info,
			size_t maxFileSize = 10 * 1024 * 1024
		)  // 10MB default
			: ConsoleLogger(minSeverity)
			, _logPath(TimestampedPath(logFile))
			, _logBase(std::move(logFile))
			, _maxFileSize(maxFileSize) {
			std::error_code ec;
			std::filesystem::create_directories(base.parent_path(), ec);

			_logFile.open(_logPath, std::ios::app);
			if (!_logFile) {
				throw std::runtime_error("Failed to open log file: " + plg::as_string(_logPath));
			}
		}

		~FileLogger() {
			if (_logFile.is_open()) {
				_logFile.close();
			}
		}

		void Log(
			std::string_view message,
			Severity severity,
			const Location& loc = Location::current()
		) override {
			if (severity < _minSeverity) {
				return;
			}

			auto output = FormatMessage(message, severity, loc);

			std::lock_guard lock(_mutex);

			// Check for rotation
			if (ShouldRotate()) {
				RotateLog();
			}

			_logFile << output << std::endl;

			// Also log to console for warnings and errors
			if (severity >= Severity::Warning) {
				if (severity >= Severity::Error) {
					std::cerr << output << std::endl;
				} else {
					std::cout << output << std::endl;
				}
			}
		}

		void Flush() override {
			std::lock_guard lock(_mutex);
			_logFile.flush();
			ConsoleLogger::Flush();
		}

	private:
		std::ofstream _logFile;
		std::filesystem::path _logPath;
		std::filesystem::path _logBase;
		size_t _maxFileSize;

		bool ShouldRotate() {
			auto pos = _logFile.tellp();
			return pos >= static_cast<std::streamoff>(_maxFileSize);
		}

		void RotateLog() {
			_logFile.close();
			_logPath = TimestampedPath(_logBase);
			_logFile.open(_logPath, std::ios::trunc);
		}

		// Helper: turn /logs/session.log → /logs/session-20260418_143022.log
		static std::filesystem::path TimestampedPath(const std::filesystem::path& base) {
			using namespace std::chrono;
			auto now = system_clock::now();
			auto seconds = floor<std::chrono::seconds>(now);

			return base.parent_path()
			   / std::format(
				   "{}-{:%Y%m%d_%H%M%S}{}",
				   plg::as_string(base.stem()),
				   utc_clock::from_sys(seconds),
				   plg::as_string(base.extension())
			   );
		}
	};
}
