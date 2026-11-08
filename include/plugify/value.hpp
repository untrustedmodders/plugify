#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "plugify/global.h"

namespace plugify {
	class Method;

	/**
	 * @class Value
	 * @brief A single entry of an enumeration declared in a manifest.
	 *
	 * Each value pairs an identifier with the integer it maps to, which is what
	 * gives an enum a consistent representation across every language that binds
	 * to the extension. The class is a PIMPL wrapper over the parsed manifest
	 * entry.
	 *
	 * @see Enum
	 */
	class PLUGIFY_API Value {
	public:
		/**
		 * @brief Construct an empty value.
		 */
		Value();

		/**
		 * @brief Destructor.
		 */
		~Value();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Value.
		 */
		Value(const Value& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Value.
		 */
		Value(Value&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Value.
		 * @return A reference to this instance after copying.
		 */
		Value& operator=(const Value& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Value.
		 * @return A reference to this instance after moving.
		 */
		Value& operator=(Value&& other) noexcept;

		// Getters

		/**
		 * @brief Get the identifier of this entry.
		 * @return The name of the enum entry, unique within its enumeration.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Get the integer this entry maps to.
		 * @return The integer value of the enum entry.
		 */
		[[nodiscard]] int64_t GetValue() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the identifier of this entry.
		 * @param name The name of the enum entry.
		 */
		void SetName(std::string name);

		/**
		 * @brief Set the integer this entry maps to.
		 * @param value The integer value of the enum entry.
		 */
		void SetValue(int64_t value);

		/**
		 * @brief Compare two values for equality.
		 * @param other The value to compare against.
		 * @return True if both entries hold the same name and integer.
		 */
		[[nodiscard]] bool operator==(const Value& other) const noexcept;

		/**
		 * @brief Order two values.
		 * @param other The value to compare against.
		 * @return The relative ordering of the two entries.
		 */
		[[nodiscard]] auto operator<=>(const Value& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
