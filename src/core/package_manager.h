#pragma once

#include "wizard_context.h"
#include "package.h"
#include <wizard/package_manager.h>

namespace wizard {
	class PackageManager : public IPackageManager, public WizardContext {
	public:
		explicit PackageManager(std::weak_ptr<IWizard> wizard);
		~PackageManager();

		void UpdatePackages();
		void InstallPackages(const fs::path& manifestFilePath, bool reinstall = true);
		void SnapshotPackages(const fs::path& manifestFilePath, bool prettify);
	};
}