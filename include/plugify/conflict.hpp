#pragma once

#include <memory>
#include <string>

#include "plugify/global.h"
#include "plugify/types.hpp"

namespace plugify {
	class Manager;

	/**
	 * @class Conflict
	 * @brief Another extension that cannot be loaded alongside this one.
	 *
	 * The dependency resolver refuses to load two extensions that declare each
	 * other, or one another, as a conflict. Use it for extensions that provide
	 * overlapping or incompatible functionality.
	 *
	 * The same type describes an obsolete relation under the @ref Obsolete alias:
	 * there, a matching extension counts as replaced by this one and the two are
	 * never loaded side by side, which is what a rename or a merge of two
	 * extensions needs.
	 *
	 * The class is a PIMPL wrapper over the parsed manifest entry.
	 *
	 * @see Dependency, IDependencyResolver
	 */
	class PLUGIFY_API Conflict {
	public:
		/**
		 * @brief Construct an empty conflict.
		 */
		Conflict();

		/**
		 * @brief Destructor.
		 */
		~Conflict();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Conflict.
		 */
		Conflict(const Conflict& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Conflict.
		 */
		Conflict(Conflict&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Conflict.
		 * @return A reference to this instance after copying.
		 */
		Conflict& operator=(const Conflict& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Conflict.
		 * @return A reference to this instance after moving.
		 */
		Conflict& operator=(Conflict&& other) noexcept;

		// Getters

		/**
		 * @brief Get the name of the conflicting extension.
		 * @return The name of a plugin or language module, as written in its own manifest.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Get the versions of the other extension this applies to.
		 * @return The version constraints; an empty set conflicts with every version.
		 */
		[[nodiscard]] const Constraint& GetConstraints() const noexcept;

		/**
		 * @brief Get the explanation of the conflict.
		 * @return A human-readable reason, reported when the resolver rejects the pair.
		 */
		[[nodiscard]] const std::string& GetReason() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the name of the conflicting extension.
		 * @param name The name of the plugin or language module.
		 */
		void SetName(std::string name);

		/**
		 * @brief Set the versions of the other extension this applies to.
		 * @param constraints The version constraints.
		 */
		void SetConstraints(Constraint constraints);

		/**
		 * @brief Set the explanation of the conflict.
		 * @param reason A human-readable reason.
		 */
		void SetReason(std::string reason);

		/**
		 * @brief Compare two conflicts for equality.
		 * @param other The conflict to compare against.
		 * @return True if both name the same extension under the same constraints.
		 */
		[[nodiscard]] bool operator==(const Conflict& other) const noexcept;

		/**
		 * @brief Order two conflicts.
		 * @param other The conflict to compare against.
		 * @return The relative ordering of the two conflicts.
		 */
		[[nodiscard]] auto operator<=>(const Conflict& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

	/**
	 * @typedef Obsolete
	 * @brief An extension that this one supersedes.
	 *
	 * Structurally identical to @ref Conflict, and resolved the same way: the
	 * named extension is treated as replaced by this one, so the two never load
	 * together.
	 */
	using Obsolete = Conflict;
}
