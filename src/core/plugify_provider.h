#pragma once

#include "plugify_context.h"
#include <plugify/plugify_provider.h>
#include <plugify/plugify.h>

namespace plugify {
	class PlugifyProvider final : public IPlugifyProvider, public PlugifyContext {
	public:
		explicit PlugifyProvider(std::weak_ptr<IPlugify> plugify);
		~PlugifyProvider();

		void Log(const std::string& msg, Severity severity);

		const fs::path& GetBaseDir();

		bool IsPreferOwnSymbols();
		
		bool IsPluginLoaded(const std::string& name, std::optional<int32_t> requiredVersion = {}, bool minimum = false);
		
		bool IsModuleLoaded(const std::string& name, std::optional<int32_t> requiredVersion = {}, bool minimum = false);
	};
}
