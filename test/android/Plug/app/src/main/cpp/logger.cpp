#include "logger.h"

namespace plug {
    void AndroidLogger::Log(std::string_view message, plugify::Severity severity) {
        if (severity <= _severity) {
            switch (severity) {
                case plugify::Severity::Fatal:
                    __android_log_print(ANDROID_LOG_FATAL, "Plugify", "%s", message.data());
                    break;
                case plugify::Severity::Error:
                   __android_log_print(ANDROID_LOG_ERROR, "Plugify", "%s", message.data());
                    break;
                case plugify::Severity::Warning:
                    __android_log_print(ANDROID_LOG_WARN, "Plugify", "%s", message.data());
                    break;
                case plugify::Severity::Info:
                    __android_log_print(ANDROID_LOG_INFO, "Plugify", "%s", message.data());
                    break;
                case plugify::Severity::Debug:
                    __android_log_print(ANDROID_LOG_DEBUG, "Plugify", "%s", message.data());
                    break;
                case plugify::Severity::Verbose:
                    __android_log_print(ANDROID_LOG_VERBOSE, "Plugify", "%s", message.data());
                    break;
                default:
                    __android_log_print(ANDROID_LOG_SILENT, "Plugify", "Unsupported error message logged");
                    break;
            }
        }
    }

    void AndroidLogger::SetSeverity(plugify::Severity severity) {
		std::lock_guard<std::mutex> lock(_mutex);
        _severity = severity;
    }
}
