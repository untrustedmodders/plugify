#pragma once

#include <plugify/scanner.hpp>

namespace plugify {
	/**
	 * @brief Default package scanner implementation
	 */
	class PackageScanner : public IPackageScanner {
	private:
		PackageScannerConfig _config;
		mutable std::optional<std::vector<ScannedPackage>> _lastScanResults;
		std::atomic<bool> _watching{false};

		Result<PackageInfo> ParseJSONManifest(const fs::path& path);
		Result<PackageInfo> ParseYAMLManifest(const fs::path& path);
		Result<PackageInfo> ParseTOMLManifest(const fs::path& path);
		Result<PackageInfo> ParseXMLManifest(const fs::path& path);

	public:
		explicit PackageScanner(PackageScannerConfig config = {});

		Result<std::vector<ScannedPackage>> ScanForPackages(
			const PackageScannerConfig& config) override;

		Result<std::vector<ScannedPackage>> ScanDirectory(
			const fs::path& directory,
			bool recursive) override;

		Result<ScannedPackage> ScanPackage(const fs::path& packagePath) override;

		Result<PackageInfo> ParseManifest(
			const fs::path& manifestPath,
			ManifestFormat format) override;

		Result<bool> ValidatePackageStructure(const fs::path& packagePath) override;

		std::optional<std::vector<ScannedPackage>> GetLastScanResults() const override;

		Result<void> StartWatching(
			std::function<void(const ScannedPackage&, bool added)> callback) override;

		void StopWatching() override;
	};

} // nnamespace plugify