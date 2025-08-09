#include "../helpers.hpp"

namespace plugify {
	bool HasHiArgSlot(asmjit::TypeId typeId) noexcept {
		// 64bit width regs can fit wider args
		if constexpr (PLUGIFY_ARCH_BITS == 64) {
			return false;
		} else {
		    switch (typeId) {
		        case asmjit::TypeId::kInt64:
		        case asmjit::TypeId::kUInt64:
		            return true;
		        default:
		            return false;
		    }
		}
	}

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
			case ValueType::Any:
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
			case ValueType::ArrayAny:
			case ValueType::ArrayVector2:
			case ValueType::ArrayVector3:
			case ValueType::ArrayVector4:
			case ValueType::ArrayMatrix4x4:
			case ValueType::Vector2:
			case ValueType::Vector3:
			case ValueType::Vector4:
			case ValueType::Matrix4x4:
				return asmjit::TypeId::kUIntPtr;
		}
		return asmjit::TypeId::kVoid;
	}

	asmjit::TypeId GetRetTypeId(ValueType valueType) noexcept {
#if PLUGIFY_ARCH_BITS == 64
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
			case ValueType::Any:
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
			case ValueType::ArrayAny:
			case ValueType::ArrayVector2:
			case ValueType::ArrayVector3:
			case ValueType::ArrayVector4:
			case ValueType::ArrayMatrix4x4:
			case ValueType::Matrix4x4:
				return asmjit::TypeId::kUIntPtr; //-V525
			case ValueType::Vector2:
#if PLUGIFY_PLATFORM_WINDOWS
				return asmjit::TypeId::kInt64;
#else
				return asmjit::TypeId::kFloat64;
#endif // PLUGIFY_PLATFORM_WINDOWS
			case ValueType::Vector3:
			case ValueType::Vector4:
#if PLUGIFY_PLATFORM_WINDOWS
				return asmjit::TypeId::kUIntPtr;
#else
				return asmjit::TypeId::kFloat32x4;
#endif // PLUGIFY_PLATFORM_WINDOWS
		}
#elif PLUGIFY_ARCH_BITS == 32
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
			case ValueType::Any:
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
			case ValueType::ArrayAny:
			case ValueType::ArrayVector2:
			case ValueType::ArrayVector3:
			case ValueType::ArrayVector4:
			case ValueType::ArrayMatrix4x4:
			case ValueType::Vector3:
			case ValueType::Vector4:
			case ValueType::Matrix4x4:
				return asmjit::TypeId::kUIntPtr; //-V525
			case ValueType::Vector2:
				return asmjit::TypeId::kInt64;
		}
#endif // PLUGIFY_ARCH_BITS
		return asmjit::TypeId::kVoid;
	}

	asmjit::CallConvId GetCallConvId(CallConv callConv) noexcept {
	    switch (callConv) {
	        case CallConv::CDecl:        return asmjit::CallConvId::kCDecl;
	        case CallConv::StdCall:      return asmjit::CallConvId::kStdCall;
	        case CallConv::FastCall:     return asmjit::CallConvId::kFastCall;
	        case CallConv::VectorCall:   return asmjit::CallConvId::kVectorCall;
	        case CallConv::ThisCall:     return asmjit::CallConvId::kThisCall;
	        case CallConv::RegParm1:     return asmjit::CallConvId::kRegParm1;
	        case CallConv::RegParm2:     return asmjit::CallConvId::kRegParm2;
	        case CallConv::RegParm3:     return asmjit::CallConvId::kRegParm3;
	        case CallConv::LightCall2:   return asmjit::CallConvId::kLightCall2;
	        case CallConv::LightCall3:   return asmjit::CallConvId::kLightCall3;
	        case CallConv::LightCall4:   return asmjit::CallConvId::kLightCall4;
	        case CallConv::SoftFloat:    return asmjit::CallConvId::kSoftFloat;
	        case CallConv::HardFloat:    return asmjit::CallConvId::kHardFloat;
	        case CallConv::X64SystemV:   return asmjit::CallConvId::kX64SystemV;
	        case CallConv::X64Windows:   return asmjit::CallConvId::kX64Windows;
	        default:
	            // Fallback for unknown values
	            return asmjit::CallConvId::kCDecl;
	    }
	}

} // namespace plugify
