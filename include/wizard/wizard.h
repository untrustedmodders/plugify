#pragma once

#include <cstdint>
#include <memory>

namespace wizard {
	class ILogger;
	class IWizardProvider;
	class IPluginManager;
	enum class ErrorLevel : uint8_t;

	class IWizard {
	public:
		virtual ~IWizard() = default;

		virtual bool Initialize() = 0;
		virtual void Terminate() = 0;

		virtual void SetLogger(std::shared_ptr<ILogger> logger) = 0;
		virtual void Log(const std::string& msg, Severity severity) = 0;

		virtual std::weak_ptr<IWizardProvider> GetProvider() = 0;
		virtual std::weak_ptr<IPluginManager> GetPluginManager() = 0;
	};

	// Entry Point
	std::shared_ptr<IWizard> MakeWizard();
}
