#include "wizard_provider.h"

using namespace wizard;

WizardProvider::WizardProvider(std::weak_ptr<IWizard> wizard) : IWizardProvider(*this), WizardContext(std::move(wizard)) {
}

WizardProvider::~WizardProvider() = default;

void WizardProvider::Log(const std::string& msg, Severity severity) {
	if (auto locker = _wizard.lock()) {
		locker->Log(msg, severity);
	}
}

std::weak_ptr<IPluginManager> WizardProvider::GetPluginManager() {
    if (auto locker = _wizard.lock()) {
        return locker->GetPluginManager();
    }
    return {};
}
