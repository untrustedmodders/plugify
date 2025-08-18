#pragma once

#include "manifest.hpp"

namespace plugify {
	struct ModuleManifest : Manifest {
		// Module specific
		std::string language;
		std::optional<std::vector<std::string>> directories;
		std::optional<bool> forceLoad;

		bool Validate() const {
			for (auto&& error : Manifest::Validate()) {
				co_yield std::move(error);
			}

			if (language.empty()) {
				co_yield "Missing language name";
			}

			if (directories && !directories->empty()) {
				size_t i = 0;
				for (const auto& directory : *directories) {
					if (directory.empty()) {
						co_yield std::format("Missing directory at {}", ++i);
					}
				}
			}
		}
	};
}
