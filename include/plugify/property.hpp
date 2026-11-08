#pragma once

#include <memory>

#include "plugify/global.h"
#include "plugify/value_type.hpp"

namespace plugify {
	class Enum;
	class Method;

	/**
	 * @class Property
	 * @brief One parameter or the return value of a method.
	 *
	 * A property is a type from Plugify's type system plus everything needed to
	 * describe that type across languages: whether it is passed by reference, the
	 * signature it has when it is a callback, and the enumeration it belongs to
	 * when it is an enum.
	 *
	 * The class is a PIMPL wrapper over the parsed manifest entry.
	 *
	 * @see Method::GetParamTypes, Method::GetRetType
	 */
	class PLUGIFY_API Property {
	public:
		/**
		 * @brief Construct an empty property.
		 */
		Property();

		/**
		 * @brief Destructor.
		 */
		~Property();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Property.
		 */
		Property(const Property& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Property.
		 */
		Property(Property&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Property.
		 * @return A reference to this instance after copying.
		 */
		Property& operator=(const Property& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Property.
		 * @return A reference to this instance after moving.
		 */
		Property& operator=(Property&& other) noexcept;

		// Getters

		/**
		 * @brief Get the data type of this property.
		 * @return The value type, guaranteed to have the same representation in every language.
		 */
		[[nodiscard]] ValueType GetType() const noexcept;

		/**
		 * @brief Check whether this property is passed by reference.
		 * @return True if the callee can write through it and the caller observes the change.
		 * @note This changes the ABI of the method, so both sides of the call have to agree on it.
		 */
		[[nodiscard]] bool IsRef() const noexcept;

		/**
		 * @brief Get the signature of this property, when it is a callback.
		 * @return The prototype describing the callback, or nullptr for any other type.
		 * @note Only meaningful for @ref ValueType::Function. Valid only after
		 * Manifest::Resolve() has linked by-name references to their definitions.
		 */
		[[nodiscard]] std::shared_ptr<Prototype> GetPrototype() const noexcept;

		/**
		 * @brief Get the enumeration this property belongs to.
		 * @return The enumeration with all of its entries, or nullptr if the property is not an enum.
		 * @note Valid only after Manifest::Resolve() has linked by-name references to their
		 * definitions.
		 */
		[[nodiscard]] std::shared_ptr<Enum> GetEnumerate() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the data type of this property.
		 * @param type The value type.
		 */
		void SetType(ValueType type);

		/**
		 * @brief Set whether this property is passed by reference.
		 * @param ref True to pass it by reference.
		 */
		void SetRef(bool ref);

		/**
		 * @brief Set the callback signature of this property.
		 * @param prototype The prototype describing the callback.
		 */
		void SetPrototype(std::shared_ptr<Prototype> prototype);

		/**
		 * @brief Set the enumeration this property belongs to.
		 * @param enumerate The enumeration.
		 */
		void SetEnumerate(std::shared_ptr<Enum> enumerate);

		/**
		 * @brief Compare two properties for equality.
		 * @param other The property to compare against.
		 * @return True if both describe the same type in the same way.
		 */
		[[nodiscard]] bool operator==(const Property& other) const noexcept;

		/**
		 * @brief Order two properties.
		 * @param other The property to compare against.
		 * @return The relative ordering of the two properties.
		 */
		[[nodiscard]] auto operator<=>(const Property& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
