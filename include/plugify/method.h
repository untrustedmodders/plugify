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
		Pointer,
		Float,
		Double,

		Function,

		// Objects
		String,

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
		ArrayPointer,
		ArrayFloat,
		ArrayDouble,
		ArrayString,

		// Structs
		Vector2,
		Vector3,
		Vector4,

		Matrix4x4,
		//Matrix2x2,
		//Matrix2x3,
		//Matrix2x4,
		//Matrix3x2,
		//Matrix3x3,
		//Matrix3x4,
		//Matrix4x2,
		//Matrix4x3,

		//! Helpers

		_BaseStart = Void,
		_BaseEnd = Function,

		_FloatStart = Float,
		_FloatEnd = Double,

		_ObjectStart = String,
		_ObjectEnd = ArrayString,

		_ArrayStart = ArrayBool,
		_ArrayEnd = ArrayString,

		_StructStart = Vector2,
		_StructEnd = Matrix4x4,

		// First struct which return as 1st parameter
#if _WIN32
		_HiddenParamStart = Vector3,
#else
		_HiddenParamStart = Matrix4x4,
#endif
		_LastAssigned = Matrix4x4,
	};

	/**
	 * @brief Namespace containing string representations of ValueType enum values.
	 */
	namespace ValueName {
		static constexpr std::string_view Void = "void";
		static constexpr std::string_view Bool = "bool";
		static constexpr std::string_view Char8 = "char8";
		static constexpr std::string_view Char16 = "char16";
		static constexpr std::string_view Int8 = "int8";
		static constexpr std::string_view Int16 = "int16";
		static constexpr std::string_view Int32 = "int32";
		static constexpr std::string_view Int64 = "int64";
		static constexpr std::string_view UInt8 = "uint8";
		static constexpr std::string_view UInt16 = "uint16";
		static constexpr std::string_view UInt32 = "uint32";
		static constexpr std::string_view UInt64 = "uint64";
		static constexpr std::string_view Float = "float";
		static constexpr std::string_view Double = "double";
		static constexpr std::string_view Function = "function";
		static constexpr std::string_view String = "string";
		static constexpr std::string_view ArrayBool = "bool*";
		static constexpr std::string_view ArrayChar8 = "char8*";
		static constexpr std::string_view ArrayChar16 = "char16*";
		static constexpr std::string_view ArrayInt8 = "int8*";
		static constexpr std::string_view ArrayInt16 = "int16*";
		static constexpr std::string_view ArrayInt32 = "int32*";
		static constexpr std::string_view ArrayInt64 = "int64*";
		static constexpr std::string_view ArrayUInt8 = "uint8*";
		static constexpr std::string_view ArrayUInt16 = "uint16*";
		static constexpr std::string_view ArrayUInt32 = "uint32*";
		static constexpr std::string_view ArrayUInt64 = "uint64*";
		static constexpr std::string_view ArrayFloat = "float*";
		static constexpr std::string_view ArrayDouble = "double*";
		static constexpr std::string_view ArrayString = "string*";
		static constexpr std::string_view Vec2 = "vec2";
		static constexpr std::string_view Vec3 = "vec3";
		static constexpr std::string_view Vec4 = "vec4";
		static constexpr std::string_view Mat4x4 = "mat4x4";
		static constexpr std::string_view Invalid = "invalid";
#if INTPTR_MAX == INT32_MAX
		static constexpr std::string_view Pointer = "ptr32";
		static constexpr std::string_view ArrayPointer = "ptr32*";
#elif INTPTR_MAX == INT64_MAX
		static constexpr std::string_view Pointer = "ptr64";
		static constexpr std::string_view ArrayPointer = "ptr64*";
#else
		#error "Environment not 32 or 64-bit."
#endif
	}

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
	};

	/**
	 * @brief Namespace containing utility functions of ValueType enum.
	 */
	namespace TypeUtils {
		template<typename T>
		[[maybe_unused]] constexpr bool IsBetween(T x, T a, T b) {
			return x >= a && x <= b;
		}

		//! Tests whether a given type `type` is \ref ValueType::Void.
		[[maybe_unused]] constexpr bool IsVoid(ValueType type)     { return type == ValueType::Void; }
		//! Tests whether a given type `type` is a valid non-void type.
		[[maybe_unused]] constexpr bool IsValid(ValueType type)    { return IsBetween(type, ValueType::Void, ValueType::_LastAssigned); }
		//! Tests whether a given type `type` is scalar (has no vector part).
		[[maybe_unused]] constexpr bool IsScalar(ValueType type)   { return IsBetween(type, ValueType::_BaseStart, ValueType::_BaseEnd); }
		//! Tests whether a given type is a scalar floating point of any size.
		[[maybe_unused]] constexpr bool IsFloating(ValueType type) { return IsBetween(type, ValueType::_FloatStart, ValueType::_FloatEnd); }
		
		//! Tests whether a given type is a 1-bit boolean.
		[[maybe_unused]] constexpr bool IsBool(ValueType type)    { return type == ValueType::Bool; }
		//! Tests whether a given type is a 8-bit character.
		[[maybe_unused]] constexpr bool IsChat8(ValueType type)   { return type == ValueType::Char8; }
		//! Tests whether a given type is a 16-bit character.
		[[maybe_unused]] constexpr bool IsChar16(ValueType type)  { return type == ValueType::Char16; }
		//! Tests whether a given type is a 8-bit integer.
		[[maybe_unused]] constexpr bool IsInt8(ValueType type)    { return type == ValueType::Int8; }
		//! Tests whether a given type is a 8-bit unsigned integer.
		[[maybe_unused]] constexpr bool IsUInt8(ValueType type)   { return type == ValueType::UInt8; }
		//! Tests whether a given type is a 16-bit integer.
		[[maybe_unused]] constexpr bool IsInt16(ValueType type)   { return type == ValueType::Int16; }
		//! Tests whether a given type is a 16-bit unsigned integer.
		[[maybe_unused]] constexpr bool IsUInt16(ValueType type)  { return type == ValueType::UInt16; }
		//! Tests whether a given type is a 32-bit integer.
		[[maybe_unused]] constexpr bool IsInt32(ValueType type)   { return type == ValueType::Int32; }
		//! Tests whether a given type is a 32-bit unsigned integer.
		[[maybe_unused]] constexpr bool IsUInt32(ValueType type)  { return type == ValueType::UInt32; }
		//! Tests whether a given type is a 64-bit integer.
		[[maybe_unused]] constexpr bool IsInt64(ValueType type)   { return type == ValueType::Int64; }
		//! Tests whether a given type is a 64-bit unsigned integer.
		[[maybe_unused]] constexpr bool IsUInt64(ValueType type)  { return type == ValueType::UInt64; }
		//! Tests whether a given type is a pointer.
		[[maybe_unused]] constexpr bool IsPointer(ValueType type) { return type == ValueType::Pointer; }
		//! Tests whether a given type is a float.
		[[maybe_unused]] constexpr bool IsFloat(ValueType type)   { return type == ValueType::Float; }
		//! Tests whether a given type is a double.
		[[maybe_unused]] constexpr bool IsDouble(ValueType type)  { return type == ValueType::Double; }

		//! Tests whether a given type is a C-function pointer.
		[[maybe_unused]] constexpr bool IsFunction(ValueType type) { return type == ValueType::Function; }
		//! Tests whether a given type is a string.
		[[maybe_unused]] constexpr bool IsString(ValueType type) { return type == ValueType::String; }

		//! Tests whether a given type is a object of any size.
		[[maybe_unused]] constexpr bool IsObject(ValueType type)  { return IsBetween(type, ValueType::_ObjectStart, ValueType::_ObjectEnd); }
		//! Tests whether a given type is a array of any size.
		[[maybe_unused]] constexpr bool IsArray(ValueType type)   { return IsBetween(type, ValueType::_ArrayStart, ValueType::_ArrayEnd); }
		//! Tests whether a given type is a pod structure of any size.
		[[maybe_unused]] constexpr bool IsStruct(ValueType type)  { return IsBetween(type, ValueType::_StructStart, ValueType::_StructEnd); }

		/**
		 * @brief Checks if a given ValueType is considered a hidden object parameter.
		 *
		 * @param type The ValueType to check.
		 * @return true if considered a hidden object parameter, false otherwise.
		 *
		 * This function determines if the provided ValueType is typically treated as a hidden object parameter.
		 * Hidden object parameters are those where the return argument is allocated by the caller function and
		 * passed as the first argument. This is often true for objects and large structs.
		 */
		[[maybe_unused]] constexpr bool IsHiddenParam(ValueType type) {
			return IsObject(type) || IsBetween(type, ValueType::_HiddenParamStart, ValueType::_StructEnd);
		}
		
		/**
		 * @brief Convert a ValueType enum value to its string representation.
		 * @param value The ValueType value to convert.
		 * @return The string representation of the ValueType.
		 */
		[[maybe_unused]] constexpr std::string_view ToString(ValueType value) {
			switch (value) {
				case ValueType::Void:          return ValueName::Void;
				case ValueType::Bool:          return ValueName::Bool;
				case ValueType::Char8:         return ValueName::Char8;
				case ValueType::Char16:        return ValueName::Char16;
				case ValueType::Int8:          return ValueName::Int8;
				case ValueType::Int16:         return ValueName::Int16;
				case ValueType::Int32:         return ValueName::Int32;
				case ValueType::Int64:         return ValueName::Int64;
				case ValueType::UInt8:         return ValueName::UInt8;
				case ValueType::UInt16:        return ValueName::UInt16;
				case ValueType::UInt32:        return ValueName::UInt32;
				case ValueType::UInt64:        return ValueName::UInt64;
				case ValueType::Pointer:       return ValueName::Pointer;
				case ValueType::Float:         return ValueName::Float;
				case ValueType::Double:        return ValueName::Double;
				case ValueType::Function:      return ValueName::Function;
				case ValueType::String:        return ValueName::String;
				case ValueType::ArrayBool:     return ValueName::ArrayBool;
				case ValueType::ArrayChar8:    return ValueName::ArrayChar8;
				case ValueType::ArrayChar16:   return ValueName::ArrayChar16;
				case ValueType::ArrayInt8:     return ValueName::ArrayInt8;
				case ValueType::ArrayInt16:    return ValueName::ArrayInt16;
				case ValueType::ArrayInt32:    return ValueName::ArrayInt32;
				case ValueType::ArrayInt64:    return ValueName::ArrayInt64;
				case ValueType::ArrayUInt8:    return ValueName::ArrayUInt8;
				case ValueType::ArrayUInt16:   return ValueName::ArrayUInt16;
				case ValueType::ArrayUInt32:   return ValueName::ArrayUInt32;
				case ValueType::ArrayUInt64:   return ValueName::ArrayUInt64;
				case ValueType::ArrayPointer:  return ValueName::ArrayPointer;
				case ValueType::ArrayFloat:    return ValueName::ArrayFloat;
				case ValueType::ArrayDouble:   return ValueName::ArrayDouble;
				case ValueType::ArrayString:   return ValueName::ArrayString;
				case ValueType::Vector2:       return ValueName::Vec2;
				case ValueType::Vector3:       return ValueName::Vec3;
				case ValueType::Vector4:       return ValueName::Vec4;
				case ValueType::Matrix4x4:     return ValueName::Mat4x4;
				default:                       return ValueName::Invalid;
			}
		}

		/**
		 * @brief Convert a string representation to a ValueType enum value.
		 * @param value The string representation of ValueType.
		 * @return The corresponding ValueType enum value.
		 */
		[[maybe_unused]] constexpr ValueType FromString(std::string_view value) {
			if (value == ValueName::Void) {
				return ValueType::Void;
			} else if (value == ValueName::Bool) {
				return ValueType::Bool;
			} else if (value == ValueName::Char8) {
				return ValueType::Char8;
			} else if (value == ValueName::Char16) {
				return ValueType::Char16;
			} else if (value == ValueName::Int8) {
				return ValueType::Int8;
			} else if (value == ValueName::Int16) {
				return ValueType::Int16;
			} else if (value == ValueName::Int32) {
				return ValueType::Int32;
			} else if (value == ValueName::Int64) {
				return ValueType::Int64;
			} else if (value == ValueName::UInt8) {
				return ValueType::UInt8;
			} else if (value == ValueName::UInt16) {
				return ValueType::UInt16;
			} else if (value == ValueName::UInt32) {
				return ValueType::UInt32;
			} else if (value == ValueName::UInt64) {
				return ValueType::UInt64;
			} else if (value == ValueName::Pointer) {
				return ValueType::Pointer;
			} else if (value == ValueName::Float) {
				return ValueType::Float;
			} else if (value == ValueName::Double) {
				return ValueType::Double;
			} else if (value == ValueName::Function) {
				return ValueType::Function;
			} else if (value == ValueName::String) {
				return ValueType::String;
			} else if (value == ValueName::ArrayBool) {
				return ValueType::ArrayBool;
			} else if (value == ValueName::ArrayChar8) {
				return ValueType::ArrayChar8;
			} else if (value == ValueName::ArrayChar16) {
				return ValueType::ArrayChar16;
			} else if (value == ValueName::ArrayInt8) {
				return ValueType::ArrayInt8;
			} else if (value == ValueName::ArrayInt16) {
				return ValueType::ArrayInt16;
			} else if (value == ValueName::ArrayInt32) {
				return ValueType::ArrayInt32;
			} else if (value == ValueName::ArrayInt64) {
				return ValueType::ArrayInt64;
			} else if (value == ValueName::ArrayUInt8) {
				return ValueType::ArrayUInt8;
			} else if (value == ValueName::ArrayUInt16) {
				return ValueType::ArrayUInt16;
			} else if (value == ValueName::ArrayUInt32) {
				return ValueType::ArrayUInt32;
			} else if (value == ValueName::ArrayUInt64) {
				return ValueType::ArrayUInt64;
			} else if (value == ValueName::ArrayPointer) {
				return ValueType::ArrayPointer;
			} else if (value == ValueName::ArrayFloat) {
				return ValueType::ArrayFloat;
			} else if (value == ValueName::ArrayDouble) {
				return ValueType::ArrayDouble;
			} else if (value == ValueName::ArrayString) {
				return ValueType::ArrayString;
			} else if (value == ValueName::Vec2) {
				return ValueType::Vector2;
			} else if (value == ValueName::Vec3) {
				return ValueType::Vector3;
			} else if (value == ValueName::Vec4) {
				return ValueType::Vector4;
			} else if (value == ValueName::Mat4x4) {
				return ValueType::Matrix4x4;
			}
			return ValueType::Invalid;
		}
	}

} // namespace plugify