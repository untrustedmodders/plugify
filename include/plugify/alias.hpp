#pragma once

#include <memory>
#include <string>

#include "plugify/global.h"

namespace plugify {
	/**
	 * @class Alias
	 * @brief An alternative type name for a parameter or a return value.
	 *
	 * An alias replaces the raw type name in generated bindings and in
	 * documentation, so a manifest can give a domain-specific name to a value
	 * whose underlying type is generic (a handle passed as a pointer, for
	 * instance). It also carries the ownership semantics of that value.
	 *
	 * Aliases come from the `paramAliases` and `retAlias` fields of a class
	 * binding, and from a class's handle alias. The class is a PIMPL wrapper
	 * over the parsed manifest entry.
	 */
	class PLUGIFY_API Alias {
	public:
		/**
		 * @brief Construct an empty alias.
		 */
		Alias();

		/**
		 * @brief Destructor.
		 */
		~Alias();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Alias.
		 */
		Alias(const Alias& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Alias.
		 */
		Alias(Alias&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Alias.
		 * @return A reference to this instance after copying.
		 */
		Alias& operator=(const Alias& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Alias.
		 * @return A reference to this instance after moving.
		 */
		Alias& operator=(Alias&& other) noexcept;

		// Getters

		/**
		 * @brief Get the aliased type name.
		 * @return The name used in generated bindings and documentation instead of the raw type.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Check whether the aliased value carries ownership.
		 * @return True if the value owns the resource it names, false otherwise.
		 * @note Used by language modules to decide between move semantics, a transfer of
		 * ownership, and a plain borrow. Defaults to false when the manifest omits it.
		 */
		[[nodiscard]] bool IsOwner() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the aliased type name.
		 * @param name The name to use instead of the raw type.
		 */
		void SetName(std::string name);

		/**
		 * @brief Set the ownership semantics of the aliased value.
		 * @param owner True if the value owns the resource it names.
		 */
		void SetOwner(bool owner);

		/**
		 * @brief Compare two aliases for equality.
		 * @param other The alias to compare against.
		 * @return True if both aliases hold the same values.
		 */
		[[nodiscard]] bool operator==(const Alias& other) const noexcept;

		/**
		 * @brief Order two aliases.
		 * @param other The alias to compare against.
		 * @return The relative ordering of the two aliases.
		 */
		[[nodiscard]] auto operator<=>(const Alias& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
