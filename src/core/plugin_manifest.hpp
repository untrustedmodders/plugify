#pragma once

#include "manifest.hpp"
#include "method.hpp"

namespace plugify {
	struct PluginManifest : Manifest {
		// Plugin specific
		std::string entry;
		Dependency language;
		std::optional<std::vector<std::unique_ptr<Method>>> methods;

		std::vector<std::string> Validate() {
			std::vector<std::string> errors = Manifest::Validate();

			if (entry.empty()) {
				errors.emplace_back("Missing entry point");
			}

			if (language.name.empty()) {
				errors.emplace_back("Missing language name");
			}

			if (methods && !methods->empty()) {
				if (RemoveDuplicates(*methods)) {
					PL_LOG_WARNING("Manifest: '{}' has multiple same methods!", name);
				}
				size_t i = 0;
				for (const auto& method : *methods) {
					auto methodErrors = method->Validate(++i);
					errors.insert(errors.end(), methodErrors.begin(), methodErrors.end());
				}
			}

			return errors;
		}
	};

}
