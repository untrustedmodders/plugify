#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

#include "descriptor.hpp"
#include "constrant.hpp"
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
	struct PackageVersion {
		plg::version version; ///< The semantic version of the package.
		std::string checksum; ///< The checksum of the package.
		std::string download; ///< The download URL for the package.
		std::optional<std::vector<std::string>> platforms; ///< The platforms supported by the package.
		std::optional<std::vector<PackageConstraint>> dependencies; ///< The package dependencies.
		std::optional<std::vector<PackageConstraint>> conflicts; ///< The package conflicts.
	};

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
	};

	/**
	 * @struct RemotePackage
	 * @brief Represents a remotely available software package.
	 *
	 * The RemotePackage structure extends the Package structure to include additional
	 * information such as author, description, and available versions. It also provides
	 * methods to retrieve the latest version or a specific version of the package.
	 */
	struct RemotePackage : Package {
		std::string author; ///< The author of the package.
		std::string description; ///< The description of the package.
		std::vector<PackageVersion> versions; ///< The set of available versions for the package.
	};

	/**
	 * @struct LocalPackage
	 * @brief Represents a locally installed software package.
	 *
	 * The LocalPackage structure extends the Package structure to include information
	 * about the local installation, such as the file path, version, and descriptor.
	 * It also provides a conversion operator to convert to a RemotePackage for compatibility.
	 */
	struct LocalPackage : Package {
		std::filesystem::path path; ///< The file path to the locally installed package.
		plg::version version; ///< The semantic version of the locally installed package.
		std::shared_ptr<Descriptor> descriptor; ///< A shared pointer to the package descriptor.
	};

	/**
	 * @namespace PackageType
	 * @brief Contains constants for different package types.
	 *
	 * This namespace defines string constants representing different types of packages
	 * that can be managed by the plugify system, such as plugins.
	 */
	namespace PackageType {
		inline constexpr std::string_view Plugin = "plugin";
	}
} // namespace plugify
