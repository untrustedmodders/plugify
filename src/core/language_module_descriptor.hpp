#pragma once

#include <plugify/descriptor.hpp>

namespace plugify {
	struct LanguageModuleDescriptor final : public Descriptor {
		std::string language;
		std::optional<std::vector<std::string>> libraryDirectories;
		bool forceLoad{false};

	private:
		mutable std::shared_ptr<std::vector<std::string_view>> _supportedPlatforms;
		mutable std::shared_ptr<std::vector<std::string_view>> _resourceDirectories;
		mutable std::shared_ptr<std::vector<std::string_view>> _libraryDirectories;
		friend class LanguageModuleDescriptorRef;
	};

	struct LanguageModuleInfo final {
		std::string name;
	};
}
