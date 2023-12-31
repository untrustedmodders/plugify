#pragma once

#include <wizard_export.h>
#include <wizard/method.h>
#include <asmjit/asmjit.h>
#include <memory>

namespace wizard {
    struct Parameters {
        template<typename T>
        void SetArgument(uint8_t idx, T val) const {
            *(T*) GetArgumentPtr(idx) = val;
        }

        template<typename T>
        T GetArgument(uint8_t idx) const {
            return *(T*) GetArgumentPtr(idx);
        }

        // asm depends on this specific type
        // we the Native allocates stack space that is set to point here
        volatile uintptr_t arguments;

        // must be char* for aliasing rules to work when reading back out
        int8_t* GetArgumentPtr(uint8_t idx) const {
            return ((int8_t*)&arguments) + sizeof(uintptr_t) * idx;
        }
    };

    struct ReturnValue {
        template<typename T>
        void SetReturnPtr(T val) const {
            *(T*) GetReturnPtr() = val;
        }
        uint8_t* GetReturnPtr() const {
            return (uint8_t*)&ret;
        }
        uintptr_t ret;
    };

    class Function {
    public:
        explicit Function(std::weak_ptr<asmjit::JitRuntime> rt);
        Function(Function&& other) noexcept;
        ~Function();

        typedef void(*FuncCallback)(const Method* method, const Parameters* params, const uint8_t count, const ReturnValue* ret);

        /**
         * Create a callback function dynamically based on the raw signature at runtime.
         * The "callback" parameter serves as the C stub for transfer, allowing the modification of parameters through a structure that is then written back to the parameter slots, depending on the calling convention.
         */
        void* GetJitFunc(const asmjit::FuncSignature& sig, const Method& method, FuncCallback callback);

        /**
         * Create a callback function dynamically using the typedef represented as a string.
         * The types can be any valid C/C++ data type (basic types), and pointers to any type are simply denoted as a pointer.
         * The calling convention defaults to the typical convention for the compiler in use, but it can be overridden with stdcall, fastcall, or cdecl (with cdecl being the default on x86). On x64, these conventions map to the same behavior.
         */
        void* GetJitFunc(const Method& method, FuncCallback callback);

    private:
        static asmjit::CallConvId GetCallConv(const std::string& conv);
        static asmjit::TypeId GetTypeId(ValueType type);

        static bool IsGeneralReg(asmjit::TypeId typeId) ;
        static bool IsXmmReg(asmjit::TypeId typeId) ;

    private:
        std::weak_ptr<asmjit::JitRuntime> _rt;
        void* _callback{ nullptr };
    };
}