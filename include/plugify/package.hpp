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
#include "version.hpp"

namespace plugify {
	/**
	 * @brief Package types supported by the system
	 */
	enum class PackageType {
		LanguageModule,
		Plugin
	};

	/**
	 * @brief Unique package identifier
	 */
	struct PackageId {
	    std::string name;
	    std::string author;

	    auto operator<=>(const PackageId&) const = default;
	    bool operator==(const PackageId&) const = default;
	};

	/**
	 * @brief Version constraint for package requirements
	 */
	struct Constraint {
	    enum class Type {
	        Equal,        // Exactly this version
	        NotEqual,     // Any version except this
	        Greater,      // Strictly greater than
	        GreaterEqual, // Greater than or equal
	        Less,         // Strictly less than
	        LessEqual,    // Less than or equal
	        Compatible,   // Compatible with (e.g., same major version)
	        Any          // Any version acceptable
	    } type{Type::Any};
	    plg::version version{};

	    /**
	     * @brief Check if a version satisfies this constraint
	     */
	    bool IsSatisfiedBy(const plg::version& v) const;
	};

	/**
	 * @brief Package dependency specification with flexible constraints
	 */
	struct Dependency {
		PackageId id;
		std::vector<Constraint> constraints; // ANDed together
		bool optional{false};

		/**
		 * @brief Check if a package version satisfies all constraints
		 */
		bool IsSatisfiedBy(const plg::version& v) const;
	};

	/**
	 * @brief Package conflict specification with version constraints
	 */
	struct Conflict {
	    PackageId id;
	    std::vector<Constraint> constraints; // Package versions that conflict
	    std::optional<std::string> reason;

	    /**
	     * @brief Check if a package version triggers this conflict
	     */
	    bool IsConflictingWith(const plg::version& v) const;
	};

	/**
	 * @brief Complete package metadata
	 */
	struct PackageInfo {
	    PackageId id;
	    plg::version version;
	    PackageType type;
	    std::string description;
	    std::vector<Dependency> dependencies;
	    std::vector<Conflict> conflicts;
	    DateTime releaseDate;
	    size_t size{};
	    std::string checksum;
	    std::unordered_map<std::string, std::string> metadata;
	};

	/**
	 * @brief Package location abstraction
	 */
	struct PackageLocation {
	    std::variant<std::filesystem::path, std::string> source; // local path or remote URL
	    
	    bool IsLocal() const { return std::holds_alternative<std::filesystem::path>(source); }
	    bool IsRemote() const { return std::holds_alternative<std::string>(source); }
	};

	/**
	 * @brief Complete package representation
	 */
	struct Package {
	    PackageInfo info;
	    PackageLocation location;
	    bool installed{false};
	    std::optional<fs::path> installPath;
	};
} // namespace plugify

namespace std {
	template<>
	struct hash<plugify::PackageId> {
		size_t operator()(const plugify::PackageId& id) const noexcept {
			size_t h1 = std::hash<std::string>{}(id.name);
			size_t h2 = std::hash<std::string>{}(id.author);
			return h1 ^ (h2 << 1);
		}
	};
} // namespace std