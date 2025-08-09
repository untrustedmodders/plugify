#pragma once

#include "manifest.hpp"
#include <util/algorithm.hpp>

namespace plugify {
	struct ModuleManifest : Manifest {
		std::string language;
		std::optional<std::vector<std::string>> directories;
		std::optional<bool> forceLoad;

	private:
		// temp storage to return spans of handles easily
		mutable std::shared_ptr<std::vector<std::string_view>> _platforms;
		mutable std::shared_ptr<std::vector<std::string_view>> _directories;
		friend class ModuleManifestHandle;

	public:
		std::vector<std::string> Validate() {
			std::vector<std::string> errors;

			if (language.empty()) {
				errors.emplace_back("Missing language name");
			}

			if (directories) {
				if (RemoveDuplicates(*directories)) {
					PL_LOG_WARNING("Language Module: '{}' has multiple same directories!", name);
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
