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

		std::weak_ptr<IPluginManager> GetPluginManager();
	};
}
