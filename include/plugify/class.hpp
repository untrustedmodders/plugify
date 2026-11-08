#pragma once

#include <memory>
#include <vector>
#include <string>

#include "plugify/global.h"
#include "plugify/value_type.hpp"

namespace plugify {
	class Binding;

	/**
	 * @class Class
	 * @brief An object-oriented wrapper over a handle-based API.
	 *
	 * A class groups methods the extension already exports around the handle they
	 * operate on, so languages that support RAII can expose them as a type whose
	 * lifetime is managed: the constructors produce a handle, the destructor
	 * releases it, and the bindings become its instance methods.
	 *
	 * The class is a PIMPL wrapper over the parsed manifest entry.
	 *
	 * @see Binding
	 */
	class PLUGIFY_API Class {
	public:
		/**
		 * @brief Construct an empty class.
		 */
		Class();

		/**
		 * @brief Destructor.
		 */
		~Class();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Class.
		 */
		Class(const Class& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Class.
		 */
		Class(Class&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Class.
		 * @return A reference to this instance after copying.
		 */
		Class& operator=(const Class& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Class.
		 * @return A reference to this instance after moving.
		 */
		Class& operator=(Class&& other) noexcept;

		// Getters

		/**
		 * @brief Get the name of this class.
		 * @return The class name used in generated bindings.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Get the type of the handle this class wraps.
		 * @return The handle type, usually a pointer type, or @ref ValueType::Void for a
		 * handleless class that only groups static methods.
		 */
		[[nodiscard]] ValueType GetHandleType() const noexcept;

		/**
		 * @brief Get the value that stands for an invalid handle.
		 * @return The literal used to initialise a handle and to test it for validity.
		 */
		[[nodiscard]] const std::string& GetInvalidValue() const noexcept;

		/**
		 * @brief Get the methods that construct instances of this class.
		 * @return Public API names of methods that each return a handle of the handle type of this
		 * class.
		 */
		[[nodiscard]] const std::vector<std::string>& GetConstructors() const noexcept;

		/**
		 * @brief Get the method that destroys instances of this class.
		 * @return The public API name of the method that releases the handle, empty if the class
		 * has no destructor.
		 */
		[[nodiscard]] const std::string& GetDestructor() const noexcept;

		/**
		 * @brief Get the instance methods of this class.
		 * @return The bindings, each wrapping one of the methods the extension exports.
		 */
		[[nodiscard]] const std::vector<Binding>& GetBindings() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the name of this class.
		 * @param name The class name.
		 */
		void SetName(std::string name);

		/**
		 * @brief Set the type of the handle this class wraps.
		 * @param handleType The handle type.
		 */
		void SetHandleType(ValueType handleType);

		/**
		 * @brief Set the value that stands for an invalid handle.
		 * @param invalidValue The literal for an invalid handle.
		 */
		void SetInvalidValue(std::string invalidValue);

		/**
		 * @brief Set the methods that construct instances of this class.
		 * @param constructors Public API names of the constructing methods.
		 */
		void SetConstructors(std::vector<std::string> constructors);

		/**
		 * @brief Set the method that destroys instances of this class.
		 * @param destructor The public API name of the destroying method.
		 */
		void SetDestructor(std::string destructor);

		/**
		 * @brief Set the instance methods of this class.
		 * @param bindings The bindings to expose on instances.
		 */
		void SetBindings(std::vector<Binding> bindings);

		/**
		 * @brief Compare two classes for equality.
		 * @param other The class to compare against.
		 * @return True if both describe the same wrapper in the same way.
		 */
		[[nodiscard]] bool operator==(const Class& other) const noexcept;

		/**
		 * @brief Order two classes.
		 * @param other The class to compare against.
		 * @return The relative ordering of the two classes.
		 */
		[[nodiscard]] auto operator<=>(const Class& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
