#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <asmjit/asmjit.h>
#include <plugify/mem_addr.h>
#include <plugify/method.h>

namespace plugify{
	/**
	 * @class JitCall
	 * @brief Class encapsulates architecture-, OS- and compiler-specific
	 * function call semantics in a virtual "bind argument parameters from left
	 * to right and then call" interface allowing programmers to call C
	 * functions in a completely dynamic manner.
	 * In other words, instead of calling a function directly, class
	 * provides a mechanism to push the function parameters manually and to issue the call afterwards.
	 */
	class JitCall {
	public:
		/**
		 * @brief Constructor.
		 * @param rt Weak pointer to the asmjit::JitRuntime.
		 */
		explicit JitCall(std::weak_ptr<asmjit::JitRuntime> rt);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Caller.
		 */
		JitCall(JitCall&& other) noexcept;

		/**
		 * @brief Destructor.
		 */
		~JitCall();

		/**
		 * @struct Parameters
		 * @brief Structure to represent function parameters.
		 */
		struct Parameters {
			typedef const uintptr_t* Data;
			
			/**
			 * @brief Constructor.
			 * @param count Parameters count.
			 */
			explicit Parameters(uint8_t count) {
				arguments.reserve(count);
			}

			/**
			 * @brief Set the value of the argument at next available position.
			 * @tparam T Type of the argument.
			 * @param val Value to set.
			 * @noreturn
			 */
			template<typename T>
			void AddArgument(T val) {
				uintptr_t& arg = arguments.emplace_back(0);
				*(T*) &arg = val;
			}

			/**
			 * @brief Get a pointer to the argument storage.
			 * @return Pointer to the arguments storage.
			 */
			Data GetDataPtr() const noexcept {
				return arguments.data();
			}
			
		private:
			std::vector<uintptr_t> arguments; ///< Raw storage for function arguments.
		};
		
		struct Return {
			/**
			 * @brief Set the return value.
			 * @tparam T Type of the return value.
			 * @param val Value to set as the return value.
			 * @noreturn
			 */
			template<typename T>
			void SetReturn(T val) const noexcept {
				*(T*) GetReturnPtr() = val;
			}

			/**
			 * @brief Get the value of the return.
			 * @tparam T Type of the return.
			 * @return Value of the return.
			 */
			template<typename T>
			[[nodiscard]] T GetReturn() const noexcept {
				return *(T*) GetReturnPtr();
			}

			/**
			 * @brief Get a pointer to the return value.
			 * @return Pointer to the return value.
			 */
			[[nodiscard]] int8_t* GetReturnPtr() const noexcept {
				return (int8_t*) &ret[0];
			}

		private:
			volatile uintptr_t ret[2]{}; ///< Raw storage for the return value.
		};
		
		enum class WaitType {
			None,
			Int3,
			Wait_Keypress
		};
	
		using CallingFunc = void(*)(const Parameters::Data params, const Return*); // Return can be null
		using HiddenParam = bool(*)(ValueType);

		/**
		 * @brief Get a dynamically created function based on the raw signature. 
		 * @param sig Function signature.
		 * @param target Target function to call.
		 * @param waitType Optionally insert a breakpoint before the call.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(const asmjit::FuncSignature& sig, MemAddr target, WaitType waitType = WaitType::None);

		/**
		 * @brief Get a dynamically created function based on the method reference.
		 * @param method Reference to the method.
		 * @param target Target function to call.
		 * @param waitType Optionally insert a breakpoint before the call.
		 * @param hidden If true, return will be pass as first argument.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(MethodRef method, MemAddr target, WaitType waitType = WaitType::None, HiddenParam hidden = &ValueUtils::IsHiddenParam);

		/**
		 * @brief Get a dynamically created function.
		 * @return Pointer to the already generated function.
		 * @note The returned pointer can be nullptr if function is not generate.
		 */
		[[nodiscard]] MemAddr GetFunction() const noexcept { return _function; }

		/**
		 * @brief Get the target associated with the object.
		 * @details This function returns a pointer to the target function associated with the object.
		 * @return A void pointer to the target function.
		 * @note The returned pointer can be nullptr if no target is set.
		 */
		[[nodiscard]] MemAddr GetTargetFunc() const noexcept { return _targetFunc; }

		/**
		 * @brief Get the error message, if any.
		 * @return Error message.
		 */
		[[nodiscard]] std::string_view GetError() noexcept { return !_function && _errorCode ? _errorCode : ""; }

	private:
		std::weak_ptr<asmjit::JitRuntime> _rt;
		MemAddr _function;
		union {
			MemAddr _targetFunc;
			const char* _errorCode{};
		};
	};
} // namespace plugify