#pragma once

#include "language_module_descriptor.hpp"
#include "method.hpp"
#include "plugin_reference_descriptor.hpp"
#include <plugify/descriptor.hpp>
#include <plugify/method.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <plugify/value_type.hpp>

namespace plugify {
	struct PluginDescriptor : Descriptor {
		std::string entryPoint;
		LanguageModuleInfo languageModule;
		std::optional<std::vector<PluginReferenceDescriptor>> dependencies;
		std::optional<std::vector<Method>> exportedMethods;

	private:
		mutable std::shared_ptr<std::vector<std::string_view>> _supportedPlatforms;
		mutable std::shared_ptr<std::vector<std::string_view>> _resourceDirectories;
		mutable std::shared_ptr<std::vector<PluginReferenceDescriptorHandle>> _dependencies;
		mutable std::shared_ptr<std::vector<MethodHandle>> _exportedMethods;

		friend class PluginDescriptorHandle;
	};
}
