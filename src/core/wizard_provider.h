#pragma once

#include "wizard_context.h"
#include <wizard/wizard_provider.h>
#include <wizard/wizard.h>

namespace wizard {
	class WizardProvider final : public IWizardProvider, public WizardContext {
	public:
        explicit WizardProvider(std::weak_ptr<IWizard> wizard);
        ~WizardProvider();

		void Log(const std::string& msg, ErrorLevel level);

        std::weak_ptr<IPluginManager> GetPluginManager();
	};
}
