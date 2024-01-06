#pragma once

#include <wizard/descriptor.h>

namespace wizard {
	struct LanguageModuleDescriptor : public Descriptor {
		std::string language;
		bool forceLoad{ false };
	};

	struct LanguageModuleInfo {
		std::string name;
	};
}