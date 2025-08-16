#pragma once

#include <asmjit/core.h>
#include <plugify/asm/mem_addr.hpp>
#include <plugify/api/method.hpp>
#include <string>
#include <utility>
#include <memory>
#include <vector>

namespace plugify {
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
		 * @brief Copy constructor.
		 * @param other Another instance of Caller.
		 */
		JitCall(const JitCall& other) = delete;

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
			typedef const uint64_t* Data;
			
			/**
			 * @brief Constructor.
			 * @param count Parameters count.
			 */
			explicit Parameters(size_t count) {
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
				uint64_t& arg = arguments.emplace_back(0);
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
			std::vector<uint64_t> arguments; ///< Raw storage for function arguments.
		};
		
		struct Return {
			/**
			 * @brief Constructs an object of type `T` at the memory location.
			 * @param args The arguments to be forwarded to the constructor.
			 */
			template<typename T, typename...Args>
			void ConstructAt(Args&&... args) const noexcept {
				std::construct_at((T*) GetReturnPtr(), std::forward<Args>(args)...);
			}

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
			T GetReturn() const noexcept {
				return *(T*) GetReturnPtr();
			}

			/**
			 * @brief Get a pointer to the return value.
			 * @return Pointer to the return value.
			 */
			int8_t* GetReturnPtr() const noexcept {
				return (int8_t*) &ret[0];
			}

		private:
			volatile uint64_t ret[2]{}; ///< Raw storage for the return value.
		};
		
		enum class WaitType {
			None,
			Breakpoint,
			Wait_Keypress
		};
	
		using CallingFunc = void(*)(Parameters::Data params, const Return*); // Return can be null
		using HiddenParam = bool(*)(ValueType);

		/**
		 * @brief Get a dynamically created function based on the raw signature. 
		 * @param sig Function signature.
		 * @param target Target function to call.
		 * @param waitType Optionally insert a breakpoint before the call.
		 * @param hidden If true, return will be pass as hidden argument.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(const asmjit::FuncSignature& sig, MemAddr target, WaitType waitType, bool hidden);

		/**
		 * @brief Get a dynamically created function based on the method reference.
		 * @param method Reference to the method.
		 * @param target Target function to call.
		 * @param waitType Optionally insert a breakpoint before the call.
		 * @param hidden If true, return will be pass as hidden argument.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(MethodHandle method, MemAddr target, WaitType waitType = WaitType::None, HiddenParam hidden = &ValueUtils::IsHiddenParam);

		/**
		 * @brief Get a dynamically created function.
		 * @return Pointer to the already generated function.
		 * @note The returned pointer can be nullptr if function is not generate.
		 */
		MemAddr GetFunction() const noexcept { return _function; }

		/**
		 * @brief Get the target associated with the object.
		 * @details This function returns a pointer to the target function associated with the object.
		 * @return A void pointer to the target function.
		 * @note The returned pointer can be nullptr if no target is set.
		 */
		MemAddr GetTargetFunc() const noexcept { return _targetFunc; }

		/**
		 * @brief Get the error message, if any.
		 * @return Error message.
		 */
		std::string_view GetError() noexcept { return !_function && _errorCode ? _errorCode : ""; }

		/**
		 * @brief Copy assignment operator for JitCall.
		 *
		 * @param other The other JitCall instance to copy from.
		 * @return A reference to this instance after copying.
		 */
		JitCall& operator=(const JitCall& other) = delete;

		/**
		 * @brief Move assignment operator for JitCall.
		 *
		 * @param other The other JitCall instance to move from.
		 * @return A reference to this instance after moving.
		 * @note This operator is marked noexcept to indicate it does not throw exceptions.
		 */
		JitCall& operator=(JitCall&& other) noexcept;

	private:
		std::weak_ptr<asmjit::JitRuntime> _rt;
		MemAddr _function;
		union {
			MemAddr _targetFunc;
			const char* _errorCode{};
		};
	};
} // namespace plugify
