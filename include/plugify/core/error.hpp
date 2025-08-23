#pragma once

#include <string>
#include <optional>
#include <filesystem>

namespace plugify {
	using ErrorMessage = std::string;

	enum class ErrorCode {
		None,
	    ConfigurationMissing,
		FileNotFound,
		InvalidManifest,
		MissingDependency,
		VersionConflict,
		InitializationFailed,
		LanguageModuleNotLoaded,
		CircularDependency,
		ValidationFailed,
		DisabledByPolicy,
	    MaxRetriesExceeded
	};

    enum class ErrorCategory {
        Transient,      // Retryable: file locks, network issues, temporary failures
        Configuration,  // Non-retryable: bad manifest, invalid config
        Dependency,     // Non-retryable: missing dependencies
        Resource,       // Partially retryable: out of memory, disk space
        Runtime,        // Context-dependent: initialization failures
        Validation,     // Non-retryable: validation failures
    };

    struct Error {
        ErrorCode code{ErrorCode::None};
        std::string message;
        ErrorCategory category;
        bool retryable{false};
        std::chrono::milliseconds retryDelay{0};
        //std::optional<std::filesystem::path> source;
        //std::optional<std::vector<std::string>> warnings;

        // Factory methods make it easy to create correctly
        static Error Permanent(ErrorCode code, std::string msg, ErrorCategory category) {
            return {code, std::move(msg), category, false, std::chrono::milliseconds{0}};
        }

        static Error Transient(ErrorCode code, std::string message, ErrorCategory category,
                              std::chrono::milliseconds delay = std::chrono::milliseconds{100}) {
            return {code, std::move(message), category, true, delay};
        }

        // Determine retryability from code if needed
        static Error Auto(ErrorCode code, std::string message, ErrorCategory category) {
            // Automatically determine retryability based on error code
            static const std::unordered_set<ErrorCode> RETRYABLE_CODES = {
                ErrorCode::FileNotFound,        // File might appear
                ErrorCode::InitializationFailed, // Might work on retry
            };

            bool retryable = RETRYABLE_CODES.contains(code);
            auto delay = retryable ? std::chrono::milliseconds{100} : std::chrono::milliseconds{0};
            return {code, std::move(message), category, retryable, delay};
        }

        // Chain methods for optional fields
        /*Error& WithSource(std::filesystem::path path) {
            source = std::move(path);
            return *this;
        }

        Error& WithDelay(std::chrono::milliseconds delay) {
            retryDelay = delay;
            return *this;
        }*/
    };
}