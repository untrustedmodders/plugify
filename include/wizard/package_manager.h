#pragma once

#include <filesystem>
#include <wizard_export.h>

namespace wizard {
	class PackageManager;

	// Package manager provided to user, which implemented in core
	class WIZARD_API IPackageManager {
	protected:
		explicit IPackageManager(PackageManager& impl);
		~IPackageManager() = default;

	public:
		void SnapshotPackages(const std::filesystem::path& filepath, bool prettify);

	private:
		PackageManager& _impl;
	};
}