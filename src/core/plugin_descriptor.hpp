#pragma once

#include "language_module_descriptor.hpp"
#include "method.hpp"
#include "plugin_reference_descriptor.hpp"
#include <plugify/descriptor.hpp>
#include <plugify/package.hpp>
#include <plugify/method.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <plugify/value_type.hpp>
#include <utils/algorithm.hpp>

namespace plugify {
	struct PluginDescriptor : Descriptor {
		std::string entryPoint;
		LanguageModuleInfo languageModule;
		std::optional<std::vector<PluginReferenceDescriptor>> dependencies;
		std::optional<std::vector<Method>> exportedMethods;

	private:
		mutable std::shared_ptr<std::vector<std::string_view>> _supportedPlatforms;
		mutable std::shared_ptr<std::vector<PluginReferenceDescriptorHandle>> _dependencies;
		mutable std::shared_ptr<std::vector<MethodHandle>> _exportedMethods;

		friend class PluginDescriptorHandle;

	public:
		std::vector<std::string> Validate(const std::string& name) {
			std::vector<std::string> errors;

			if (fileVersion < 1) {
				errors.emplace_back("Invalid file version");
			}

			if (friendlyName.empty()) {
				errors.emplace_back("Missing friendly name");
			}

			if (entryPoint.empty()) {
				errors.emplace_back("Missing entry point");
			}

			if (languageModule.name.empty()) {
				errors.emplace_back("Missing language name");
			}

			if (auto& deps = dependencies) {
				if (RemoveDuplicates(*deps)) {
					PL_LOG_WARNING("Package: '{}' has multiple dependencies with same name!", name);
				}
				size_t i = 0;
				for (const auto& dependency : *deps) {
					if (dependency.name.empty()) {
						errors.emplace_back(std::format("Missing dependency name at {}", ++i));
					}
				}
			}

			if (auto& methods = exportedMethods) {
				if (RemoveDuplicates(*methods)) {
					PL_LOG_WARNING("Package: '{}' has multiple method with same name!", name);
				}
				size_t i = 0;
				for (const auto& method : *methods) {
					auto methodErrors = method.Validate(++i);
					errors.insert(errors.end(), methodErrors.begin(), methodErrors.end());
				}
			}

			return errors;
		}

		std::string GetType() const {
			return std::string(PackageType::Plugin);
		}
	};
}
