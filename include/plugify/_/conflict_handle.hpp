#pragma once

#include <span>
#include <string>

#include "plugify/core/constraint.hpp"
#include "handle.hpp"

#include "plugify_export.h"

namespace plugify {
	struct Conflict;

	/**
	 * @class ConflictHandle
	 * @brief A handle class for the `ConflictHandle` structure.
	 */
	class PLUGIFY_API ConflictHandle : public Handle<const Conflict> {
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
		 *
		 *
		 * @return
		 */
		const std::string& GetReason() const noexcept;

	};
} // namespace plugify
