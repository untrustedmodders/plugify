#pragma once

#include <plugify/descriptor.h>
#include <plugify/language_module_descriptor.h>
#include <plugify/plugin_reference_descriptor.h>
#include <plugify/method.h>
#include <filesystem>

namespace plugify {
	/**
	 * @struct PluginDescriptor
	 * @brief Describes the properties of a plugin.
	 *
	 * The PluginDescriptor structure extends the Descriptor structure to include additional
	 * information specific to plugins, such as the entry point, language module info,
	 * dependencies, and exported methods.
	 */
	struct PluginDescriptor : public Descriptor {
		std::string entryPoint; ///< The entry point of the plugin.
		LanguageModuleInfo languageModule; ///< Information about the language module.
		std::vector<PluginReferenceDescriptor> dependencies; ///< The dependencies of the plugin.
		std::vector<Method> exportedMethods; ///< The methods exported by the plugin.
	};
} // namespace plugify