#pragma once

#include <plugify/descriptor.h>
#include <optional>

namespace plugify {
	struct LanguageModuleDescriptor : public Descriptor {
		std::string language;
		std::optional<std::vector<std::string>> libraryDirectories;
		bool forceLoad{ false };
	};

	struct LanguageModuleInfo {
		std::string name;
	};
}