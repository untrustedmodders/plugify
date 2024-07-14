#pragma once

#include <memory>
#include <string_view>
#include <asmjit/asmjit.h>
#include <plugify/mem_addr.h>
#include <plugify/method.h>

namespace plugify {
	/**
	 * @struct Parameters
	 * @brief Structure to represent function parameters.
	 */
	struct Parameters {
		/**
		 * @brief Set the value of the argument at the specified index.
		 * @tparam T Type of the argument.
		 * @param idx Index of the argument.
		 * @param val Value to set.
		 * @noreturn
		 */
		template<typename T>
		void SetArgument(uint8_t idx, T val) const {
			*(T*) GetArgumentPtr(idx) = val;
		}

		/**
		 * @brief Set the value of the argument at the specified index and position within a multi-dimensional array.
		 * @tparam T Type of the argument.
		 * @param idx Index of the argument.
		 * @param val Value to set.
		 * @param i Position within the array (optional, default is 0).
		 * @noreturn
		 */
		template<typename T>
		void SetArgumentAt(uint8_t idx, T val, uint8_t i = 0) const {
			(*(T**) GetArgumentPtr(idx))[i] = val;
		}

		/**
		 * @brief Get the value of the argument at the specified index.
		 * @tparam T Type of the argument.
		 * @param idx Index of the argument.
		 * @return Value of the argument.
		 */
		template<typename T>
		[[nodiscard]] T GetArgument(uint8_t idx) const {
			return *(T*) GetArgumentPtr(idx);
		}

		volatile uintptr_t arguments; ///< Raw storage for function arguments.

		/**
		 * @brief Get a pointer to the argument at the specified index.
		 * @param idx Index of the argument.
		 * @return Pointer to the argument.
		 */
		[[nodiscard]] int8_t* GetArgumentPtr(uint8_t idx) const {
			return ((int8_t*)&arguments) + sizeof(uintptr_t) * idx;
		}
	};

	/**
	 * @struct ReturnValue
	 * @brief Structure to represent the return value of a function.
	 */
	struct ReturnValue {
		/**
		 * @brief Set the return value.
		 * @tparam T Type of the return value.
		 * @param val Value to set as the return value.
		 * @noreturn
		 */
		template<typename T>
		void SetReturnPtr(T val) const {
			*(T*) GetReturnPtr() = val;
		}

		/**
		 * @brief Get a pointer to the return value.
		 * @return Pointer to the return value.
		 */
		[[nodiscard]] int8_t* GetReturnPtr() const {
			return (int8_t*)&ret;
		}

		volatile uintptr_t ret; ///< Raw storage for the return value.
	};

	/**
	 * @class Function
	 * @brief Class for dynamic function generation.
	 */
	class Function {
	public:
		/**
		 * @brief Constructor.
		 * @param rt Weak pointer to the asmjit::JitRuntime.
		 */
		explicit Function(std::weak_ptr<asmjit::JitRuntime> rt);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Function.
		 */
		Function(Function&& other) noexcept;

		/**
		 * @brief Destructor.
		 */
		~Function();

		using FuncCallback = void(*)(MethodRef method, MemAddr data, const Parameters* params, uint8_t count, const ReturnValue* ret);
		using HiddenParam = bool(*)(ValueType);

		/**
		 * @brief Get a dynamically created callback function based on the raw signature.
		 * @param sig Function signature.
		 * @param method Reference to the method.
		 * @param callback Callback function.
		 * @param data User data.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(const asmjit::FuncSignature& sig, MethodRef method, FuncCallback callback, MemAddr data = nullptr);

		/**
		 * @brief Get a dynamically created callback function using a typedef represented as a string.
		 * @param method Reference to the method.
		 * @param callback Callback function.
		 * @param data User data.
		 * @param hidden If true, return will be pass as first argument.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(MethodRef method, FuncCallback callback, MemAddr data = nullptr, HiddenParam hidden = &ValueUtils::IsHiddenParam);

		/**
		 * @brief Get a dynamically created function.
		 * @return Pointer to the already generated function.
		 * @note The returned pointer can be nullptr if function is not generate.
		 */
		[[nodiscard]] MemAddr GetFunction() const { return _function; }

		/**
		 * @brief Get the user data associated with the object.
		 * @details This function returns a pointer to the user data associated with the object.
		 * @return A void pointer to the user data.
		 * @note The returned pointer can be nullptr if no user data is set.
		 */
		[[nodiscard]] MemAddr GetUserData() const { return _userData; }

		/**
		 * @brief Get the error message, if any.
		 * @return Error message.
		 */
		[[nodiscard]] std::string_view GetError() { return !_function && _errorCode ? _errorCode : std::string_view{}; }

	private:
		std::weak_ptr<asmjit::JitRuntime> _rt;
		MemAddr _function;
		union {
			MemAddr _userData;
			const char* _errorCode{};
		};
	};
} // namespace plugify