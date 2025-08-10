#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <variant>
#include <set>
#include <tuple>
#include <vector>

#include "date_time.hpp"
#include "descriptor.hpp"
#include "repository.hpp"
#include "version.hpp"

namespace plugify {
	/**
	 * @brief Version representation with semantic versioning support
	 *
	 * @note Follows Semantic Versioning 2.0.0 specification
	 *       MAJOR.MINOR.PATCH-PRERELEASE+BUILD
	 */
	using PackageVersion = plg::version;

	/**
	 * @brief Package type enumeration
	 */
	enum class PackageType {
	    LanguageModule,
	    Plugin
	};

	/**
	 * @brief Package state enumeration
	 */
	enum class PackageState {
	    NotInstalled,    // Remote package, not installed
	    Installed,       // Local package, installed
	    UpdateAvailable, // Installed but newer version available
	    Corrupted,       // Installed but manifest/files corrupted
	    Conflicted       // Has unresolved conflicts
	};

	/**
	 * @brief Version constraint types for flexible version requirements
	 */
	struct VersionConstraint {
	    enum class Type {
	        Equal,        // Exactly this version
	        NotEqual,     // Any version except this
	        Greater,      // Strictly greater than
	        GreaterEqual, // Greater than or equal to
	        Less,         // Strictly less than
	        LessEqual,    // Less than or equal to
	        Compatible,   // Compatible with (e.g., ^2.1.0 = >=2.1.0 <3.0.0)
	        Any           // Any version acceptable
	    };

	    Type type{Type::Any};
	    PackageVersion version;

	    /**
	     * @brief Check if a version satisfies this constraint
	     *
	     * @note For Compatible type:
	     *       - ^1.2.3 := >=1.2.3 <2.0.0 (compatible with 1.x.x)
	     *       - ^0.2.3 := >=0.2.3 <0.3.0 (0.x.x is special - minor is breaking)
	     *       - ^0.0.3 := >=0.0.3 <0.0.4 (0.0.x is special - patch is breaking)
	     */
	    bool IsSatisfiedBy(const PackageVersion& v) const;
	};

	/**
	 * @brief Package dependency with flexible constraints
	 *
	 * @note Multiple constraints are ANDed together. For example:
	 *       constraints = [">=2.0.0", "<3.0.0"] means version must be >=2.0.0 AND <3.0.0
	 *       This allows expressing complex version requirements precisely.
	 */
	struct PackageDependency {
		std::string packageName;
		std::vector<VersionConstraint> constraints;  // ANDed together
		std::optional<bool> optional;

		/**
		 * @brief Check if a package version satisfies all constraints
		 */
		bool IsSatisfiedBy(const PackageVersion& version) const;
	};

	/**
	 * @brief Conflict information with constraint-based versioning
	 */
	struct PackageConflict {
	    std::string packageName;
	    std::vector<VersionConstraint> constraints;  // Versions that conflict
	    std::optional<std::string> reason;

	    /**
	     * @brief Check if a package version would conflict
	     */
	    bool ConflictsWith(const PackageVersion& version) const;
	};

	/**
	 * @brief Package manifest containing all metadata
	 */
	struct PackageManifest {
	    std::string name;
	    PackageVersion version;
	    PackageType type;
	    std::vector<PackageDependency> dependencies;
		std::vector<PackageConflict> conflicts;
	    std::unordered_map<std::string, std::string> metadata;
	};

	/**
	 * @brief Local package extends base with filesystem information
	 */
	struct PackageLocal {
		std::filesystem::path installPath;
		std::filesystem::path manifestPath;
	};

	/**
	 * @brief Remote package extends base with repository information
	 */
	struct PackageRemote {
		std::string repositoryId;
		std::string downloadUrl;
		std::string checksum;  // SHA256 or similar
		std::string signature; // GPG signature if available
	};

	/**
	 * @brief Package location abstraction
	 */
	struct PackageLocation {
		std::variant<PackageLocal, PackageRemote> source; // local or remote

		bool IsLocal() const { return std::holds_alternative<PackageLocal>(source); }
		bool IsRemote() const { return std::holds_alternative<PackageRemote>(source); }

		/*const PackageLocal& ToLocal() const {
			if (IsLocal()) {
				return std::get<PackageLocal>(source);
			}
			throw std::bad_variant_access();
		}

		const PackageRemote& ToRemote() const {
			if (IsRemote()) {
				return std::get<PackageRemote>(source);
			}
			throw std::bad_variant_access();
		}*/
	};

	/**
	 * @brief Complete package information
	 */
	struct Package {
	    PackageManifest manifest;
	    PackageState state{PackageState::NotInstalled};
	    PackageLocation location;
	    //std::optional<PackageVersion> installedVersion;
	    //std::optional<PackageInfo> availableVersions;
	};
} // namespace plugify
