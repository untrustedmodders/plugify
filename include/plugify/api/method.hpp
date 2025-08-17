#pragma once

#include <span>
#include <string>

#include <plugify/api/handle.hpp>
#include <plugify/api/value_type.hpp>
#include <plugify/asm/mem_addr.hpp>

#include <plugify_export.h>

namespace plugify {
	struct EnumValue;
	struct Enum;
	struct Method;
	struct Property;
	class MethodHandle;

	/**
	 * @class EnumValueHandle
	 * @brief A handle class for an `EnumValue` structure.
	 */
	class PLUGIFY_API EnumValueHandle : public Handle<const EnumValue> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the enum value name.
		 *
		 * @return A string view representing the name of the enum value.
		 */
		const std::string& GetName() const noexcept;

		/**
		 * @brief Retrieves the enum value.
		 *
		 * @return An integer representing the value of the enum element.
		 */
		int64_t GetValue() const noexcept;
	};

	/**
	 * @class EnumHandle
	 * @brief A handle class for the `Enum` structure.
	 */
	class PLUGIFY_API EnumHandle : public Handle<const Enum> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the enum name.
		 *
		 * @return A string view representing the name of the enum.
		 */
		const std::string& GetName() const noexcept;

		/**
		 * @brief Retrieves the values contained within the enum.
		 *
		 * @return A span of `EnumValueHandle` objects representing the values in the enum.
		 */
		std::span<const EnumValueHandle> GetValues() const noexcept;
	};

	/**
	 * @class PropertyHandle
	 * @brief A handle class for the `Property` structure.
	 */
	class PLUGIFY_API PropertyHandle : public Handle<const Property> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the value type of the property.
		 *
		 * @return The value type of the property.
		 */
		ValueType GetType() const noexcept;

		/**
		 * @brief Checks if the property is a reference.
		 *
		 * @return `true` if the property is a reference, otherwise `false`.
		 */
		bool IsReference() const noexcept;

		/**
		 * @brief Retrieves the prototype method for the property.
		 *
		 * @return A `MethodHandle` representing the prototype method.
		 *  Returns an empty handle if no prototype exists.
		 */
		MethodHandle GetPrototype() const noexcept;

		/**
		 * @brief Retrieves the enum information for the property.
		 *
		 * @return A `EnumHandle` representing the enum information.
		 *  Returns an empty handle if no enum exists.
		 */
		EnumHandle GetEnum() const noexcept;
	};

	/**
	 * @class MethodHandle
	 * @brief A handle class for the `Method` structure.
	 */
	class PLUGIFY_API MethodHandle : public Handle<const Method> {
		using Handle::Handle;
	public:
		/**
		 * @brief Retrieves the method name.
		 *
		 * @return A string view representing the name of the method.
		 */
		const std::string& GetName() const noexcept;

		/**
		 * @brief Retrieves the function name of the method.
		 *
		 * @return A string view representing the function name of the method.
		 */
		const std::string& GetFunctionName() const noexcept;

		/**
		 * @brief Retrieves the calling convention of the method.
		 *
		 * @return A string view representing the calling convention.
		 */
		const std::string& GetCallingConvention() const noexcept;

		/**
		 * @brief Retrieves the parameter types for the method.
		 *
		 * @return A span of `PropertyHandle` objects representing the parameter types.
		 */
		std::span<const PropertyHandle> GetParamTypes() const noexcept;

		/**
		 * @brief Retrieves the return type of the method.
		 *
		 * @return A `PropertyHandle` representing the return type of the method.
		 */
		PropertyHandle GetReturnType() const noexcept;

		/**
		 * @brief Retrieves the variable index for the method.
		 *
		 * @return An 8-bit unsigned integer representing the variable index.
		 */
		uint8_t GetVarIndex() const noexcept;

		/**
		 * @brief Attempts to find a prototype method by its name in the current method's parameters or return type.
		 *
		 * This method searches through the parameter types (`paramTypes`) and return type (`retType`) of the current method
		 * to find a method prototype with the given name. The search is recursive, meaning it will also look into
		 * the prototypes of parameter and return types if they themselves have prototypes.
		 *
		 * @param name The name of the prototype method to search for.
		 *
		 * @return MethodHandle
		 * - A `MethodHandle` containing the found prototype if a method with the specified name exists, or
		 * - An empty `MethodHandle` if no matching prototype is found.
		 *
		 * Example usage:
		 * @code
		 * MethodHandle prototype = method.FindPrototype("targetMethodName");
		 * if (prototype) {
		 *     // Process the found prototype
		 * }
		 * @endcode
		 */
		MethodHandle FindPrototype(std::string_view name) const noexcept;
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
		MethodHandle method; ///< Handle representing the method.
		MemAddr addr; ///< Pointer to the method's memory address.
	};

	/**
	 * @struct MethodTable
	 * @brief Represents a table of method availability flags.
	 *
	 * This structure contains offsets that indicate the presence of certain methods
	 * within a plugin. Each field represents whether a corresponding method is available.
	 */
	struct MethodTable {
		bool hasUpdate{}; ///< Boolean indicating if an update method exists.
		bool hasStart{}; ///< Boolean indicating if a start method exists.
		bool hasEnd{}; ///< Boolean indicating if an end method exists.
		bool hasExport{}; ///< Boolean indicating if a export methods exists.
	};

} // namespace plugify
