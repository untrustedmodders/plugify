#pragma once

#include <wizard/descriptor.h>
#include <wizard/language_module_descriptor.h>
#include <wizard/plugin_reference_descriptor.h>
#include <wizard/method.h>
#include <filesystem>

namespace wizard {
	struct PluginDescriptor : public Descriptor {
		std::filesystem::path assemblyPath;
		LanguageModuleInfo languageModule;
		std::vector<PluginReferenceDescriptor> dependencies;
		std::vector<Method> exportedMethods;
	};
}