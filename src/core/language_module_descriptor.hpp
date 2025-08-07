#pragma once

#include <plugify/descriptor.hpp>
#include <plugify/package.hpp>
#include <utils/algorithm.hpp>

namespace plugify {
	struct LanguageModuleDescriptor : Descriptor {
		std::string language;
		std::optional<std::vector<std::string>> libraryDirectories;
		std::optional<bool> forceLoad;

	private:
		mutable std::shared_ptr<std::vector<std::string_view>> _supportedPlatforms;
		mutable std::shared_ptr<std::vector<std::string_view>> _libraryDirectories;
		friend class LanguageModuleDescriptorHandle;

	public:
		std::vector<std::string> Validate(const std::string& name) {
			std::vector<std::string> errors;

			if (fileVersion < 1) {
				errors.emplace_back("Invalid file version");
			}

			if (friendlyName.empty()) {
				errors.emplace_back("Missing friendly name");
			}

			if (language.empty()) {
				errors.emplace_back("Missing language name");
			}

			if (language == PackageType::Plugin) {
				errors.emplace_back("Invalid language name");
			}

			if (auto& directories = libraryDirectories) {
				if (RemoveDuplicates(*directories)) {
					PL_LOG_WARNING("Package: '{}' has multiple library directories with same name!", name);
				}
				size_t i = 0;
				for (const auto& directory : *directories) {
					if (directory.empty()) {
						errors.emplace_back(std::format("Missing library directory at {}", ++i));
					}
				}
			}

			return errors;
		}

		std::string GetType() const {
			return language;
		}
	};

	struct LanguageModuleInfo {
		std::string name;
		std::optional<plg::version> version;
	};
}
