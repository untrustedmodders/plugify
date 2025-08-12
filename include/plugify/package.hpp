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

	struct PackageId {
		std::string name;
		PackageVersion version;

		bool operator==(const PackageId&) const = default;
		auto operator<=>(const PackageId&) const = default;
	};

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
	    Conflicted,      // Has unresolved conflicts
	    UpdateAvailable, // Installed but newer version available
	    Corrupted,       // Installed but manifest/files corrupted
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
		std::string name;
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
	    std::string name;
	    std::vector<VersionConstraint> constraints;  // Versions that conflict
	    std::optional<std::string> reason;

	    /**
	     * @brief Check if a package version would conflict
	     */
	    bool ConflictsWith(const PackageVersion& version) const;
	};

	struct Checksum {
		enum class Type {
			SHA256
		};
		Type algorithm{ Type::SHA256 };
		std::string value;
	};

	struct Signature {
		enum class Type {
			GPG
		};
		Type algorithm{ Type::GPG };
		std::string value;
	};

	struct Security {
		std::optional<Checksum> checksum;
		std::optional<Signature> signature;
	};

	struct RepositoryInfo {
		enum class Type {
			Zip,
			//Git, // Git repository
		};
		Type type{ Type::Zip };
		std::string url;
	};

	/**
	 * @brief Package manifest containing all metadata
	 */
	struct PackageManifest {
	    std::string name;
	    PackageVersion version;
		PackageType type;

		RepositoryInfo repository; // Repository information for remote packages
		std::string description;
		std::string license;
		std::string author;
		std::string website;

		std::optional<std::vector<std::string>> platforms;
		std::optional<std::vector<PackageDependency>> dependencies;
		std::optional<std::vector<PackageConflict>> conflicts;

		std::optional<std::unordered_map<std::string, std::string>> metadata;
		DateTime releaseDate;
		size_t installSize{0};

		std::optional<bool> privated;
		std::optional<Security> security;
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

		/*const std::string& GetName() const noexcept {
			return manifest->name;
		}

		plg::version GetVersion() const noexcept {
			return manifest->version;
		}*/

	    std::shared_ptr<PackageManifest> manifest;
	    PackageState state{PackageState::NotInstalled};
	    PackageLocation location;
		std::string error;
	    //std::optional<PackageVersion> installedVersion;
	    //std::vector<PackageInfo> availableVersions;
	};

	/*class IPackage {
	public:
		virtual ~IPackage() = default;

		virtual const PackageId& GetId() const = 0;
		virtual const PackageManifest& GetManifest() const = 0;
		virtual PackageState GetState() const = 0;
		virtual std::filesystem::path GetInstallPath() const = 0;
		virtual bool IsInstalled() const = 0;
		virtual std::optional<PackageVersion> GetInstalledVersion() const = 0;
		virtual std::vector<std::filesystem::path> GetFiles() const = 0;
	};*/

	//using PackagePtr = std::shared_ptr<Package>;

} // namespace plugify
