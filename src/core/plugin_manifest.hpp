#pragma once

#include "manifest.hpp"
#include "method.hpp"
#include "module_manifest.hpp"
#include "dependency.hpp"
#include <plugify/api/dependency.hpp>
#include <plugify/api/method.hpp>
#include <plugify/api/value_type.hpp>
#include <util/algorithm.hpp>

namespace plugify {
	struct PluginManifest : Manifest {
		std::string entry;
		Dependency language;
		std::optional<std::vector<Dependency>> dependencies;
		std::optional<std::vector<Method>> methods;

	private:
		// temp storage to return spans of handles easily
		mutable std::shared_ptr<std::vector<std::string_view>> _platforms;
		mutable std::shared_ptr<std::vector<DependencyHandle>> _dependencies;
		mutable std::shared_ptr<std::vector<MethodHandle>> _methods;

		friend class PluginManifestHandle;

	public:
		std::vector<std::string> Validate() {
			std::vector<std::string> errors;

			if (entry.empty()) {
				errors.emplace_back("Missing entry point");
			}

			if (language.name.empty()) {
				errors.emplace_back("Missing language name");
			}

			if (dependencies) {
				if (RemoveDuplicates(*dependencies)) {
					PL_LOG_WARNING("Plugin: '{}' has multiple same dependencies!", name);
				}
				size_t i = 0;
				for (const auto& dependency : *dependencies) {
					if (dependency.name.empty()) {
						errors.emplace_back(std::format("Missing dependency name at {}", ++i));
					}
				}
			}

			if (methods) {
				if (RemoveDuplicates(*methods)) {
					PL_LOG_WARNING("Plugin: '{}' has multiple same methods!", name);
				}
				size_t i = 0;
				for (const auto& method : *methods) {
					auto methodErrors = method.Validate(++i);
					errors.insert(errors.end(), methodErrors.begin(), methodErrors.end());
				}
			}

			return errors;
		}
	};
}
