#pragma once

#include <span>
#include <optional>
#include <string>
#include <plugify/reference_wrapper.h>
#include <plugify/value_type.h>
#include <plugify_export.h>

namespace plugify {
	struct Method;
	struct Property;
	class MethodRef;

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
		[[nodiscard]] ValueType GetType() const noexcept;

		/**
		 * @brief Checks if the property is a reference.
		 *
		 * @return `true` if the property is a reference, otherwise `false`.
		 */
		[[nodiscard]] bool IsReference() const noexcept;

		/**
		 * @brief Retrieves the prototype method for the property.
		 *
		 * @return An optional `MethodRef` representing the prototype method.
		 *         Returns an empty optional if no prototype exists.
		 */
		[[nodiscard]] std::optional<MethodRef> GetPrototype() const noexcept;
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
		[[nodiscard]] std::string_view GetName() const noexcept;

		/**
		 * @brief Retrieves the function name of the method.
		 *
		 * @return A string view representing the function name of the method.
		 */
		[[nodiscard]] std::string_view GetFunctionName() const noexcept;

		/**
		 * @brief Retrieves the calling convention of the method.
		 *
		 * @return A string view representing the calling convention.
		 */
		[[nodiscard]] std::string_view GetCallingConvention() const noexcept;

		/**
		 * @brief Retrieves the parameter types for the method.
		 *
		 * @return A span of `PropertyRef` objects representing the parameter types.
		 */
		[[nodiscard]] std::span<const PropertyRef> GetParamTypes() const noexcept;

		/**
		 * @brief Retrieves the return type of the method.
		 *
		 * @return A `PropertyRef` representing the return type of the method.
		 */
		[[nodiscard]] PropertyRef GetReturnType() const noexcept;

		/**
		 * @brief Retrieves the variable index for the method.
		 *
		 * @return An 8-bit unsigned integer representing the variable index.
		 */
		[[nodiscard]] uint8_t GetVarIndex() const noexcept;
	};
	static_assert(is_ref_v<MethodRef>);
} // namespace plugify