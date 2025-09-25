#pragma once

#include "core/console_logger.hpp"

namespace plugify {
	class FileLogger final : public ConsoleLogger {
	public:
		FileLogger(
			const std::filesystem::path& logFile,
			Severity minSeverity = Severity::Info,
			size_t maxFileSize = 10 * 1024 * 1024
		)  // 10MB default
			: ConsoleLogger(minSeverity)
			, _logPath(logFile)
			, _maxFileSize(maxFileSize) {
			// Create log directory if needed
			auto parent = _logPath.parent_path();
			if (!parent.empty() && !std::filesystem::exists(parent)) {
				std::filesystem::create_directories(parent);
			}

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
			std::source_location loc = std::source_location::current()
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
		std::filesystem::path _logPath;
		std::ofstream _logFile;
		size_t _maxFileSize;

		bool ShouldRotate() {
			auto pos = _logFile.tellp();
			return pos >= static_cast<std::streamoff>(_maxFileSize);
		}

		void RotateLog() {
			using namespace std::chrono;

			_logFile.close();

			auto now = system_clock::now();
			// floor to seconds so filename has no fractional seconds
			auto seconds = floor<seconds>(now);

			// zoned_time with current_zone() -> local time
			std::chrono::zoned_time zt{ std::chrono::current_zone(), seconds };

			// Format as: stem.YYYYMMDD_HHMMSS.extension
			auto rotatedPath = _logPath.parent_path()
							   / std::format(
								   "{}.{:%Y%m%d_%H%M%S}{}",
								   plg::as_string(_logPath.stem()),
								   zt,	// formatted using chrono spec
								   plg::as_string(_logPath.extension())
							   );

			std::filesystem::rename(_logPath, rotatedPath);
			_logFile.open(_logPath, std::ios::app);
		}
	};
}
