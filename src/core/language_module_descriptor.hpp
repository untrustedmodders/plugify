#pragma once

#include <plugify/descriptor.hpp>

namespace plugify {
	struct LanguageModuleDescriptor : public Descriptor {
		std::string language;
		std::optional<std::vector<std::string>> libraryDirectories;
		std::optional<bool> forceLoad;

	private:
		mutable std::shared_ptr<std::vector<std::string_view>> _supportedPlatforms;
		mutable std::shared_ptr<std::vector<std::string_view>> _resourceDirectories;
		mutable std::shared_ptr<std::vector<std::string_view>> _libraryDirectories;
		friend class LanguageModuleDescriptorHandle;
	};

	struct LanguageModuleInfo {
		std::string name;
	};
}
