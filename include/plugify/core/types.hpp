#pragma once

#include <string>
#include <vector>
#include <optional>

#include "plg/version.hpp"
#include "plg/expected.hpp"

namespace plugify {
	/**
	 * @typedef Version
	 * @brief Represents a version of a package.
	 *
	 * This type is an alias for plg::version, which encapsulates the versioning
	 * information for packages in the Plugify ecosystem.
	 */
	using Version = plg::version<int, int, int>;

    /**
     * @typedef Constraint
     */
    using Constraint = plg::range_set<int, int, int>;

	/**
	 * @typedef UniqueId
	 * @brief Represents a unique identifier for modules.
	 */
	using UniqueId = std::ptrdiff_t;

	/**
	 * TODO:
	 */
	using PackageId = std::string;

    /**
     * TODO:
     */
    enum class PackageType {
		Module,
		Plugin
	};

	/**
	 * TODO:
	 */
	template<typename T>
	using Result = plg::expected<T, std::string>;

}