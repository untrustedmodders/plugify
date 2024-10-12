#pragma once

#include <memory>
#include <string_view>
#include <asmjit/asmjit.h>
#include <plugify/mem_addr.h>
#include <plugify/method.h>

namespace plugify {
	/**
	 * @class JitCallback
	 * @brief Class to create callback
	 * objects, that can be passed to functions as callback function pointers.
	 * In other words, a pointer to the callback object can be "called",
	 * directly. A generic callback handler invoked by this object then allows
	 * iterating dynamically over the arguments once called back.
	 */
	class JitCallback {
	public:
		/**
		 * @brief Constructor.
		 * @param rt Weak pointer to the asmjit::JitRuntime.
		 */
		explicit JitCallback(std::weak_ptr<asmjit::JitRuntime> rt);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Callback.
		 */
		JitCallback(JitCallback&& other) noexcept;

		/**
		 * @brief Destructor.
		 */
		~JitCallback();

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
			void SetArgument(uint8_t idx, T val) const noexcept {
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
			void SetArgumentAt(uint8_t idx, T val, uint8_t i = 0) const noexcept {
				(*(T**) GetArgumentPtr(idx))[i] = val;
			}

			/**
			 * @brief Get the value of the argument at the specified index.
			 * @tparam T Type of the argument.
			 * @param idx Index of the argument.
			 * @return Value of the argument.
			 */
			template<typename T>
			[[nodiscard]] T GetArgument(uint8_t idx) const noexcept {
				return *(T*) GetArgumentPtr(idx);
			}

			/**
			 * @brief Get a pointer to the argument at the specified index.
			 * @param idx Index of the argument.
			 * @return Pointer to the argument.
			 */
			[[nodiscard]] int8_t* GetArgumentPtr(uint8_t idx) const noexcept {
				return ((int8_t*) &arguments) + sizeof(uint64_t) * idx;
			}

		private:
			volatile uint64_t arguments; ///< Raw storage for function arguments.
		};

		/**
		 * @struct ReturnValue
		 * @brief Structure to represent the return value of a function.
		 */
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
			 * @brief Get the return value.
			 * @tparam T Type of the return value.
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
				return (int8_t*) &ret;
			}

		private:
			volatile uint64_t ret; ///< Raw storage for the return value.
		};

		using CallbackHandler = void(*)(MethodRef method, MemAddr data, const Parameters* params, uint8_t count, const Return* ret);
		using HiddenParam = bool(*)(ValueType);

		/**
		 * @brief Get a dynamically created callback function based on the raw signature.
		 * @param sig Function signature.
		 * @param method Reference to the method.
		 * @param callback Callback function.
		 * @param data User data.
		 * @param hidden If true, return will be pass as hidden argument.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(const asmjit::FuncSignature& sig, MethodRef method, CallbackHandler callback, MemAddr data, bool hidden);

		/**
		 * @brief Get a dynamically created function based on the method reference.
		 * @param method Reference to the method.
		 * @param callback Callback function.
		 * @param data User data.
		 * @param hidden If true, return will be pass as hidden argument.
		 * @return Pointer to the generated function.
		 *
		 * @details Creates a new callback object, where method is a
		 * signature describing the function to be called back, and callback is a pointer to a generic
		 * callback handler (see below). The signature is needed in the generic
		 * callback handler to correctly retrieve the arguments provided by the
		 * caller of the callback. Note that the generic handler's function
		 * type/declaration is always the same for any callback. userdata is a
		 * pointer to arbitrary user data to be available in the generic callback handler.
		 */
		MemAddr GetJitFunc(MethodRef method, CallbackHandler callback, MemAddr data = nullptr, HiddenParam hidden = &ValueUtils::IsHiddenParam);

		/**
		 * @brief Get a dynamically created function.
		 * @return Pointer to the already generated function.
		 * @note The returned pointer can be nullptr if function is not generate.
		 */
		[[nodiscard]] MemAddr GetFunction() const noexcept { return _function; }

		/**
		 * @brief Get the user data associated with the object.
		 * @details This function returns a pointer to the user data associated with the object.
		 * @return A void pointer to the user data.
		 * @note The returned pointer can be nullptr if no user data is set.
		 */
		[[nodiscard]] MemAddr GetUserData() const noexcept { return _userData; }

		/**
		 * @brief Get the error message, if any.
		 * @return Error message.
		 */
		[[nodiscard]] std::string_view GetError() noexcept { return !_function && _errorCode ? _errorCode : ""; }

	private:
		std::weak_ptr<asmjit::JitRuntime> _rt;
		MemAddr _function;
		union {
			MemAddr _userData;
			const char* _errorCode{};
		};
	};
} // namespace plugify
