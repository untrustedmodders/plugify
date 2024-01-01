#pragma once

namespace wizard {
	struct Config {
		fs::path baseDir;
		Severity logSeverity{ Severity::Verbose };
		bool strictMode{ true };
	};
}