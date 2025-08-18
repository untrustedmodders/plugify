#pragma once

#include "manifest.hpp"
#include "method.hpp"

namespace plugify {
	struct PluginManifest : Manifest {
		// Plugin specific
		std::string entry;
		Dependency language;
		std::optional<std::vector<std::unique_ptr<Method>>> methods;

		std::generator<std::string> Validate() {
			for (auto&& error : Manifest::Validate()) {
				co_yield std::move(error);
			}

			if (entry.empty()) {
				co_yield "Missing entry point";
			}

			if (language.name.empty()) {
				co_yield "Missing language name";
			}

			if (methods && !methods->empty()) {
				size_t i = 0;
				for (const auto& method : *methods) {
					for (auto&& error : method->Validate(++i)) {
						co_yield std::move(error);
					}
				}
			}
		}
	};

}
