#include <wizard/wizard_provider.h>
#include <core/wizard_provider.h>

using namespace wizard;

IWizardProvider::IWizardProvider(WizardProvider& impl) : _impl{impl} {
}

IWizardProvider::~IWizardProvider() = default;

void IWizardProvider::Log(const std::string& msg, ErrorLevel level) {
    _impl.Log(msg, level);
}

std::weak_ptr<IPluginManager> IWizardProvider::GetPluginManager() {
    return _impl.GetPluginManager();
}
