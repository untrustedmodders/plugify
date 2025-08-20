#pragma once

#include <span>
#include <string>

#include "../core/constraint.hpp"
#include "handle.hpp"

#include "plugify_export.h"

namespace plugify {
	struct Dependency;

	/**
	 * @class DependencyHandle
	 * @brief A handle class for the `DependencyHandle` structure.
	 */
	class PLUGIFY_API DependencyHandle : public Handle<const Dependency> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the name of the referenced plugin.
		 *
		 * @return A string view representing the name of the plugin.
		 */
		const std::string& GetName() const noexcept;

		/**
		 *
		 * @return
		 */
		std::span<const Constraint> GetConstrants() const noexcept;

		/**
		 * @brief Checks if the referenced plugin is optional.
		 *
		 * @return `true` if the plugin is optional, otherwise `false`.
		 */
		bool IsOptional() const noexcept;

	};
} // namespace plugify
