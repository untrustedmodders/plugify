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

	class PLUGIFY_API Method {
	public:
		Method();
		~Method();
		Method(const Method& other);
		Method(Method&& other) noexcept;
		Method& operator=(const Method& other);
		Method& operator=(Method&& other) noexcept;

		// Getters
		[[nodiscard]] const std::inplace_vector<Property, Signature::kMaxFuncArgs>& GetParamTypes() const noexcept;
		[[nodiscard]] const Property& GetRetType() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] const std::string& GetFuncName() const noexcept;
		[[nodiscard]] CallConv GetCallConv() const noexcept;
		[[nodiscard]] uint8_t GetVarIndex() const noexcept;

		// Setters (pass by value and move)
		void SetParamTypes(std::inplace_vector<Property, Signature::kMaxFuncArgs> paramTypes);
		void SetRetType(Property retType);
		void SetName(std::string name);
		void SetFuncName(std::string funcName);
		void SetCallConv(CallConv callConv);
		void SetVarIndex(uint8_t varIndex);

		[[nodiscard]] bool operator==(const Method& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Method& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;
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

	using Prototype = Method;
}
