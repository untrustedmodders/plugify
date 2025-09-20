#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "plg/any.hpp"

namespace plugify {
	/**
	 * @enum ValueType
	 * @brief Enumerates possible types of values in the reflection system.
	 */
	enum class ValueType : uint8_t {
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
		Any,

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
		ArrayAny,
		ArrayVector2,
		ArrayVector3,
		ArrayVector4,
		ArrayMatrix4x4,

		// Structs
		Vector2,
		Vector3,
		Vector4,

		Matrix4x4,
		// Matrix2x2,
		// Matrix2x3,
		// Matrix2x4,
		// Matrix3x2,
		// Matrix3x3,
		// Matrix3x4,
		// Matrix4x2,
		// Matrix4x3,

		//! Helpers

		_BaseStart = Void,
		_BaseEnd = Function,

		_FloatStart = Float,
		_FloatEnd = Double,

		_ObjectStart = String,
		_ObjectEnd = ArrayMatrix4x4,

		_ArrayStart = ArrayBool,
		_ArrayEnd = ArrayMatrix4x4,

		_StructStart = Vector2,
		_StructEnd = Matrix4x4,

	// First struct which return as hidden parameter
#if PLUGIFY_PLATFORM_WINDOWS && !_M_ARM64 || INTPTR_MAX == INT32_MAX
		_HiddenParamStart = Vector3,
#else
		_HiddenParamStart = Matrix4x4,
#endif
		_LastAssigned = Matrix4x4,
	};

	/**
	 * @brief Namespace containing string representations of ValueType enum values.
	 */
	struct ValueName {
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
		static constexpr std::string_view Any = "any";
		static constexpr std::string_view ArrayBool = "bool[]";
		static constexpr std::string_view ArrayChar8 = "char8[]";
		static constexpr std::string_view ArrayChar16 = "char16[]";
		static constexpr std::string_view ArrayInt8 = "int8[]";
		static constexpr std::string_view ArrayInt16 = "int16[]";
		static constexpr std::string_view ArrayInt32 = "int32[]";
		static constexpr std::string_view ArrayInt64 = "int64[]";
		static constexpr std::string_view ArrayUInt8 = "uint8[]";
		static constexpr std::string_view ArrayUInt16 = "uint16[]";
		static constexpr std::string_view ArrayUInt32 = "uint32[]";
		static constexpr std::string_view ArrayUInt64 = "uint64[]";
		static constexpr std::string_view ArrayFloat = "float[]";
		static constexpr std::string_view ArrayDouble = "double[]";
		static constexpr std::string_view ArrayString = "string[]";
		static constexpr std::string_view ArrayAny = "any[]";
		static constexpr std::string_view ArrayVector2 = "vec2[]";
		static constexpr std::string_view ArrayVector3 = "vec3[]";
		static constexpr std::string_view ArrayVector4 = "vec4[]";
		static constexpr std::string_view ArrayMatrix4x4 = "mat4x4[]";
		static constexpr std::string_view Vector2 = "vec2";
		static constexpr std::string_view Vector3 = "vec3";
		static constexpr std::string_view Vector4 = "vec4";
		static constexpr std::string_view Matrix4x4 = "mat4x4";
		static constexpr std::string_view Invalid = "invalid";
#if INTPTR_MAX == INT32_MAX
		static constexpr std::string_view Pointer = "ptr32";
		static constexpr std::string_view ArrayPointer = "ptr32[]";
#elif INTPTR_MAX == INT64_MAX
		static constexpr std::string_view Pointer = "ptr64";
		static constexpr std::string_view ArrayPointer = "ptr64[]";
#else
#error "Environment not 32 or 64-bit."
#endif
	};

	/**
	 * @brief Namespace containing utility functions of ValueType enum.
	 */
	struct ValueUtils {

		/**
		 * @brief Checks if a value is between two other values.
		 *
		 * @tparam T The type of the values.
		 * @param x The value to check.
		 * @param a The lower bound.
		 * @param b The upper bound.
		 * @return True if x is between a and b, inclusive. False otherwise.
		 */
		template <typename T>
		static constexpr bool IsBetween(T x, T a, T b) noexcept {
			return x >= a && x <= b;
		}

		/**
		 * @brief Tests whether a given type is \ref ValueType::Void.
		 *
		 * @param type The type to test.
		 * @return True if type is ValueType::Void. False otherwise.
		 */
		static constexpr bool IsVoid(ValueType type) noexcept {
			return type == ValueType::Void;
		}

		/**
		 * @brief Tests whether a given type is a valid non-void type.
		 *
		 * @param type The type to test.
		 * @return True if type is a valid non-void type. False otherwise.
		 */
		static constexpr bool IsValid(ValueType type) noexcept {
			return IsBetween(type, ValueType::Void, ValueType::_LastAssigned);
		}

		/**
		 * @brief Tests whether a given type is scalar (has no vector part).
		 *
		 * @param type The type to test.
		 * @return True if type is scalar. False otherwise.
		 */
		static constexpr bool IsScalar(ValueType type) noexcept {
			return IsBetween(type, ValueType::_BaseStart, ValueType::_BaseEnd);
		}

		/**
		 * @brief Tests whether a given type is a scalar floating point of any size.
		 *
		 * @param type The type to test.
		 * @return True if type is a scalar floating point. False otherwise.
		 */
		static constexpr bool IsFloating(ValueType type) noexcept {
			return IsBetween(type, ValueType::_FloatStart, ValueType::_FloatEnd);
		}

		/**
		 * @brief Tests whether a given type is a 1-bit boolean.
		 *
		 * @param type The type to test.
		 * @return True if type is a 1-bit boolean. False otherwise.
		 */
		static constexpr bool IsBool(ValueType type) noexcept {
			return type == ValueType::Bool;
		}

		/**
		 * @brief Tests whether a given type is an 8-bit character.
		 *
		 * @param type The type to test.
		 * @return True if type is an 8-bit character. False otherwise.
		 */
		static constexpr bool IsChar8(ValueType type) noexcept {
			return type == ValueType::Char8;
		}

		/**
		 * @brief Tests whether a given type is a 16-bit character.
		 *
		 * @param type The type to test.
		 * @return True if type is a 16-bit character. False otherwise.
		 */
		static constexpr bool IsChar16(ValueType type) noexcept {
			return type == ValueType::Char16;
		}

		/**
		 * @brief Tests whether a given type is an 8-bit integer.
		 *
		 * @param type The type to test.
		 * @return True if type is an 8-bit integer. False otherwise.
		 */
		static constexpr bool IsInt8(ValueType type) noexcept {
			return type == ValueType::Int8;
		}

		/**
		 * @brief Tests whether a given type is an 8-bit unsigned integer.
		 *
		 * @param type The type to test.
		 * @return True if type is an 8-bit unsigned integer. False otherwise.
		 */
		static constexpr bool IsUInt8(ValueType type) noexcept {
			return type == ValueType::UInt8;
		}

		/**
		 * @brief Tests whether a given type is a 16-bit integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 16-bit integer. False otherwise.
		 */
		static constexpr bool IsInt16(ValueType type) noexcept {
			return type == ValueType::Int16;
		}

		/**
		 * @brief Tests whether a given type is a 16-bit unsigned integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 16-bit unsigned integer. False otherwise.
		 */
		static constexpr bool IsUInt16(ValueType type) noexcept {
			return type == ValueType::UInt16;
		}

		/**
		 * @brief Tests whether a given type is a 32-bit integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 32-bit integer. False otherwise.
		 */
		static constexpr bool IsInt32(ValueType type) noexcept {
			return type == ValueType::Int32;
		}

		/**
		 * @brief Tests whether a given type is a 32-bit unsigned integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 32-bit unsigned integer. False otherwise.
		 */
		static constexpr bool IsUInt32(ValueType type) noexcept {
			return type == ValueType::UInt32;
		}

		/**
		 * @brief Tests whether a given type is a 64-bit integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 64-bit integer. False otherwise.
		 */
		static constexpr bool IsInt64(ValueType type) noexcept {
			return type == ValueType::Int64;
		}

		/**
		 * @brief Tests whether a given type is a 64-bit unsigned integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 64-bit unsigned integer. False otherwise.
		 */
		static constexpr bool IsUInt64(ValueType type) noexcept {
			return type == ValueType::UInt64;
		}

		/**
		 * @brief Tests whether a given type is a pointer.
		 *
		 * @param type The type to test.
		 * @return True if type is a pointer. False otherwise.
		 */
		static constexpr bool IsPointer(ValueType type) noexcept {
			return type == ValueType::Pointer;
		}

		/**
		 * @brief Tests whether a given type is a float.
		 *
		 * @param type The type to test.
		 * @return True if type is a float. False otherwise.
		 */
		static constexpr bool IsFloat(ValueType type) noexcept {
			return type == ValueType::Float;
		}

		/**
		 * @brief Tests whether a given type is a double.
		 *
		 * @param type The type to test.
		 * @return True if type is a double. False otherwise.
		 */
		static constexpr bool IsDouble(ValueType type) noexcept {
			return type == ValueType::Double;
		}

		/**
		 * @brief Tests whether a given type is a C-function pointer.
		 *
		 * @param type The type to test.
		 * @return True if type is a C-function pointer. False otherwise.
		 */
		static constexpr bool IsFunction(ValueType type) noexcept {
			return type == ValueType::Function;
		}

		/**
		 * @brief Tests whether a given type is a string.
		 *
		 * @param type The type to test.
		 * @return True if type is a string. False otherwise.
		 */
		static constexpr bool IsString(ValueType type) noexcept {
			return type == ValueType::String;
		}

		/**
		 * @brief Tests whether a given type is an any.
		 *
		 * @param type The type to test.
		 * @return True if type is an any. False otherwise.
		 */
		static constexpr bool IsAny(ValueType type) noexcept {
			return type == ValueType::Any;
		}

		/**
		 * @brief Tests whether a given type is an object of any size.
		 *
		 * @param type The type to test.
		 * @return True if type is an object. False otherwise.
		 */
		static constexpr bool IsObject(ValueType type) noexcept {
			return IsBetween(type, ValueType::_ObjectStart, ValueType::_ObjectEnd);
		}

		/**
		 * @brief Tests whether a given type is an array of any size.
		 *
		 * @param type The type to test.
		 * @return True if type is an array. False otherwise.
		 */
		static constexpr bool IsArray(ValueType type) noexcept {
			return IsBetween(type, ValueType::_ArrayStart, ValueType::_ArrayEnd);
		}

		/**
		 * @brief Tests whether a given type is a POD (plain old data) structure of any size.
		 *
		 * @param type The type to test.
		 * @return True if type is a POD structure. False otherwise.
		 */
		static constexpr bool IsStruct(ValueType type) noexcept {
			return IsBetween(type, ValueType::_StructStart, ValueType::_StructEnd);
		}

		/**
		 * @brief Checks if a given ValueType is considered a hidden object parameter.
		 *
		 * @param type The ValueType to check.
		 * @return true if considered a hidden object parameter, false otherwise.
		 *
		 * This function determines if the provided ValueType is typically treated as a hidden
		 * object parameter. Hidden object parameters are those where the return argument is
		 * allocated by the caller function and passed as the first argument. This is often true for
		 * objects and large structs.
		 */
		static constexpr bool IsHiddenParam(ValueType type) noexcept {
			return IsObject(type) || IsBetween(type, ValueType::_HiddenParamStart, ValueType::_StructEnd);
		}

		/**
		 * @brief Returns the size in bytes of the given ValueType.
		 *
		 * Note: For arrays, this returns the size of the container (std::vector),
		 * not the size of the elements inside.
		 *
		 * @param type The ValueType to check.
		 * @return The size in bytes of the corresponding C++ type.
		 */
		static constexpr size_t SizeOf(ValueType type) noexcept {
			switch (type) {
				case ValueType::Bool:
					return sizeof(bool);
				case ValueType::Char8:
					return sizeof(char);
				case ValueType::Char16:
					return sizeof(char16_t);
				case ValueType::Int8:
					return sizeof(int8_t);
				case ValueType::Int16:
					return sizeof(int16_t);
				case ValueType::Int32:
					return sizeof(int32_t);
				case ValueType::Int64:
					return sizeof(int64_t);
				case ValueType::UInt8:
					return sizeof(uint8_t);
				case ValueType::UInt16:
					return sizeof(uint16_t);
				case ValueType::UInt32:
					return sizeof(uint32_t);
				case ValueType::UInt64:
					return sizeof(uint64_t);
				case ValueType::Pointer:
					return sizeof(void*);
				case ValueType::Float:
					return sizeof(float);
				case ValueType::Double:
					return sizeof(double);

				case ValueType::String:
					return sizeof(plg::string);
				case ValueType::Any:
					return sizeof(plg::any);

				// Arrays -> plg::vector overhead (not element size!)
				case ValueType::ArrayBool:
					return sizeof(plg::vector<bool>);
				case ValueType::ArrayChar8:
					return sizeof(plg::vector<char>);
				case ValueType::ArrayChar16:
					return sizeof(plg::vector<char16_t>);
				case ValueType::ArrayInt8:
					return sizeof(plg::vector<int8_t>);
				case ValueType::ArrayInt16:
					return sizeof(plg::vector<int16_t>);
				case ValueType::ArrayInt32:
					return sizeof(plg::vector<int32_t>);
				case ValueType::ArrayInt64:
					return sizeof(plg::vector<int64_t>);
				case ValueType::ArrayUInt8:
					return sizeof(plg::vector<uint8_t>);
				case ValueType::ArrayUInt16:
					return sizeof(plg::vector<uint16_t>);
				case ValueType::ArrayUInt32:
					return sizeof(plg::vector<uint32_t>);
				case ValueType::ArrayUInt64:
					return sizeof(plg::vector<uint64_t>);
				case ValueType::ArrayPointer:
					return sizeof(plg::vector<void*>);
				case ValueType::ArrayFloat:
					return sizeof(plg::vector<float>);
				case ValueType::ArrayDouble:
					return sizeof(plg::vector<double>);
				case ValueType::ArrayString:
					return sizeof(plg::vector<plg::string>);
				case ValueType::ArrayAny:
					return sizeof(plg::vector<plg::any>);
				case ValueType::ArrayVector2:
					return sizeof(plg::vector<plg::vec2>);
				case ValueType::ArrayVector3:
					return sizeof(plg::vector<plg::vec3>);
				case ValueType::ArrayVector4:
					return sizeof(plg::vector<plg::vec4>);
				case ValueType::ArrayMatrix4x4:
					return sizeof(plg::vector<plg::mat4x4>);

				// Structs
				case ValueType::Vector2:
					return sizeof(plg::vec2);
				case ValueType::Vector3:
					return sizeof(plg::vec3);
				case ValueType::Vector4:
					return sizeof(plg::vec4);
				case ValueType::Matrix4x4:
					return sizeof(plg::mat4x4);

				// Special cases
				case ValueType::Function:
					return sizeof(plg::function);
				case ValueType::Void:
					return 0;

				default:
					return 0;  // Invalid / unsupported
			}
		}
	};

	// Test if ValueType enum values match the indices in plg::any
	static_assert(ValueType::Void == static_cast<ValueType>(plg::any::index_of<plg::none>));
	static_assert(ValueType::Bool == static_cast<ValueType>(plg::any::index_of<bool>));
	static_assert(ValueType::Char8 == static_cast<ValueType>(plg::any::index_of<char>));
	static_assert(ValueType::Char16 == static_cast<ValueType>(plg::any::index_of<char16_t>));
	static_assert(ValueType::Int8 == static_cast<ValueType>(plg::any::index_of<int8_t>));
	static_assert(ValueType::Int16 == static_cast<ValueType>(plg::any::index_of<int16_t>));
	static_assert(ValueType::Int32 == static_cast<ValueType>(plg::any::index_of<int32_t>));
	static_assert(ValueType::Int64 == static_cast<ValueType>(plg::any::index_of<int64_t>));
	static_assert(ValueType::UInt8 == static_cast<ValueType>(plg::any::index_of<uint8_t>));
	static_assert(ValueType::UInt16 == static_cast<ValueType>(plg::any::index_of<uint16_t>));
	static_assert(ValueType::UInt32 == static_cast<ValueType>(plg::any::index_of<uint32_t>));
	static_assert(ValueType::UInt64 == static_cast<ValueType>(plg::any::index_of<uint64_t>));
	static_assert(ValueType::Pointer == static_cast<ValueType>(plg::any::index_of<void*>));
	static_assert(ValueType::Float == static_cast<ValueType>(plg::any::index_of<float>));
	static_assert(ValueType::Double == static_cast<ValueType>(plg::any::index_of<double>));
	static_assert(ValueType::Function == static_cast<ValueType>(plg::any::index_of<plg::function>));
	static_assert(ValueType::String == static_cast<ValueType>(plg::any::index_of<plg::string>));
	static_assert(ValueType::Any == static_cast<ValueType>(plg::any::index_of<plg::variant<plg::none>>));
	static_assert(ValueType::ArrayBool == static_cast<ValueType>(plg::any::index_of<plg::vector<bool>>));
	static_assert( ValueType::ArrayChar8 == static_cast<ValueType>(plg::any::index_of<plg::vector<char>>));
	static_assert( ValueType::ArrayChar16 == static_cast<ValueType>(plg::any::index_of<plg::vector<char16_t>>));
	static_assert( ValueType::ArrayInt8 == static_cast<ValueType>(plg::any::index_of<plg::vector<int8_t>>));
	static_assert( ValueType::ArrayInt16 == static_cast<ValueType>(plg::any::index_of<plg::vector<int16_t>>));
	static_assert( ValueType::ArrayInt32 == static_cast<ValueType>(plg::any::index_of<plg::vector<int32_t>>));
	static_assert( ValueType::ArrayInt64 == static_cast<ValueType>(plg::any::index_of<plg::vector<int64_t>>));
	static_assert( ValueType::ArrayUInt8 == static_cast<ValueType>(plg::any::index_of<plg::vector<uint8_t>>));
	static_assert( ValueType::ArrayUInt16 == static_cast<ValueType>(plg::any::index_of<plg::vector<uint16_t>>));
	static_assert( ValueType::ArrayUInt32 == static_cast<ValueType>(plg::any::index_of<plg::vector<uint32_t>>));
	static_assert( ValueType::ArrayUInt64 == static_cast<ValueType>(plg::any::index_of<plg::vector<uint64_t>>));
	static_assert( ValueType::ArrayPointer == static_cast<ValueType>(plg::any::index_of<plg::vector<void*>>));
	static_assert( ValueType::ArrayFloat == static_cast<ValueType>(plg::any::index_of<plg::vector<float>>));
	static_assert( ValueType::ArrayDouble == static_cast<ValueType>(plg::any::index_of<plg::vector<double>>));
	static_assert( ValueType::ArrayString == static_cast<ValueType>(plg::any::index_of<plg::vector<plg::string>>));
	static_assert( ValueType::ArrayAny == static_cast<ValueType>(plg::any::index_of<plg::vector<plg::variant<plg::none>>>));
	static_assert( ValueType::ArrayVector2 == static_cast<ValueType>(plg::any::index_of<plg::vector<plg::vec2>>));
	static_assert( ValueType::ArrayVector3 == static_cast<ValueType>(plg::any::index_of<plg::vector<plg::vec3>>));
	static_assert( ValueType::ArrayVector4 == static_cast<ValueType>(plg::any::index_of<plg::vector<plg::vec4>>));
	static_assert( ValueType::ArrayMatrix4x4 == static_cast<ValueType>(plg::any::index_of<plg::vector<plg::mat4x4>>));
	static_assert(ValueType::Vector2 == static_cast<ValueType>(plg::any::index_of<plg::vec2>));
	static_assert(ValueType::Vector3 == static_cast<ValueType>(plg::any::index_of<plg::vec3>));
	static_assert(ValueType::Vector4 == static_cast<ValueType>(plg::any::index_of<plg::vec4>));
}  // namespace plugify
