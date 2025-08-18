#pragma once

#include "manifest.hpp"

namespace plugify {
	struct ModuleManifest : Manifest {
		// Module specific
		std::string language;
		std::optional<std::vector<std::string>> directories;
		std::optional<bool> forceLoad;

		bool Validate(StackLogger& logger) const {
			Manifest::Validate(logger);
			ScopeLogger{logger};

			if (language.empty()) {
				logger.Log("Missing language name");
			}

			if (directories && !directories->empty()) {
				size_t i = 0;
				ScopeLogger{logger};
				for (const auto& directory : *directories) {
					if (directory.empty()) {
						logger.Log(std::format("Missing directory at {}", ++i));
					}
				}
			}
		}
	};
}
