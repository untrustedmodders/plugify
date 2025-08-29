#pragma once

#include <optional>
#include <string>
#include <vector>

//#include "plg/uuid.hpp"
#include "plg/version.hpp"
#include "plg/expected.hpp"
#include "plg/flat_map.hpp"
#include "plg/enum.hpp"

namespace plugify {
	/**
	 * @typedef Version
	 * @brief Represents a version of a extension.
	 *
	 * This type is an alias for plg::version, which encapsulates the versioning
	 * information for extensions in the Plugify ecosystem.
	 */
	using Version = plg::version<>;

    /**
     * @typedef Constraint
     */
    using Constraint = plg::range_set<>;

	/**
	 * @typedef UniqueId
	 * @brief Represents a unique identifier for everything.
	 */
	using UniqueId = std::ptrdiff_t;

    /**
     * TODO:
     */
    enum class ExtensionType {
		Unknown,
		Module,
		Plugin
	};

	/**
	 * TODO:
	 */
	template<typename T>
	using Result = plg::expected<T, std::string>;

    /**
     *
     */
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::microseconds;
}