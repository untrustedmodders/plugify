#pragma once

#include <memory>
#include <string>
#include <vector>

#include "plugify/global.h"

namespace plugify {
	class Value;

	/**
	 * @class Enum
	 * @brief An enumeration declared in a manifest.
	 *
	 * An enum names a set of integer constants, which language modules turn into
	 * a native enum type in the languages they bind to. A manifest may declare it
	 * in the top-level `enums` array and refer to it by name from any property,
	 * or write it inline on the property that uses it; after Manifest::Resolve()
	 * both forms are reachable from the same flat list.
	 *
	 * The class is a PIMPL wrapper over the parsed manifest entry.
	 *
	 * @see Value, Property::GetEnumerate
	 */
	class PLUGIFY_API Enum {
	public:
		/**
		 * @brief Construct an empty enumeration.
		 */
		Enum();

		/**
		 * @brief Destructor.
		 */
		~Enum();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Enum.
		 */
		Enum(const Enum& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Enum.
		 */
		Enum(Enum&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Enum.
		 * @return A reference to this instance after copying.
		 */
		Enum& operator=(const Enum& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Enum.
		 * @return A reference to this instance after moving.
		 */
		Enum& operator=(Enum&& other) noexcept;

		// Getters

		/**
		 * @brief Get the type name of this enumeration.
		 * @return The name used to generate the enum type in target languages.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Get the entries of this enumeration.
		 * @return The complete list of entries, each a name mapped to an integer.
		 */
		[[nodiscard]] const std::vector<Value>& GetValues() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the type name of this enumeration.
		 * @param name The name of the enum type.
		 */
		void SetName(std::string name);

		/**
		 * @brief Set the entries of this enumeration.
		 * @param values The entries of the enum.
		 */
		void SetValues(std::vector<Value> values);

		/**
		 * @brief Compare two enumerations for equality.
		 * @param other The enumeration to compare against.
		 * @return True if both hold the same name and the same entries.
		 */
		[[nodiscard]] bool operator==(const Enum& other) const noexcept;

		/**
		 * @brief Order two enumerations.
		 * @param other The enumeration to compare against.
		 * @return The relative ordering of the two enumerations.
		 */
		[[nodiscard]] auto operator<=>(const Enum& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
