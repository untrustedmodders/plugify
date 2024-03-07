#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace plugify {
	/**
	 * @enum ValueType
	 * @brief Enumerates possible types of values in the reflection system.
	 */
	enum class ValueType : std::uint8_t {
		Invalid,

		// C types
		Void,
		Bool,
		Char8,
		Char16,
		Int8,
		Int16,
		Int32,
		Int64,
		UInt8,
		UInt16,
		UInt32,
		UInt64,
		Ptr64,
		Float,
		Double,
		Function,

		// std::string
		String,

		// std::vector
		ArrayBool,
		ArrayChar8,
		ArrayChar16,
		ArrayInt8,
		ArrayInt16,
		ArrayInt32,
		ArrayInt64,
		ArrayUInt8,
		ArrayUInt16,
		ArrayUInt32,
		ArrayUInt64,
		ArrayPtr64,
		ArrayFloat,
		ArrayDouble,
		ArrayString,

		// glm:vec
		Vector2,
		Vector3,
		Vector4,

		// glm:mat
		//Mat2x2,
		//Mat2x3,
		//Mat2x4,
		Matrix3x2,
		//Mat3x3,
		//Mat3x4,
		//Mat4x2,
		//Mat4x3,
		Matrix4x4,

		LastPrimitive = Function
	};

	struct Method;

	/**
	 * @struct Property
	 * @brief Describes the properties of a value in the reflection system.
	 *
	 * The Property structure holds information about the type of a value,
	 * whether it's a reference, and for function types, a prototype of the method.
	 */
	struct Property final {
		ValueType type{}; ///< The type of the value.
		bool ref{ false }; ///< Indicates whether the value is a reference.
		std::shared_ptr<Method> prototype; ///< Prototype of the method for function types.
	};

	/**
	 * @struct Method
	 * @brief Describes a method in the reflection system.
	 *
	 * The Method structure holds information about the name, function name,
	 * calling convention, parameter types, return type, and variable index.
	 */
	struct Method final {
		std::string name; ///< The name of the method.
		std::string funcName; ///< The function name of the method.
		std::string callConv; ///< The calling convention of the method.
		std::vector<Property> paramTypes; ///< The parameter types of the method.
		Property retType; ///< The return type of the method.
		std::uint8_t varIndex{ kNoVarArgs }; ///< The variable index (0xFFu if no variable arguments).

		static inline const std::uint8_t kNoVarArgs = 0xFFU; ///< Constant for indicating no variable arguments.

		/**
		 * @brief Overloaded equality operator for comparing Method instances.
		 * @param rhs The right-hand side Method for comparison.
		 * @return True if the names of this instance and rhs are equal.
		 */
		[[nodiscard]] bool operator==(const Method& rhs) const { return name == rhs.name; }

		/**
		 * @brief Checks if the return type and all parameter types are primitive.
		 *
		 * This method examines the return type and parameter types to determine
		 * whether they are primitive types. It returns true if both the return type
		 * and all parameter types are primitive, and false otherwise.
		 *
		 * @return True if the return type and all parameter types are primitive, false otherwise.
		 */
		[[nodiscard]] bool IsPrimitive() const {
			if (retType.type >= ValueType::LastPrimitive)
				return false;

			for (const auto& param : paramTypes) {
				if (param.type >= ValueType::LastPrimitive)
					return false;
			}

			return true;
		}
	};

	/**
	 * @brief Convert a ValueType enum value to its string representation.
	 * @param value The ValueType value to convert.
	 * @return The string representation of the ValueType.
	 */
	[[maybe_unused]] constexpr std::string_view ValueTypeToString(ValueType value) {
		switch (value) {
			case ValueType::Void:          return "void";
			case ValueType::Bool:          return "bool";
			case ValueType::Char8:         return "char8";
			case ValueType::Char16:        return "char16";
			case ValueType::Int8:          return "int8";
			case ValueType::Int16:         return "int16";
			case ValueType::Int32:         return "int32";
			case ValueType::Int64:         return "int64";
			case ValueType::UInt8:         return "uint8";
			case ValueType::UInt16:        return "uint16";
			case ValueType::UInt32:        return "uint32";
			case ValueType::UInt64:        return "uint64";
			case ValueType::Ptr64:         return "ptr64";
			case ValueType::Float:         return "float";
			case ValueType::Double:        return "double";
			case ValueType::Function:      return "function";
			case ValueType::String:        return "string";
			case ValueType::ArrayBool:     return "bool*";
			case ValueType::ArrayChar8:    return "char8*";
			case ValueType::ArrayChar16:   return "char16*";
			case ValueType::ArrayInt8:     return "int8*";
			case ValueType::ArrayInt16:    return "int16*";
			case ValueType::ArrayInt32:    return "int32*";
			case ValueType::ArrayInt64:    return "int64*";
			case ValueType::ArrayUInt8:    return "uint8*";
			case ValueType::ArrayUInt16:   return "uint16*";
			case ValueType::ArrayUInt32:   return "uint32*";
			case ValueType::ArrayUInt64:   return "uint64*";
			case ValueType::ArrayPtr64:    return "ptr64*";
			case ValueType::ArrayFloat:    return "float*";
			case ValueType::ArrayDouble:   return "double*";
			case ValueType::ArrayString:   return "string*";
			case ValueType::Vector2:       return "vec2";
			case ValueType::Vector3:       return "vec3";
			case ValueType::Vector4:       return "vec4";
			case ValueType::Matrix3x2:     return "mat3x2";
			case ValueType::Matrix4x4:     return "mat4x4";
			default:                       return "unknown";
		}
	}

	/**
	 * @brief Convert a string representation to a ValueType enum value.
	 * @param value The string representation of ValueType.
	 * @return The corresponding ValueType enum value.
	 */
	[[maybe_unused]] constexpr ValueType ValueTypeFromString(std::string_view value) {
		if (value == "void") {
			return ValueType::Void;
		} else if (value == "bool") {
			return ValueType::Bool;
		} else if (value == "char8") {
			return ValueType::Char8;
		} else if (value == "char16") {
			return ValueType::Char16;
		} else if (value == "int8") {
			return ValueType::Int8;
		} else if (value == "int16") {
			return ValueType::Int16;
		} else if (value == "int32") {
			return ValueType::Int32;
		} else if (value == "int64") {
			return ValueType::Int64;
		} else if (value == "uint8") {
			return ValueType::UInt8;
		} else if (value == "uint16") {
			return ValueType::UInt16;
		} else if (value == "uint32") {
			return ValueType::UInt32;
		} else if (value == "uint64") {
			return ValueType::UInt64;
		} else if (value == "ptr64") {
			return ValueType::Ptr64;
		} else if (value == "float") {
			return ValueType::Float;
		} else if (value == "double") {
			return ValueType::Double;
		} else if (value == "function") {
			return ValueType::Function;
		} else if (value == "string") {
			return ValueType::String;
		} else if (value == "bool*") {
			return ValueType::ArrayBool;
		} else if (value == "char8*") {
			return ValueType::ArrayChar8;
		} else if (value == "char16*") {
			return ValueType::ArrayChar16;
		} else if (value == "int8*") {
			return ValueType::ArrayInt8;
		} else if (value == "int16*") {
			return ValueType::ArrayInt16;
		} else if (value == "int32*") {
			return ValueType::ArrayInt32;
		} else if (value == "int64*") {
			return ValueType::ArrayInt64;
		} else if (value == "uint8*") {
			return ValueType::ArrayUInt8;
		} else if (value == "uint16*") {
			return ValueType::ArrayUInt16;
		} else if (value == "uint32*") {
			return ValueType::ArrayUInt32;
		} else if (value == "uint64*") {
			return ValueType::ArrayUInt64;
		} else if (value == "ptr64*") {
			return ValueType::ArrayPtr64;
		} else if (value == "float*") {
			return ValueType::ArrayFloat;
		} else if (value == "double*") {
			return ValueType::ArrayDouble;
		} else if (value == "string*") {
			return ValueType::ArrayString;
		} else if (value == "vec2") {
			return ValueType::Vector2;
		} else if (value == "vec3") {
			return ValueType::Vector3;
		} else if (value == "vec4") {
			return ValueType::Vector4;
		} else if (value == "mat3x2") {
			return ValueType::Matrix3x2;
		} else if (value == "mat4x4") {
			return ValueType::Matrix4x4;
		}
		return ValueType::Invalid;
	}
} // namespace plugify