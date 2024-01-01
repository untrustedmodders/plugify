#pragma once

#include <string>
#include <wizard_export.h>

namespace wizard {
	class WizardProvider;
	class IPluginManager;
	enum class ErrorLevel : uint8_t;

	// Wizard provided to user, which implemented in core
	class WIZARD_API IWizardProvider {
	protected:
		IWizardProvider(WizardProvider& impl);
		~IWizardProvider();

	public:
		void Log(const std::string& msg, Severity severity);

		std::weak_ptr<IPluginManager> GetPluginManager();

	private:
		WizardProvider& _impl;
	};
}
