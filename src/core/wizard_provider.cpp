#include "wizard_provider.h"

using namespace wizard;

WizardProvider::WizardProvider(std::weak_ptr<IWizard> wizard) : IWizardProvider(*this), WizardContext(std::move(wizard)) {
}

WizardProvider::~WizardProvider() {

}

void WizardProvider::Log(const std::string& msg, ErrorLevel level) {
	if (auto locker = _wizard.lock()) {
		locker->Log(msg, level);
	}
}

std::weak_ptr<IPluginManager> WizardProvider::GetPluginManager() {
    if (auto locker = _wizard.lock()) {
        return locker->GetPluginManager();
    }
    return {};
}
