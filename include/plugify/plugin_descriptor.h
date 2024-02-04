#pragma once

#include <plugify/descriptor.h>
#include <plugify/language_module_descriptor.h>
#include <plugify/plugin_reference_descriptor.h>
#include <plugify/method.h>
#include <filesystem>

namespace plugify {
	struct PluginDescriptor : public Descriptor {
		std::string entryPoint;
		LanguageModuleInfo languageModule;
		std::vector<PluginReferenceDescriptor> dependencies;
		std::vector<Method> exportedMethods;
	};
}