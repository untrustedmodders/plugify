#pragma once

#include <string>
#include <variant>
#include <vector>
#include <span>
#include <filesystem>

#include "package.hpp"
#include "package_manager.hpp"

namespace plugify {
	/**
	 * @brief Abstract repository interface for package sources
	 */
	class IPackageRepository {
	public:
		virtual ~IPackageRepository() = default;

		/**
		 * @brief Enumerate all packages available in this repository
		 */
		virtual Result<std::vector<Package>> EnumeratePackages() = 0;

		/**
		 * @brief Search for packages matching criteria
		 */
		virtual Result<std::vector<Package>> SearchPackages(std::string_view query) = 0;

		/**
		 * @brief Get detailed information about a specific package
		 */
		virtual Result<Package> GetPackage(const PackageId& id, const std::optional<plg::version>& version = {}) = 0;

		/**
		 * @brief Download package content to specified location
		 */
		virtual Result<std::filesystem::path> DownloadPackage(const Package& package, const std::filesystem::path& destination, ProgressCallback progress = {}) = 0;

		/**
		 * @brief Verify package integrity
		 */
		virtual Result<bool> VerifyPackage(const Package& package, const std::filesystem::path& path) = 0;

		/**
		 * @brief Get repository name/identifier
		 */
		virtual std::string GetName() const = 0;

		/**
		 * @brief Check if repository is available/online
		 */
		virtual Result<bool> IsAvailable() = 0;
	};
} // namespace plugify
