#pragma once

namespace wizard {
	class IWizard;

	class WizardContext {
	public:
		WizardContext(std::weak_ptr<IWizard> wizard);
		~WizardContext();

	protected:
		std::weak_ptr<IWizard> _wizard;
	};
}
