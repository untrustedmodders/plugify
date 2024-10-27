#pragma once

#include "language_module_descriptor.hpp"
#include "method.hpp"
#include "plugin_reference_descriptor.hpp"
#include <plugify/descriptor.hpp>
#include <plugify/method.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <plugify/value_type.hpp>

namespace plugify {
	struct PluginDescriptor final : public Descriptor {
		std::string entryPoint;
		LanguageModuleInfo languageModule;
		std::vector<PluginReferenceDescriptor> dependencies;
		std::vector<Method> exportedMethods;

	private:
		mutable std::shared_ptr<std::vector<std::string_view>> _supportedPlatforms;
		mutable std::shared_ptr<std::vector<std::string_view>> _resourceDirectories;
		mutable std::shared_ptr<std::vector<PluginReferenceDescriptorRef>> _dependencies;
		mutable std::shared_ptr<std::vector<MethodRef>> _exportedMethods;

		friend class PluginDescriptorRef;
	};
}
