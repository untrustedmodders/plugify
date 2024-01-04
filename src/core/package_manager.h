#pragma once

#include "wizard_context.h"
#include "package.h"

namespace wizard {
	class PackageManager : public WizardContext {
	public:
		explicit PackageManager(std::weak_ptr<IWizard> wizard);
		~PackageManager();

	private:
		using PackageMap = std::unordered_map<std::string, Package>;

		void LoadPackages();

	private:
		PackageMap _allPackages;
	};
}