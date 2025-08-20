#pragma once

#include <string>
#include <optional>
#include <filesystem>

namespace plugify {
	using ErrorMessage = std::string;

	enum class ErrorCode {
		None,
		FileNotFound,
		InvalidManifest,
		MissingDependency,
		VersionConflict,
		InitializationFailed,
		LanguageModuleNotLoaded,
		CircularDependency,
		ValidationFailed,
		DisabledByPolicy
	};

	struct Error {
		ErrorCode code{ErrorCode::None};
		ErrorMessage message;
		std::optional<std::filesystem::path> source;
	};

	enum class ErrorCategory {
		Transient,      // Retryable: file locks, network issues, temporary failures
		Configuration,  // Non-retryable: bad manifest, invalid config
		Dependency,     // Non-retryable: missing dependencies
		Resource,       // Partially retryable: out of memory, disk space
		Runtime,        // Context-dependent: initialization failures
		Validation      // Non-retryable: validation failures
	};

	struct EnhancedError : Error {
		ErrorCategory category;
		bool isRetryable;
		std::optional<std::chrono::milliseconds> suggestedRetryDelay;

		static EnhancedError Transient(ErrorCode code, const std::string& msg,
									   std::chrono::milliseconds delay = std::chrono::milliseconds{100}) {
			return EnhancedError{
	            {code, msg, std::nullopt},
				ErrorCategory::Transient,
				true,
				delay
			};
		}

		static EnhancedError NonRetryable(ErrorCode code, const std::string& msg, ErrorCategory cat) {
			return EnhancedError{
	            {code, msg, std::nullopt},
				cat,
				false,
				std::nullopt
			};
		}
	};
}