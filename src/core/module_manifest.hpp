#pragma once

#include "manifest.hpp"

namespace plugify {
	struct ModuleManifest : Manifest {
		// Module specific
		std::string language;
		std::optional<std::vector<std::string>> directories;
		std::optional<bool> forceLoad;

		std::vector<std::string> Validate() {
			std::vector<std::string> errors = Manifest::Validate();

			if (language.empty()) {
				errors.emplace_back("Missing language name");
			}

			if (directories && !directories->empty()) {
				if (RemoveDuplicates(*directories)) {
					PL_LOG_WARNING("Manifest: '{}' has multiple same directories!", name);
				}
				size_t i = 0;
				for (const auto& directory : *directories) {
					if (directory.empty()) {
						errors.emplace_back(std::format("Missing directory at {}", ++i));
					}
				}
			}

			return errors;
		}
	};
}
