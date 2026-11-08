#pragma once

#include <memory>
#include <string>
#include <optional>

#include "plugify/global.h"
#include "plugify/signarure.hpp"
#include "plg/inplace_vector.hpp"

namespace plugify {
	class Alias;

	/**
	 * @class Binding
	 * @brief An instance method of a class, wrapping one of the methods an extension exports.
	 *
	 * A binding does not declare a signature of its own. It points at a method the
	 * extension already exports and says how that method appears on instances of
	 * the class: under which name, whether the instance handle is passed as the
	 * first argument, and under which alternative parameter and return names.
	 *
	 * The class is a PIMPL wrapper over the parsed manifest entry.
	 *
	 * @see Class, Method, Alias
	 */
	class PLUGIFY_API Binding {
	public:
		/**
		 * @brief Construct an empty binding.
		 */
		Binding();

		/**
		 * @brief Destructor.
		 */
		~Binding();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Binding.
		 */
		Binding(const Binding& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Binding.
		 */
		Binding(Binding&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Binding.
		 * @return A reference to this instance after copying.
		 */
		Binding& operator=(const Binding& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Binding.
		 * @return A reference to this instance after moving.
		 */
		Binding& operator=(Binding&& other) noexcept;

		// Getters

		/**
		 * @brief Get the name this method has on class instances.
		 * @return The name generated bindings expose on the class.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Get the extension method this binding wraps.
		 * @return The public API name of an entry in the method list of the extension.
		 */
		[[nodiscard]] const std::string& GetMethod() const noexcept;

		/**
		 * @brief Check whether the instance handle is bound as the first argument.
		 * @return True if the handle is passed automatically and the first parameter is dropped
		 * from the exposed signature, false if the method is called as a static one.
		 */
		[[nodiscard]] bool IsBindSelf() const noexcept;

		/**
		 * @brief Get the alternative names of the parameters of the wrapped method.
		 * @return One optional alias per parameter, in signature order; an empty entry leaves the
		 * name of that parameter in place.
		 */
		[[nodiscard]] const std::inplace_vector<std::optional<Alias>, Signature::kMaxFuncArgs>& GetParamAliases() const noexcept;

		/**
		 * @brief Get the alternative name of the return value of the wrapped method.
		 * @return The alias for the return value, or an empty optional if there is none.
		 */
		[[nodiscard]] const std::optional<Alias>& GetRetAlias() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the name this method has on class instances.
		 * @param name The name to expose on the class.
		 */
		void SetName(std::string name);

		/**
		 * @brief Set the extension method this binding wraps.
		 * @param method The public API name of the method to bind.
		 */
		void SetMethod(std::string method);

		/**
		 * @brief Set whether the instance handle is bound as the first argument.
		 * @param bindSelf True to pass the handle automatically.
		 */
		void SetBindSelf(bool bindSelf);

		/**
		 * @brief Set the alternative names of the parameters of the wrapped method.
		 * @param paramAliases One optional alias per parameter, in signature order.
		 */
		void SetParamAliases(std::inplace_vector<std::optional<Alias>, Signature::kMaxFuncArgs> paramAliases);

		/**
		 * @brief Set the alternative name of the return value of the wrapped method.
		 * @param retAlias The alias for the return value.
		 */
		void SetRetAlias(std::optional<Alias> retAlias);

		/**
		 * @brief Compare two bindings for equality.
		 * @param other The binding to compare against.
		 * @return True if both bind the same method in the same way.
		 */
		[[nodiscard]] bool operator==(const Binding& other) const noexcept;

		/**
		 * @brief Order two bindings.
		 * @param other The binding to compare against.
		 * @return The relative ordering of the two bindings.
		 */
		[[nodiscard]] auto operator<=>(const Binding& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
