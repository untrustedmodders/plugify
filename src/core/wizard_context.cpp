#include "wizard_context.h"
#include <wizard/wizard.h>

using namespace wizard;

WizardContext::WizardContext(std::weak_ptr<IWizard> wizard) : _wizard{std::move(wizard)} {

}

WizardContext::~WizardContext() = default;
