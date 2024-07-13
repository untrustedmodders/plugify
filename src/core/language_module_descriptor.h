#pragma once

#include <plugify/descriptor.h>

namespace plugify {
	struct LanguageModuleDescriptor final : public Descriptor {
		std::string language;
		std::optional<std::vector<std::string>> libraryDirectories;
		bool forceLoad{false};
	};

	struct LanguageModuleInfo final {
		std::string name;
	};
}