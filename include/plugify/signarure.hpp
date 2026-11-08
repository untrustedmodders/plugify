#pragma once

#include <cstdint>
#include <string>

#include "plugify/value_type.hpp"
#include "plg/inplace_vector.hpp"

namespace plugify {
	/**
	 * @enum CallConv
	 * @brief Represents different function calling conventions used in code generation.
	 *
	 * Calling conventions define how function arguments are passed, who cleans the stack,
	 * and how return values are handled. Different platforms and compilers support different
	 * conventions.
	 */
	enum class CallConv : uint8_t {
		CDecl = 0,  ///< C declaration convention (caller cleans stack, right-to-left argument push)
		StdCall = 1,      ///< Standard call convention (callee cleans stack, right-to-left argument push)
		FastCall = 2,     ///< Fast call convention (arguments passed in registers when possible)
		VectorCall = 3,   ///< Vector call convention (uses SSE/vector registers for arguments)
		ThisCall = 4,     ///< 'This' call convention (C++ member functions, 'this' pointer in register)
		RegParm1 = 5,     ///< Register parameter convention 1 (platform-specific register usage)
		RegParm2 = 6,     ///< Register parameter convention 2 (platform-specific register usage)
		RegParm3 = 7,     ///< Register parameter convention 3 (platform-specific register usage)
		LightCall2 = 16,  ///< Lightweight call convention variant 2 (internal/optimized use)
		LightCall3 = 17,  ///< Lightweight call convention variant 3 (internal/optimized use)
		LightCall4 = 18,  ///< Lightweight call convention variant 4 (internal/optimized use)
		SoftFloat = 30,   ///< Software floating-point convention (uses FPU/emulation)
		HardFloat = 31,   ///< Hardware floating-point convention (uses FP registers)
		X64SystemV = 32,  ///< System V AMD64 convention (Linux/macOS x64 standard)
		X64Windows = 33,  ///< Microsoft x64 calling convention (Windows x64 standard)
		MaxValue = X64Windows  ///< Maximum valid value in this enumeration
	};

	/**
	 * @struct Signature
	 * @brief Replacement for asmjit::FuncSignature using ValueType
	 */
	struct Signature {
		inline static const size_t kMaxFuncArgs = 32;  ///< Largest number of arguments a signature can hold.
		inline static const uint8_t kNoVarArgs = 0xffU;  ///< Value of @ref varIndex for a function that is not variadic.

		CallConv callConv{CallConv::CDecl}; ///< Calling convention
		ValueType retType{ValueType::Void}; ///< Return type
		uint8_t varIndex{kNoVarArgs};  ///< Variable index for variadic functions
		std::inplace_vector<ValueType, kMaxFuncArgs> argTypes{}; ///< Argument types

		/**
		 * @brief Append an argument to the signature.
		 * @param type Type of the argument.
		 */
		void AddArg(ValueType type) {
			argTypes.push_back(type);
		}

		/**
		 * @brief Get how many arguments the signature has.
		 * @return The number of arguments.
		 */
		size_t ArgCount() const noexcept {
			return argTypes.size();
		}

		/**
		 * @brief Check whether the signature returns anything.
		 * @return True if the return type is not @ref ValueType::Void.
		 */
		bool HasRet() const noexcept {
			return retType != ValueType::Void;
		}

		/**
		 * @brief Set the return type of the signature.
		 * @param type The return type.
		 */
		void SetRet(ValueType type) noexcept {
			retType = type;
		}
	};

	/**
	 * @struct CallName
	 * @brief The string representation of each @ref CallConv value, as a manifest writes it.
	 */
	struct CallName {
		static constexpr std::string_view CDecl = "cdecl";
		static constexpr std::string_view StdCall = "stdcall";
		static constexpr std::string_view FastCall = "fastcall";
		static constexpr std::string_view VectorCall = "vectorcall";
		static constexpr std::string_view ThisCall = "thiscall";
		static constexpr std::string_view RegParm1 = "regparm1";
		static constexpr std::string_view RegParm2 = "regparm2";
		static constexpr std::string_view RegParm3 = "regparm3";
		static constexpr std::string_view LightCall2 = "lightcall2";
		static constexpr std::string_view LightCall3 = "lightcall3";
		static constexpr std::string_view LightCall4 = "lightcall4";
		static constexpr std::string_view SoftFloat = "softfloat";
		static constexpr std::string_view HardFloat = "hardfloat";
		static constexpr std::string_view X64SystemV = "x64systemv";
		static constexpr std::string_view X64Windows = "x64windows";
		static constexpr std::string_view MaxValue = "maxvalue";
	};
}
