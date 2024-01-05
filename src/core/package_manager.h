#pragma once

#include "wizard_context.h"
#include "package.h"
#include <wizard/package_manager.h>

namespace wizard {
	class PackageManager : public IPackageManager, public WizardContext {
	public:
		explicit PackageManager(std::weak_ptr<IWizard> wizard);
		~PackageManager();

		void LoadPackages();
		void SnapshotPackages(const fs::path& filepath, bool prettify);

	private:
		using PackageMap = std::unordered_map<std::string, Package>;

		static std::optional<Package> ReadDescriptor(const fs::path& path, const std::string& name, bool module);

	private:
		PackageMap _allPackages;
	};
}