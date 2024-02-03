#pragma once

#include <plugify_export.h>
#include <string>
#include <memory>

namespace plugify {
	class PlugifyProvider;
	class IPluginManager;
	enum class Severity : uint8_t;

	// Plugify provided to user, which implemented in core
	class PLUGIFY_API IPlugifyProvider {
	protected:
		explicit IPlugifyProvider(PlugifyProvider& impl);
		~IPlugifyProvider();

	public:
		void Log(const std::string& msg, Severity severity);

		std::weak_ptr<IPluginManager> GetPluginManager();

	private:
		PlugifyProvider& _impl;
	};
}
