#pragma once

#include <optional>
#include <plugify/reference_wrapper.hpp>
#include <plugify/value_type.hpp>
#include <plugify_export.h>
#include <span>
#include <string>

namespace plugify {
	struct EnumValue;
	struct Enum;
	struct Method;
	struct Property;
	class MethodRef;

	/**
	 * @class EnumValueRef
	 * @brief A reference class for an `EnumValue` structure.
	 *
	 * This class holds a reference to a specific value within an `Enum` object, allowing users
	 * to retrieve information such as the name and value of the enum element.
	 */
	class PLUGIFY_API EnumValueRef : public Ref<const EnumValue> {
		using Ref::Ref; ///< Inherit constructors from Ref<const EnumValue>.
	public:
		/**
		 * @brief Retrieves the enum value name.
		 *
		 * @return A string view representing the name of the enum value.
		 */
		std::string_view GetName() const noexcept;

		/**
		 * @brief Retrieves the enum value.
		 *
		 * @return An integer representing the value of the enum element.
		 */
		int64_t GetValue() const noexcept;
	};
	static_assert(is_ref_v<EnumValueRef>);

	/**
	 * @class EnumRef
	 * @brief A reference class for the `Enum` structure.
	 *
	 * This class holds a reference to an `Enum` object, providing access to its name and values.
	 */
	class PLUGIFY_API EnumRef : public Ref<const Enum> {
		using Ref::Ref; ///< Inherit constructors from Ref<const Enum>.
	public:
		/**
		 * @brief Retrieves the enum name.
		 *
		 * @return A string view representing the name of the enum.
		 */
		std::string_view GetName() const noexcept;

		/**
		 * @brief Retrieves the values contained within the enum.
		 *
		 * @return A span of `EnumValueRef` objects representing the values in the enum.
		 */
		std::span<const EnumValueRef> GetValues() const noexcept;
	};
	static_assert(is_ref_v<EnumRef>);

	/**
	 * @class PropertyRef
	 * @brief A reference class for the `Property` structure.
	 *
	 * This class holds a reference to a `Property` object, allowing users to retrieve
	 * information such as its type, whether it's a reference, and its prototype method if applicable.
	 */
	class PLUGIFY_API PropertyRef : public Ref<const Property> {
		using Ref::Ref; ///< Inherit constructors from Ref<const Property>.
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
		 * @return An optional `MethodRef` representing the prototype method.
		 *         Returns an empty optional if no prototype exists.
		 */
		std::optional<MethodRef> GetPrototype() const noexcept;

		/**
		 * @brief Retrieves the enum information for the property.
		 *
		 * @return An optional `EnumRef` representing the enum information.
		 *         Returns an empty optional if no enum exists.
		 */
		std::optional<EnumRef> GetEnum() const noexcept;
	};
	static_assert(is_ref_v<PropertyRef>);

	/**
	 * @class MethodRef
	 * @brief A reference class for the `Method` structure.
	 *
	 * This class holds a reference to a `Method` object, allowing users to retrieve
	 * information such as its name, function name, calling convention, parameter types,
	 * return type, and variable index.
	 */
	class PLUGIFY_API MethodRef : public Ref<const Method> {
		using Ref::Ref; ///< Inherit constructors from Ref<const Method>.
	public:
		/**
		 * @brief Retrieves the method name.
		 *
		 * @return A string view representing the name of the method.
		 */
		std::string_view GetName() const noexcept;

		/**
		 * @brief Retrieves the function name of the method.
		 *
		 * @return A string view representing the function name of the method.
		 */
		std::string_view GetFunctionName() const noexcept;

		/**
		 * @brief Retrieves the calling convention of the method.
		 *
		 * @return A string view representing the calling convention.
		 */
		std::string_view GetCallingConvention() const noexcept;

		/**
		 * @brief Retrieves the parameter types for the method.
		 *
		 * @return A span of `PropertyRef` objects representing the parameter types.
		 */
		std::span<const PropertyRef> GetParamTypes() const noexcept;

		/**
		 * @brief Retrieves the return type of the method.
		 *
		 * @return A `PropertyRef` representing the return type of the method.
		 */
		PropertyRef GetReturnType() const noexcept;

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
		 * @return std::optional<MethodRef>
		 * - A `std::optional<MethodRef>` containing the found prototype if a method with the specified name exists, or
		 * - An empty `std::optional` if no matching prototype is found.
		 *
		 * Example usage:
		 * @code
		 * std::optional<MethodRef> prototype = methodRef.FindPrototype("targetMethodName");
		 * if (prototype) {
		 *     // Process the found prototype
		 * }
		 * @endcode
		 */
		std::optional<MethodRef> FindPrototype(std::string_view name) const noexcept;
	};
	static_assert(is_ref_v<MethodRef>);


} // namespace plugify
