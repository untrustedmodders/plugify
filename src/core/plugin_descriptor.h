#pragma once

#include "method.h"
#include "language_module_descriptor.h"
#include "plugin_reference_descriptor.h"
#include <plugify/plugin_reference_descriptor.h>
#include <plugify/descriptor.h>
#include <plugify/method.h>
#include <plugify/value_type.h>

namespace plugify {
	struct PluginDescriptor final : public Descriptor {
		std::string entryPoint;
		LanguageModuleInfo languageModule;
		std::vector<PluginReferenceDescriptor> dependencies;
		std::vector<Method> exportedMethods;

	private:
		mutable std::shared_ptr<std::vector<PluginReferenceDescriptorRef>> _dependencies;
		mutable std::shared_ptr<std::vector<MethodRef>> _exportedMethods;

		friend class PluginDescriptorRef;
	};
}