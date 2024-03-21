#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <plugify_export.h>

namespace plugify {
	class PackageManager;
	struct LocalPackage;
	struct RemotePackage;

	/**
	 * @typedef LocalPackageRef
	 * @brief Reference wrapper for constant LocalPackage.
	 */
	using LocalPackageRef = std::reference_wrapper<const LocalPackage>;

	/**
	 * @typedef RemotePackageRef
	 * @brief Reference wrapper for constant RemotePackage.
	 */
	using RemotePackageRef = std::reference_wrapper<const RemotePackage>;

	/**
	 * @typedef LocalPackageOpt
	 * @brief Optional reference to a constant LocalPackage.
	 */
	using LocalPackageOpt = std::optional<LocalPackageRef>;

	/**
	 * @typedef RemotePackageOpt
	 * @brief Optional reference to a constant RemotePackage.
	 */
	using RemotePackageOpt = std::optional<RemotePackageRef>;

	/**
	 * @class IPackageManager
	 * @brief Interface for the package manager provided to the user, implemented in the core.
	 */
	class PLUGIFY_API IPackageManager {
	protected:
		explicit IPackageManager(PackageManager& impl);
		~IPackageManager() = default;

	public:
		/**
		 * @brief Initialize the package manager.
		 * @return True if initialization is successful, false otherwise.
		 */
		bool Initialize() const;

		/**
		 * @brief Terminate the package manager.
		 */
		void Terminate() const;

		/**
		 * @brief Check if the package manager is initialized.
		 * @return True if the package manager is initialized, false otherwise.
		 */
		[[nodiscard]] bool IsInitialized() const;
		
		/**
		 * @brief Reload the package manager.
		 * @return True if was initializated, false otherwise.
		 */
		bool Reload() const;

		/**
		 * @brief Install a package.
		 * @param packageName Name of the package to install.
		 * @param requiredVersion Optional required version of the package.
		 */
		void InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion = {}) const;

		/**
		 * @brief Install multiple packages.
		 * @param packageNames Span of package names to install.
		 */
		void InstallPackages(std::span<const std::string> packageNames) const;

		/**
		 * @brief Install all packages listed in a manifest file.
		 * @param manifestFilePath Path to the manifest file.
		 * @param reinstall True to reinstall packages, false otherwise.
		 */
		void InstallAllPackages(const std::filesystem::path& manifestFilePath, bool reinstall) const;

		/**
		 * @brief Install all packages listed in a manifest file from a remote location.
		 * @param manifestUrl URL of the manifest file.
		 * @param reinstall True to reinstall packages, false otherwise.
		 */
		void InstallAllPackages(const std::string& manifestUrl, bool reinstall) const;

		/**
		 * @brief Update a specific package.
		 * @param packageName Name of the package to update.
		 * @param requiredVersion Optional required version of the package.
		 */
		void UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion = {}) const;

		/**
		 * @brief Update multiple packages.
		 * @param packageNames Span of package names to update.
		 */
		void UpdatePackages(std::span<const std::string> packageNames) const;

		/**
		 * @brief Update all installed packages.
		 */
		void UpdateAllPackages() const;

		/**
		 * @brief Uninstall a specific package.
		 * @param packageName Name of the package to uninstall.
		 */
		void UninstallPackage(const std::string& packageName) const;

		/**
		 * @brief Uninstall multiple packages.
		 * @param packageNames Span of package names to uninstall.
		 */
		void UninstallPackages(std::span<const std::string> packageNames) const;

		/**
		 * @brief Uninstall all installed packages.
		 */
		void UninstallAllPackages() const;

		/**
		 * @brief Snapshot the list of installed packages to a manifest file.
		 * @param manifestFilePath Path to the manifest file.
		 * @param prettify True to prettify the output, false for compact format.
		 */
		void SnapshotPackages(const std::filesystem::path& manifestFilePath, bool prettify) const;

		/**
		 * @brief Check if there are missed packages (not installed but required by other packages).
		 * @return True if there are missed packages, false otherwise.
		 */
		[[nodiscard]] bool HasMissedPackages() const;

		/**
		 * @brief Check if there are conflicted packages (installed with conflicting versions).
		 * @return True if there are conflicted packages, false otherwise.
		 */
		[[nodiscard]] bool HasConflictedPackages() const;

		/**
		 * @brief Install missed packages.
		 */
		void InstallMissedPackages() const;

		/**
		 * @brief Uninstall conflicted packages.
		 */
		void UninstallConflictedPackages() const;

		/**
		 * @brief Find a local package by name.
		 * @param packageName Name of the package to find.
		 * @return Optional reference to the found local package.
		 */
		[[nodiscard]] LocalPackageOpt FindLocalPackage(const std::string& packageName) const;

		/**
		 * @brief Find a remote package by name.
		 * @param packageName Name of the package to find.
		 * @return Optional reference to the found remote package.
		 */
		[[nodiscard]] RemotePackageOpt FindRemotePackage(const std::string& packageName) const;

		/**
		 * @brief Get a vector of references to all local packages.
		 * @return Vector of local package references.
		 */
		[[nodiscard]] std::vector<LocalPackageRef> GetLocalPackages() const;

		/**
		 * @brief Get a vector of references to all remote packages.
		 * @return Vector of remote package references.
		 */
		[[nodiscard]] std::vector<RemotePackageRef> GetRemotePackages() const;

	private:
		PackageManager& _impl; ///< Reference to the underlying PackageManager implementation.
	};

} // namespace plugify