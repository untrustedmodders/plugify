#pragma once

#include "package.h"
#include "wizard_context.h"
#include "wizard/config.h"

namespace wizard {
	class IWizard;
	class PackageDownloader {
	public:
		explicit PackageDownloader(Config config);
		~PackageDownloader();

		/**
		 * Retrieves the verified packages list from the central authority.
		 *
		 * The Wizard auto-downloading feature does not allow automatically installing
		 * all packages for various (notably security) reasons; packages that are candidate to
		 * auto-downloading are rather listed on a GitHub repository
		 * (https://raw.githubusercontent.com/untrustedpackageders/verified_packages/verified-packages.json),
		 * which this method gets via a HTTP call to load into local state.
		 *
		 * If list fetching fails, local packages list will be initialized as empty, thus
		 * preventing any package from being auto-downloaded.
		 */
		void FetchPackagesListFromAPI();

		/**
		 * Checks whether a package is verified.
		 *
		 * A package is deemed verified/authorized through a manual validation process that is
		 * described here: https://github.com/untrustedpackageders/verified_packages/README.md;
		 */
		bool IsPackageAuthorized(const RemotePackage& package);

		std::optional<PackageManifest> FetchPackageManifest(const std::string& url);
		std::optional<RemotePackage> UpdatePackage(const LocalPackage& package);
		std::optional<fs::path> DownloadPackage(const RemotePackage& package);

		PackageState GetState() const {
			return _packageState;
		}

	private:
		struct TempFile {
			explicit TempFile(fs::path filePath);
			~TempFile();
			fs::path path;
		};

		/**
		 * Downloads a package archive from using url. Fetched archive is then stored in a temporary location.
		 *
		 * If something went wrong during archive download, this will return an empty
		 * optional object.
		 */
		std::optional<TempFile> FetchPackageFromURL(const RemotePackage& package);


		std::optional<std::string> FetchJsonFromURL(const std::string& url);

		/**
		 * Tells if a package archive has not been corrupted.
		 *
		 * The package validation procedure includes computing the SHA256 hash of the final
		 * archive, which is stored in the verified packages list. This hash is used by this
		 * very method to ensure the archive downloaded from the Internet is the exact
		 * same that has been manually verified.
		 */
		bool IsPackageLegit(const RemotePackage& package, const fs::path& packagePath);

		bool ExtractPackage(const fs::path& packagePath, const fs::path& extractPath, bool isModule);

	private:
		struct VerifiedPackageVersion {
			std::string checksum;
			std::string commitHash;
		};
		struct VerifiedPackageDetails {
			std::string username;
			std::string repository;
			std::unordered_map<std::string, VerifiedPackageVersion> versions;
		};
		struct VerifiedPackageMap {
			std::unordered_map<std::string, VerifiedPackageDetails> verified;
		};

		static int PackageFetchingProgressCallback(void* ptr, ssize_t totalDownloadSize, ssize_t finishedDownloadSize, ssize_t totalToUpload, ssize_t nowUploaded);

		static inline const char* const kDefaultPackageList = "https://raw.githubusercontent.com/untrustedmodders/verified_packages/main/verified-packages.json";

	private:
		VerifiedPackageMap _packages;
		PackageState _packageState;
		Config _config;
	};

	inline bool IsValidURL(std::string_view str) {
		return str.starts_with("http://") || str.starts_with("https://");
	}
}