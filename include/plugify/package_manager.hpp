#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <memory>
#include <plugify_export.h>

namespace plugify {
	struct LocalPackage;
	struct RemotePackage;

	/**
	 * @typedef LocalPackageRef
	 * @brief Shared pointer of a LocalPackage.
	 */
	using LocalPackagePtr = std::shared_ptr<LocalPackage>;

	/**
	 * @typedef RemotePackageRef
	 * @brief Shared pointer of a RemotePackage.
	 */
	using RemotePackagePtr = std::shared_ptr<RemotePackage>;

	/**
	 * @class IPackageManager
	 * @brief Interface for the package manager provided to the user, implemented in the core.
	 */
	class IPackageManager {
	public:
		virtual ~IPackageManager() = default;

		/**
		 * @brief Initialize the package manager.
		 * @return True if initialization is successful, false otherwise.
		 */
		virtual bool Initialize() = 0;

		/**
		 * @brief Terminate the package manager.
		 */
		virtual void Terminate() = 0;

		/**
		 * @brief Check if the package manager is initialized.
		 * @return True if the package manager is initialized, false otherwise.
		 */
		virtual bool IsInitialized() const = 0;
		
		/**
		 * @brief Reload the package manager.
		 * @return True if was initializated, false otherwise.
		 */
		virtual bool Reload() = 0;

		/**
		 * @brief Install a package.
		 * @param packageName Name of the package to install.
		 * @param requiredVersion Optional required version of the package.
		 */
		virtual void InstallPackage(std::string_view packageName, std::optional<int32_t> requiredVersion = {}) = 0;

		/**
		 * @brief Install multiple packages.
		 * @param packageNames Span of package names to install.
		 */
		virtual void InstallPackages(std::span<const std::string> packageNames) = 0;

		/**
		 * @brief Install all packages listed in a manifest file.
		 * @param manifestFilePath Path to the manifest file.
		 * @param reinstall True to reinstall packages, false otherwise.
		 */
		virtual void InstallAllPackages(const std::filesystem::path& manifestFilePath, bool reinstall) = 0;

		/**
		 * @brief Install all packages listed in a manifest file from a remote location.
		 * @param manifestUrl URL of the manifest file.
		 * @param reinstall True to reinstall packages, false otherwise.
		 */
		virtual void InstallAllPackages(const std::string& manifestUrl, bool reinstall) = 0;

		/**
		 * @brief Update a specific package.
		 * @param packageName Name of the package to update.
		 * @param requiredVersion Optional required version of the package.
		 */
		virtual void UpdatePackage(std::string_view packageName, std::optional<int32_t> requiredVersion = {}) = 0;

		/**
		 * @brief Update multiple packages.
		 * @param packageNames Span of package names to update.
		 */
		virtual void UpdatePackages(std::span<const std::string> packageNames) = 0;

		/**
		 * @brief Update all installed packages.
		 */
		virtual void UpdateAllPackages() = 0;

		/**
		 * @brief Uninstall a specific package.
		 * @param packageName Name of the package to uninstall.
		 */
		virtual void UninstallPackage(std::string_view packageName) = 0;

		/**
		 * @brief Uninstall multiple packages.
		 * @param packageNames Span of package names to uninstall.
		 */
		virtual void UninstallPackages(std::span<const std::string> packageNames) = 0;

		/**
		 * @brief Uninstall all installed packages.
		 */
		virtual void UninstallAllPackages() = 0;

		/**
		 * @brief Snapshot the list of installed packages to a manifest file.
		 * @param manifestFilePath Path to the manifest file.
		 * @param prettify True to prettify the output, false for compact format.
		 */
		virtual void SnapshotPackages(const std::filesystem::path& manifestFilePath, bool prettify) = 0;

		/**
		 * @brief Check if there are missed packages (not installed but required by other packages).
		 * @return True if there are missed packages, false otherwise.
		 */
		virtual bool HasMissedPackages() const = 0;

		/**
		 * @brief Check if there are conflicted packages (installed with conflicting versions).
		 * @return True if there are conflicted packages, false otherwise.
		 */
		virtual bool HasConflictedPackages() const = 0;

		/**
		 * @brief Install missed packages.
		 */
		virtual void InstallMissedPackages() = 0;

		/**
		 * @brief Uninstall conflicted packages.
		 */
		virtual void UninstallConflictedPackages() = 0;

		/**
		 * @brief Find a local package by name.
		 * @param packageName Name of the package to find.
		 * @return Shared pointer to the found local package.
		 */
		virtual LocalPackagePtr FindLocalPackage(std::string_view packageName) const = 0;

		/**
		 * @brief Find a remote package by name.
		 * @param packageName Name of the package to find.
		 * @return Shared pointer to the found remote package.
		 */
		virtual RemotePackagePtr FindRemotePackage(std::string_view packageName) const = 0;

		/**
		 * @brief Get a vector of all local packages.
		 * @return Vector of local package shared pointers.
		 */
		virtual std::vector<LocalPackagePtr> GetLocalPackages() const = 0;

		/**
		 * @brief Get a vector of all remote packages.
		 * @return Vector of remote package shared pointers.
		 */
		virtual std::vector<RemotePackagePtr> GetRemotePackages() const = 0;
	};

} // namespace plugify
