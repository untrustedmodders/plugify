#pragma once

#include <plugify/descriptor.h>
#include <optional>

namespace plugify {
	/**
	 * @struct LanguageModuleDescriptor
	 * @brief Describes the properties of a language module.
	 *
	 * The LanguageModuleDescriptor structure extends the Descriptor structure to include
	 * additional information specific to language modules, such as the programming language,
	 * library directories, and whether to force load the module.
	 */
	struct LanguageModuleDescriptor final : public Descriptor {
		std::string language; ///< The programming language of the module.
		std::optional<std::vector<std::string>> libraryDirectories; ///< Optional library directories for the module.
		bool forceLoad{false}; ///< Indicates whether to force load the module.
	};

	/**
	 * @struct LanguageModuleInfo
	 * @brief Holds basic information about a language module.
	 *
	 * The LanguageModuleInfo structure provides basic information about a language module,
	 * including its name.
	 */
	struct LanguageModuleInfo final {
		std::string name; ///< The name of the language module.
	};
} // namespace plugify