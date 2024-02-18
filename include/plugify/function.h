#pragma once

#include <plugify/method.h>
#include <asmjit/asmjit.h>
#include <memory>
#include <functional>

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
		 * @brief Get the value of the argument at the specified index.
		 * @tparam T Type of the argument.
		 * @param idx Index of the argument.
		 * @return Value of the argument.
		 */
		template<typename T>
		T GetArgument(uint8_t idx) const {
			return *(T*) GetArgumentPtr(idx);
		}

		volatile uintptr_t arguments; ///< Raw storage for function arguments.

		/**
		 * @brief Get a pointer to the argument at the specified index.
		 * @param idx Index of the argument.
		 * @return Pointer to the argument.
		 */
		int8_t* GetArgumentPtr(uint8_t idx) const {
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
		uint8_t* GetReturnPtr() const {
			return (uint8_t*)&ret;
		}

		uintptr_t ret; ///< Raw storage for the return value.
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
		Function(Function&& other) noexcept = default;

		/**
		 * @brief Destructor.
		 */
		~Function();

		using FuncCallback = void(*)(const Method* method, void* data, const Parameters* params, uint8_t count, const ReturnValue* ret);
		using DoneCallback = std::function<void(void* data)>;

		/**
		 * @brief Get a dynamically created callback function based on the raw signature.
		 * @param sig Function signature.
		 * @param method Reference to the method.
		 * @param callback Callback function.
		 * @param data User data.
		 * @return Pointer to the generated function.
		 */
		void* GetJitFunc(const asmjit::FuncSignature& sig, const Method& method, FuncCallback callback, void* data = nullptr);

		/**
		 * @brief Get a dynamically created callback function using a typedef represented as a string.
		 * @param method Reference to the method.
		 * @param callback Callback function.
		 * @param data User data.
		 * @return Pointer to the generated function.
		 */
		void* GetJitFunc(const Method& method, FuncCallback callback, void* data = nullptr);

		/**
		 * @brief Get a dynamically created function.
		 * @return Pointer to the already generated function.
		 */
		void* GetFunction() const { return _function; }

		/**
		 * @brief Sets a callback function to be called when the class is destroy.
		 *
		 * @param doneCallback A function pointer with the signature `void callback(void* userData)`.
		 *                     This function will be invoked upon class destroy.
		 *
		 * @note The provided callback should handle any necessary cleanup or post-processing.
		 * @noreturn
		 */
		void SetDoneCallback(DoneCallback doneCallback) { _doneCallback = std::make_unique<DoneCallback>(std::move(doneCallback)); }

		/**
		 * @brief Get the error message, if any.
		 * @return Error message.
		 */
		const std::string& GetError() { return _error; }

	private:
		static asmjit::CallConvId GetCallConv(const std::string& conv);
		static asmjit::TypeId GetTypeId(ValueType type);

		static bool IsGeneralReg(asmjit::TypeId typeId) ;
		static bool IsXmmReg(asmjit::TypeId typeId) ;

	private:
		std::weak_ptr<asmjit::JitRuntime> _rt;
		std::unique_ptr<DoneCallback> _doneCallback;
		void* _function{ nullptr };
		void* _userData{ nullptr };
		std::string _error;
	};
} // namespace plugify