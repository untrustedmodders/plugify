#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "plugify/global.h"
#include "plugify/mem_addr.hpp"
#include "plugify/signarure.hpp"
#include "plugify/value_type.hpp"

namespace plugify {
	class Property;

	// Method Class
	class PLUGIFY_API Method {
	public:
		Method();
		~Method();
		Method(const Method& other);
		Method(Method&& other) noexcept;
		Method& operator=(const Method& other);
		Method& operator=(Method&& other) noexcept;

		// Getters
		[[nodiscard]] const std::vector<Property>& GetParamTypes() const noexcept;
		[[nodiscard]] const Property& GetRetType() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] const std::string& GetFuncName() const noexcept;
		[[nodiscard]] CallConv GetCallConv() const noexcept;
		[[nodiscard]] uint8_t GetVarIndex() const noexcept;

		// Setters (pass by value and move)
		void SetParamTypes(std::vector<Property> paramTypes);
		void SetRetType(Property retType);
		void SetName(std::string name);
		void SetFuncName(std::string funcName);
		void SetCallConv(CallConv callConv);
		void SetVarIndex(uint8_t varIndex);

		[[nodiscard]] bool operator==(const Method& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Method& other) const noexcept;

		[[nodiscard]] const Method* FindPrototype(std::string_view name) const noexcept;

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

	/**
	 * @struct MethodData
	 * @brief Represents data related to a plugin method.
	 *
	 * This structure holds information about a specific plugin method, including:
	 * - A method handle to identify the method.
	 * - A memory address pointer to the method's address.
	 */
	struct MethodData {
		const Method& method;  ///< Ref to representing the method.
		MemAddr addr;          ///< Pointer to the method's memory address.
	};

	/**
	 * @struct MethodTable
	 * @brief Represents a table of method availability flags.
	 *
	 * This structure contains offsets that indicate the presence of certain methods
	 * within a plugin. Each field represents whether a corresponding method is available.
	 */
	struct MethodTable {
		bool hasUpdate{};  ///< Boolean indicating if an update method exists.
		bool hasStart{};   ///< Boolean indicating if a start method exists.
		bool hasEnd{};     ///< Boolean indicating if an end method exists.
		bool hasExport{};  ///< Boolean indicating if a export methods exists.
	};
}
