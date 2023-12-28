#include "wizard_provider.h"

using namespace wizard;

void WizardProvider::Log(const std::string& msg, ErrorLevel level) {
	if (auto locker = _wizard.lock()) {
		locker->Log(msg, level);
	}
}
