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
}
