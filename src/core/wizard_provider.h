#pragma once

#include "wizard_context.h"
#include <wizard/wizard_provider.h>
#include <wizard/wizard.h>

namespace wizard {
	class WizardProvider final : public IWizardProvider, public WizardContext {
	public:
		using WizardContext::WizardContext;

		void Log(const std::string& msg, ErrorLevel level) override;
	};
}
