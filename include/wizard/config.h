#pragma once

namespace wizard {
	struct Config {
		std::filesystem::path baseDir;
		Severity logSeverity{ Severity::Verbose };
		bool strictMode{ true };
		bool packageVerification{ true };
		std::string packageVerifyUrl;
	};
}