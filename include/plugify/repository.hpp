#pragma once

#include <string>
#include <variant>
#include <vector>
#include <span>
#include <filesystem>
#include <unordered_map>

#include "package.hpp"
#include "package_manager.hpp"

namespace plugify {
	/**
	 * @brief Search criteria for querying packages
	 */
	struct PackageQuery {
		std::optional<std::string> namePattern;
		std::optional<PackageType> type;
		std::optional<PackageState> state;
		std::optional<std::string> author;
		std::optional<std::vector<std::string>> tags;

		// Range-based filtering support
		//auto Filter(std::span<const Package> packages) const -> std::vector<Package>;
	};

	/**
	 * @brief Package constraint for flexible version requirements
	 */
	struct PackageRelease {
		PackageVersion version;
		std::string checksum;
		std::string download;
		std::optional<std::vector<std::string>> platforms;
		std::optional<std::vector<PackageDependency>> dependencies;
		std::optional<std::vector<PackageConflict>> conflicts;
	};

	/**
	 * @brief Remote package representation
	 */
	struct PackageInfo {
		std::string name;
		std::string type;
		std::string author;
		std::string licence;
		std::string description;
		std::vector<PackageRelease> versions;
		std::optional<std::unordered_map<std::string, std::string>> metadata;
	};

	/**
	 * @brief Repository containing remote packages
	 */
	struct PackageRepository {
		std::unordered_map<std::string, PackageInfo> content;
	};

	/**
	 * @brief Abstract repository interface for package sources
	 */
	class IPackageRepository {
	public:
		virtual ~IPackageRepository() = default;

		/**
		 * @brief Get repository identifier
		 */
		virtual std::string GetIdentifier() const = 0;

		/**
		 * @brief Check if repository is available
		 */
		virtual bool IsAvailable() const = 0;

		/**
		 * @brief Refresh repository metadata
		 */
		virtual Result<void> Refresh() = 0;

		/**
		 * @brief List all packages in repository
		 */
		virtual Result<std::vector<Package>> GetPackages() const = 0;

		/**
		 * @brief Find package by ID
		 */
		virtual std::optional<Package> FindPackage(std::string_view packageId, const std::optional<plg::version>& version = {}) const = 0;

		/**
		 * @brief Query packages with criteria
		 */
		virtual std::vector<Package> QueryPackages(const PackageQuery& query) const = 0;

		/**
		 * @brief Get package manifest
		 */
		virtual Result<PackageManifest> GetManifest(std::string_view packageId) const = 0;
	};
} // namespace plugify
