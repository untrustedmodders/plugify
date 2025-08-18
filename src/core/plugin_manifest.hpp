#pragma once

#include "manifest.hpp"
#include "method.hpp"

namespace plugify {
	struct PluginManifest : Manifest {
		// Plugin specific
		std::string entry;
		Dependency language;
		std::optional<std::vector<std::unique_ptr<Method>>> methods;

		void Validate(StackLogger& logger) {
			Manifest::Validate(logger);
			ScopeLogger{logger};

			if (entry.empty()) {
				logger.Log("Missing entry point");
			}

			if (language.name.empty()) {
				logger.Log("Missing language name");
			}

			if (methods && !methods->empty()) {
				size_t i = 0;
				ScopeLogger{logger};
				for (const auto& method : *methods) {
					method->Validate(logger, std::to_string(++i));
				}
			}
		}
	};
}
