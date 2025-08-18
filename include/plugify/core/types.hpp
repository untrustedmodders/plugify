#pragma once

#include <string>
#include <vector>
#include <optional>

#include <plg/version.hpp>

namespace plugify {
	/**
	 *
	 */
	using PackageId = std::string;

	/**
	 * @typedef Version
	 * @brief Represents a version of a package.
	 *
	 * This type is an alias for plg::version, which encapsulates the versioning
	 * information for packages in the Plugify ecosystem.
	 */
	using Version = plg::version;
}