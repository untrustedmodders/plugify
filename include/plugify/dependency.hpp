#pragma once

#include <memory>
#include <string>

#include "plugify/global.h"
#include "plugify/types.hpp"

namespace plugify {
	class Manager;

	/**
	 * @class Dependency
	 * @brief Another extension that an extension needs in order to work.
	 *
	 * The dependency resolver loads every dependency before the extension that
	 * declares it, and checks the version of the dependency against the declared
	 * constraints. A missing or out-of-range dependency keeps the extension from
	 * loading unless the dependency is marked optional.
	 *
	 * The class is a PIMPL wrapper over the parsed manifest entry.
	 *
	 * @see Conflict, IDependencyResolver
	 */
	class PLUGIFY_API Dependency {
	public:
		/**
		 * @brief Construct an empty dependency.
		 */
		Dependency();

		/**
		 * @brief Destructor.
		 */
		~Dependency();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Dependency.
		 */
		Dependency(const Dependency& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Dependency.
		 */
		Dependency(Dependency&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Dependency.
		 * @return A reference to this instance after copying.
		 */
		Dependency& operator=(const Dependency& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Dependency.
		 * @return A reference to this instance after moving.
		 */
		Dependency& operator=(Dependency&& other) noexcept;

		// Getters

		/**
		 * @brief Get the name of the required extension.
		 * @return The name of a plugin or language module, as written in its own manifest.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Get the accepted versions of the required extension.
		 * @return The version constraints; an empty set accepts any version.
		 */
		[[nodiscard]] const Constraint& GetConstraints() const noexcept;

		/**
		 * @brief Check whether the extension can load without this dependency.
		 * @return True if the dependency is optional, false if it is required.
		 */
		[[nodiscard]] bool IsOptional() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the name of the required extension.
		 * @param name The name of the plugin or language module.
		 */
		void SetName(std::string name);

		/**
		 * @brief Set the accepted versions of the required extension.
		 * @param constraints The version constraints.
		 */
		void SetConstraints(Constraint constraints);

		/**
		 * @brief Set whether the extension can load without this dependency.
		 * @param optional True to make the dependency optional.
		 */
		void SetOptional(bool optional);

		/**
		 * @brief Compare two dependencies for equality.
		 * @param other The dependency to compare against.
		 * @return True if both name the same extension under the same constraints.
		 */
		[[nodiscard]] bool operator==(const Dependency& other) const noexcept;

		/**
		 * @brief Order two dependencies.
		 * @param other The dependency to compare against.
		 * @return The relative ordering of the two dependencies.
		 */
		[[nodiscard]] auto operator<=>(const Dependency& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
