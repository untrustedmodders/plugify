#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

#include "descriptor.hpp"
#include "version.hpp"

namespace plugify {
	/**
	 * @struct PackageVersion
	 * @brief Represents a version of a software package.
	 *
	 * The PackageVersion structure holds information about a package version, including
	 * the version number, checksum, download URL, supported platforms, and optional
	 * dependencies and conflicts.
	 */
	struct PackageVersion final {
		plg::version version; ///< The semantic version of the package.
		std::string checksum; ///< The checksum of the package.
		std::string download; ///< The download URL for the package.
		std::optional<std::vector<std::string>> platforms; ///< The platforms supported by the package.

		/**
		 * @brief Overloaded less-than operator for comparing PackageVersion instances.
		 * @param rhs The right-hand side PackageVersion for comparison.
		 * @return True if the version of this instance is less than the version of rhs.
		 */
		bool operator <(const PackageVersion& rhs) const noexcept { return version > rhs.version || (version == rhs.version && std::tie(checksum, download, platforms) < std::tie(rhs.checksum, rhs.download, rhs.platforms)); }
	};

	/**
	 * @typedef PackageOpt
	 * @brief Represents an const pointer to a PackageVersion.
	 */
	using PackageOpt = const PackageVersion*;

	/**
	 * @struct Package
	 * @brief Represents a generic software package.
	 *
	 * The Package structure holds information about a generic software package, including
	 * the name and type. It also provides an equality operator for comparing instances.
	 */
	struct Package {
		std::string name; ///< The name of the package.
		std::string type; ///< The type of the package (e.g., module, plugin).

		/**
		 * @brief Overloaded equality operator for comparing Package instances.
		 * @param rhs The right-hand side Package for comparison.
		 * @return True if the name and type of this instance are equal to those of rhs.
		 */
		bool operator==(const Package& rhs) const noexcept { return name == rhs.name && type == rhs.type; }
	};

	/**
	 * @struct RemotePackage
	 * @brief Represents a remotely available software package.
	 *
	 * The RemotePackage structure extends the Package structure to include additional
	 * information such as author, description, and available versions. It also provides
	 * methods to retrieve the latest version or a specific version of the package.
	 */
	struct RemotePackage : public Package {
		std::string author; ///< The author of the package.
		std::string description; ///< The description of the package.
		std::set<PackageVersion> versions; ///< The set of available versions for the package.

		/**
		 * @brief Get the latest available version of the package.
		 * @return A pointer to the latest PackageVersion.
		 */
		PackageOpt LatestVersion() const noexcept {
			if (!versions.empty())
				return &(*versions.begin());
			return {};
		}

		/**
		 * @brief Get a specific version of the package.
		 * @param version The semantic version to retrieve.
		 * @return A pointer to the specified PackageVersion.
		 */
		PackageOpt Version(const plg::version& version) const {
			auto it = versions.find(PackageVersion{ version, {}, {}, {} }); // dummy key for lookup
			if (it != versions.end())
				return &(*it);
			return {};
		}
	};

	/**
	 * @struct LocalPackage
	 * @brief Represents a locally installed software package.
	 *
	 * The LocalPackage structure extends the Package structure to include information
	 * about the local installation, such as the file path, version, and descriptor.
	 * It also provides a conversion operator to convert to a RemotePackage for compatibility.
	 */
	struct LocalPackage : public Package {
		std::filesystem::path path; ///< The file path to the locally installed package.
		plg::version version; ///< The semantic version of the locally installed package.
		std::shared_ptr<Descriptor> descriptor; ///< A shared pointer to the package descriptor.

		/**
		 * @brief Conversion operator to convert LocalPackage to RemotePackage.
		 * @return A RemotePackage instance representing the local package.
		 */
		explicit operator RemotePackage() const {
			return { name, type, descriptor->createdBy.value_or("unknown"), descriptor->description.value_or(""), { PackageVersion{ descriptor->version, {}, descriptor->downloadURL.value_or(""), descriptor->supportedPlatforms } } };
		}
	};
} // namespace plugify
