#include "helpers.h"

namespace plugify::JitUtils {
	asmjit::TypeId GetValueTypeId(ValueType valueType) noexcept {
		switch (valueType) {
			case ValueType::Invalid:
			case ValueType::Void:
				return GetTypeIdx<void>();
			case ValueType::Bool:
				return GetTypeIdx<bool>();
			case ValueType::Char8:
				return GetTypeIdx<char>();
			case ValueType::Char16:
				return GetTypeIdx<char16_t>();
			case ValueType::Int8:
				return GetTypeIdx<int8_t>();
			case ValueType::Int16:
				return GetTypeIdx<int16_t>();
			case ValueType::Int32:
				return GetTypeIdx<int32_t>();
			case ValueType::Int64:
				return GetTypeIdx<int64_t>();
			case ValueType::UInt8:
				return GetTypeIdx<uint8_t>();
			case ValueType::UInt16:
				return GetTypeIdx<uint16_t>();
			case ValueType::UInt32:
				return GetTypeIdx<uint32_t>();
			case ValueType::UInt64:
				return GetTypeIdx<uint64_t>();
			case ValueType::Float:
				return GetTypeIdx<float>();
			case ValueType::Double:
				return GetTypeIdx<double>();
			case ValueType::Pointer:
			case ValueType::String:
			case ValueType::Function:
			case ValueType::ArrayBool:
			case ValueType::ArrayChar8:
			case ValueType::ArrayChar16:
			case ValueType::ArrayInt8:
			case ValueType::ArrayInt16:
			case ValueType::ArrayInt32:
			case ValueType::ArrayInt64:
			case ValueType::ArrayUInt8:
			case ValueType::ArrayUInt16:
			case ValueType::ArrayUInt32:
			case ValueType::ArrayUInt64:
			case ValueType::ArrayPointer:
			case ValueType::ArrayFloat:
			case ValueType::ArrayDouble:
			case ValueType::ArrayString:
			case ValueType::Vector2:
			case ValueType::Vector3:
			case ValueType::Vector4:
			case ValueType::Matrix4x4:
				return asmjit::TypeId::kUIntPtr;
		}
		return asmjit::TypeId::kVoid;
	}

	asmjit::TypeId GetRetTypeId(ValueType valueType) noexcept {
#if PLUGIFY_ARCH_ARM
		switch (valueType) {
			case ValueType::Invalid:
			case ValueType::Void:
				return GetTypeIdx<void>();
			case ValueType::Bool:
				return GetTypeIdx<bool>();
			case ValueType::Char8:
				return GetTypeIdx<char>();
			case ValueType::Char16:
				return GetTypeIdx<char16_t>();
			case ValueType::Int8:
				return GetTypeIdx<int8_t>();
			case ValueType::Int16:
				return GetTypeIdx<int16_t>();
			case ValueType::Int32:
				return GetTypeIdx<int32_t>();
			case ValueType::Int64:
				return GetTypeIdx<int64_t>();
			case ValueType::UInt8:
				return GetTypeIdx<uint8_t>();
			case ValueType::UInt16:
				return GetTypeIdx<uint16_t>();
			case ValueType::UInt32:
				return GetTypeIdx<uint32_t>();
			case ValueType::UInt64:
				return GetTypeIdx<uint64_t>();
			case ValueType::Float:
				return GetTypeIdx<float>();
			case ValueType::Double:
				return GetTypeIdx<double>();
			case ValueType::Pointer:
			case ValueType::Function:
				return asmjit::TypeId::kUIntPtr;
			case ValueType::String:
			case ValueType::ArrayBool:
			case ValueType::ArrayChar8:
			case ValueType::ArrayChar16:
			case ValueType::ArrayInt8:
			case ValueType::ArrayInt16:
			case ValueType::ArrayInt32:
			case ValueType::ArrayInt64:
			case ValueType::ArrayUInt8:
			case ValueType::ArrayUInt16:
			case ValueType::ArrayUInt32:
			case ValueType::ArrayUInt64:
			case ValueType::ArrayPointer:
			case ValueType::ArrayFloat:
			case ValueType::ArrayDouble:
			case ValueType::ArrayString:
			case ValueType::Matrix4x4:
				return asmjit::TypeId::kVoid;
			case ValueType::Vector2:
				return asmjit::TypeId::kFloat32x2;
			case ValueType::Vector3:
				return asmjit::TypeId::kFloat64x2;
			case ValueType::Vector4:
				return asmjit::TypeId::kFloat32x4;
		}
#else
		switch (valueType) {
			case ValueType::Invalid:
			case ValueType::Void:
				return GetTypeIdx<void>();
			case ValueType::Bool:
				return GetTypeIdx<bool>();
			case ValueType::Char8:
				return GetTypeIdx<char>();
			case ValueType::Char16:
				return GetTypeIdx<char16_t>();
			case ValueType::Int8:
				return GetTypeIdx<int8_t>();
			case ValueType::Int16:
				return GetTypeIdx<int16_t>();
			case ValueType::Int32:
				return GetTypeIdx<int32_t>();
			case ValueType::Int64:
				return GetTypeIdx<int64_t>();
			case ValueType::UInt8:
				return GetTypeIdx<uint8_t>();
			case ValueType::UInt16:
				return GetTypeIdx<uint16_t>();
			case ValueType::UInt32:
				return GetTypeIdx<uint32_t>();
			case ValueType::UInt64:
				return GetTypeIdx<uint64_t>();
			case ValueType::Float:
				return GetTypeIdx<float>();
			case ValueType::Double:
				return GetTypeIdx<double>();
			case ValueType::Pointer:
			case ValueType::String:
			case ValueType::Function:
			case ValueType::ArrayBool:
			case ValueType::ArrayChar8:
			case ValueType::ArrayChar16:
			case ValueType::ArrayInt8:
			case ValueType::ArrayInt16:
			case ValueType::ArrayInt32:
			case ValueType::ArrayInt64:
			case ValueType::ArrayUInt8:
			case ValueType::ArrayUInt16:
			case ValueType::ArrayUInt32:
			case ValueType::ArrayUInt64:
			case ValueType::ArrayPointer:
			case ValueType::ArrayFloat:
			case ValueType::ArrayDouble:
			case ValueType::ArrayString:
			case ValueType::Matrix4x4:
				return asmjit::TypeId::kUIntPtr;
			case ValueType::Vector2:
#if PLUGIFY_PLATFORM_WINDOWS
				return asmjit::TypeId::kInt64;
#else
				return asmjit::TypeId::kFloat64;
#endif
			case ValueType::Vector3:
			case ValueType::Vector4:
#if PLUGIFY_PLATFORM_WINDOWS
				return asmjit::TypeId::kUIntPtr;
#else
				return asmjit::TypeId::kFloat32x4;
#endif
		}
		return asmjit::TypeId::kVoid;
#endif // PLUGIFY_ARCH_ARM
	}

	asmjit::CallConvId GetCallConv([[maybe_unused]] std::string_view conv) noexcept {
#if PLUGIFY_ARCH_ARM
#if PLUGIFY_ARCH_BITS == 64
		return asmjit::CallConvId::kHost;
#else
		if (!conv.empty()) {
			if (conv == "soft") {
				return asmjit::CallConvId::kSoftFloat;
			} else if (conv == "hard") {
				return asmjit::CallConvId::kHardFloat;
			}
		}
		return asmjit::CallConvId::kHost;
#endif // PLUGIFY_ARCH_BITS
#else
#if PLUGIFY_ARCH_BITS == 64
#if PLUGIFY_PLATFORM_WINDOWS
		if (conv == "vectorcall") [[unlikely]] {
			return asmjit::CallConvId::kVectorCall;
		} else {
			return asmjit::CallConvId::kX64Windows;
		}
#else
		return asmjit::CallConvId::kX64SystemV;
#endif // PLUGIFY_PLATFORM_WINDOWS
#elif PLUGIFY_ARCH_BITS == 32
		if (!conv.empty()) {
			if (conv == "cdecl") {
				return asmjit::CallConvId::kCDecl;
			} else if (conv == "stdcall") {
				return asmjit::CallConvId::kStdCall;
			} else if (conv == "fastcall") {
				return asmjit::CallConvId::kFastCall;
			} else if (conv == "thiscall") {
				return asmjit::CallConvId::kThisCall;
			} else if (conv == "vectorcall") {
				return asmjit::CallConvId::kVectorCall;
			}
		}
		return asmjit::CallConvId::kHost;
#endif // PLUGIFY_ARCH_BITS
#endif // PLUGIFY_ARCH_ARM
	}

} // namespace plugify
