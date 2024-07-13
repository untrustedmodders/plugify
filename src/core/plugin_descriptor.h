#pragma once

#include "method.h"
#include "language_module_descriptor.h"
#include "plugin_reference_descriptor.h"
#include <plugify/descriptor.h>
#include <plugify/value_type.h>

namespace plugify {
	struct PluginDescriptor final : public Descriptor {
		std::string entryPoint;
		LanguageModuleInfo languageModule;
		std::vector<PluginReferenceDescriptor> dependencies;
		std::vector<Method> exportedMethods;
	};
}