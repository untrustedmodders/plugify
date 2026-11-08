#pragma once

#include <memory>
#include <string>

#include "plugify/global.h"
#include "plugify/address.hpp"
#include "plugify/signarure.hpp"
#include "plugify/value_type.hpp"
#include "plg/inplace_vector.hpp"

namespace plugify {
	class Property;

	/**
	 * @class Method
	 * @brief A callable signature declared in a manifest.
	 *
	 * A method describes one function an extension exports: the name other
	 * extensions call it by, the symbol it resolves to, its parameters, its return
	 * value and its calling convention. The same type also describes a callback
	 * signature, in which case it is referred to as a @ref Prototype.
	 *
	 * The class is a PIMPL wrapper over the parsed manifest entry.
	 *
	 * @see Property, MethodData
	 */
	class PLUGIFY_API Method {
	public:
		/**
		 * @brief Construct an empty method.
		 */
		Method();

		/**
		 * @brief Destructor.
		 */
		~Method();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Method.
		 */
		Method(const Method& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Method.
		 */
		Method(Method&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Method.
		 * @return A reference to this instance after copying.
		 */
		Method& operator=(const Method& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Method.
		 * @return A reference to this instance after moving.
		 */
		Method& operator=(Method&& other) noexcept;

		// Getters

		/**
		 * @brief Get the parameters of this method.
		 * @return The parameters in signature order.
		 */
		[[nodiscard]] const std::inplace_vector<Property, Signature::kMaxFuncArgs>& GetParamTypes() const noexcept;

		/**
		 * @brief Get the return value of this method.
		 * @return The return property, of type @ref ValueType::Void when the method returns nothing.
		 */
		[[nodiscard]] const Property& GetRetType() const noexcept;

		/**
		 * @brief Get the public API name of this method.
		 * @return The name other extensions use to call this function.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Get the implementation name of this method.
		 * @return The function name as implemented in the extension's source code.
		 * @note This is what the language module resolves, so it has to match the symbol exactly.
		 */
		[[nodiscard]] const std::string& GetFuncName() const noexcept;

		/**
		 * @brief Get the calling convention of the exported function.
		 * @return The calling convention, @ref CallConv::CDecl unless the manifest overrides it.
		 * @note Language modules that do not generate native call stubs ignore this.
		 */
		[[nodiscard]] CallConv GetCallConv() const noexcept;

		/**
		 * @brief Get the index of the first variadic parameter.
		 * @return Zero-based index into the parameter list, or @ref Signature::kNoVarArgs for a
		 * function that is not variadic.
		 */
		[[nodiscard]] uint8_t GetVarIndex() const noexcept;

		// Setters (pass by value and move)

		/**
		 * @brief Set the parameters of this method.
		 * @param paramTypes The parameters in signature order.
		 */
		void SetParamTypes(std::inplace_vector<Property, Signature::kMaxFuncArgs> paramTypes);

		/**
		 * @brief Set the return value of this method.
		 * @param retType The return property.
		 */
		void SetRetType(Property retType);

		/**
		 * @brief Set the public API name of this method.
		 * @param name The name other extensions call this function by.
		 */
		void SetName(std::string name);

		/**
		 * @brief Set the implementation name of this method.
		 * @param funcName The function name as implemented in the extension's source code.
		 */
		void SetFuncName(std::string funcName);

		/**
		 * @brief Set the calling convention of the exported function.
		 * @param callConv The calling convention.
		 */
		void SetCallConv(CallConv callConv);

		/**
		 * @brief Set the index of the first variadic parameter.
		 * @param varIndex Zero-based index into the parameter list, or @ref Signature::kNoVarArgs.
		 */
		void SetVarIndex(uint8_t varIndex);

		/**
		 * @brief Compare two methods for equality.
		 * @param other The method to compare against.
		 * @return True if both describe the same signature in the same way.
		 */
		[[nodiscard]] bool operator==(const Method& other) const noexcept;

		/**
		 * @brief Order two methods.
		 * @param other The method to compare against.
		 * @return The relative ordering of the two methods.
		 */
		[[nodiscard]] auto operator<=>(const Method& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

	/**
	 * @struct MethodData
	 * @brief Represents data related to a plugin method.
	 *
	 * This structure holds information about a specific plugin method, including:
	 * - A method reference to identify the method.
	 * - A memory address pointer to the method's address.
	 */
	struct MethodData {
		const Method& method;  ///< Ref to representing the method.
		Address addr;          ///< Pointer to the method's memory address.
	};

	/**
	 * @struct MethodTable
	 * @brief Which optional callbacks exist, so they are not called for nothing.
	 *
	 * The same structure is returned from two places, and each field is only read
	 * from one of them:
	 * - in InitData, from ILanguageModule::Initialize, only @ref hasUpdate is
	 *   read, for the module's own OnUpdate. The other three are ignored.
	 * - in LoadData, from ILanguageModule::OnPluginLoad, @ref hasUpdate,
	 *   @ref hasStart and @ref hasEnd are that plugin's OnPluginUpdate,
	 *   OnPluginStart and OnPluginEnd, and @ref hasExport says the plugin has
	 *   methods to export, which is what decides whether the module's
	 *   OnMethodExport is called for it.
	 *
	 * So @ref hasExport describes a plugin, not the module whose OnMethodExport it
	 * reaches, and a module leaves everything but @ref hasUpdate alone.
	 */
	struct MethodTable {
		bool hasUpdate{};  ///< OnUpdate (in InitData) or OnPluginUpdate (in LoadData).
		bool hasStart{};   ///< OnPluginStart exists. LoadData only.
		bool hasEnd{};     ///< OnPluginEnd exists. LoadData only.
		bool hasExport{};  ///< The plugin has methods to export. LoadData only.
	};

	/**
	 * @typedef Prototype
	 * @brief A callable signature used as a callback type.
	 *
	 * A prototype is the same thing as a @ref Method, under the name the manifest
	 * uses when the signature describes a callback a parameter takes or a return
	 * value hands back, rather than a function the extension exports.
	 */
	using Prototype = Method;
}
