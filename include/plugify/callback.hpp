#pragma once

#include <memory>
#include <string>
#include <utility>

#include "plugify/global.h"
#include "plugify/mem_addr.hpp"
#include "plugify/method.hpp"
#include "plugify/property.hpp"
#include "plugify/signarure.hpp"
#include "plugify/value_type.hpp"
#include "plg/inplace_vector.hpp"

namespace plugify {
	/**
	 * @class JitCallback
	 * @brief Class to create callback
	 * objects, that can be passed to functions as callback function pointers.
	 * In other words, a pointer to the callback object can be "called",
	 * directly. A generic callback handler invoked by this object then allows
	 * iterating dynamically over the arguments once called back.
	 */
	class PLUGIFY_API JitCallback {
	public:
		/**
		 * @brief Constructor.
		 */
		JitCallback();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of JitCallback.
		 */
		JitCallback(const JitCallback& other) = delete;

		/**
		 * @brief Move constructor.
		 * @param other Another instance of JitCallback.
		 */
		JitCallback(JitCallback&& other) noexcept;

		/**
		 * @brief Destructor.
		 */
		~JitCallback();

		/**
		 * @brief CallbackHandler is a function pointer type for generic callback invocation.
		 *        The \p params argument can be accessed safely and conveniently using
		 * ParametersSpan, and the \p ret argument (return value storage) can be managed using
		 * ReturnSlot. These helper classes provide type-safe and ergonomic access to arguments and
		 * return values.
		 */
		using CallbackHandler = void (*)(
		    const Method* method,
		    MemAddr data,
		    uint64_t* params,
		    size_t count,
		    /*uint128_t*/ void* ret
		);

		/**
		 * @brief HiddenParam is a predicate function pointer to determine if a ValueType should be
		 * passed as a hidden parameter. Use for return structs on x86 and arm arch.
		 */
		using HiddenParam = bool (*)(ValueType);

		/**
		 * @brief Generates a JIT-compiled callback function based on the provided raw signature.
		 * @param signature The function signature to use for the generated callback.
		 * @param method Optional pointer to a method descriptor for additional context. May be
		 * nullptr if not needed.
		 * @param callback Pointer to the callback handler to invoke.
		 * @param data User data to be passed to the callback handler.
		 * @param hidden If true, the return value will be passed as a hidden argument.
		 * @return Pointer to the generated function, or nullptr if generation fails.
		 * @details The \p method pointer can be nullptr and is provided for context in addition to
		 * user data, as it is common to access both in callback scenarios.
		 */
		MemAddr GetJitFunc(
		    const Signature& signature,
		    const Method* method,
		    CallbackHandler callback,
		    MemAddr data,
		    bool hidden
		);

		/**
		 * @brief Generates a callback function matching the specified method signature.
		 * @param method The method descriptor defining the function signature.
		 * @param callback Pointer to the generic callback handler to invoke.
		 * @param data Optional user data passed to the callback handler.
		 * @param hidden Predicate to determine if the return value should be passed as a hidden
		 * argument.
		 * @return Pointer to the generated function, or nullptr if generation fails.
		 *
		 * @details This function creates a JIT-compiled callback based on the provided method,
		 * allowing dynamic invocation with runtime argument handling. The callback handler
		 * receives arguments and user data as specified. If the hidden predicate returns true,
		 * the return value is passed as an additional hidden parameter.
		 */
		MemAddr GetJitFunc(
		    const Method& method,
		    CallbackHandler callback,
		    MemAddr data = nullptr,
		    HiddenParam hidden = &ValueUtils::IsHiddenParam
		);

		/**
		 * @brief Get a dynamically created function.
		 * @return Pointer to the already generated function.
		 * @note The returned pointer can be nullptr if function is not generate.
		 */
		MemAddr GetFunction() const noexcept;

		/**
		 * @brief Get the user data associated with the object.
		 * @details This function returns a pointer to the user data associated with the object.
		 * @return A void pointer to the user data.
		 * @note The returned pointer can be nullptr if no user data is set.
		 */
		MemAddr GetUserData() const noexcept;

		/**
		 * @brief Get the error message, if any.
		 * @return Error message.
		 */
		std::string_view GetError() noexcept;

		/**
		 * @brief Copy assignment operator for JitCallback.
		 *
		 * @param other The other JitCallback instance to copy from.
		 * @return A reference to this instance after copying.
		 */
		JitCallback& operator=(const JitCallback& other) = delete;

		/**
		 * @brief Move assignment operator for JitCall.
		 *
		 * @param other The other JitCallback instance to move from.
		 * @return A reference to this instance after moving.
		 * @note This operator is marked noexcept to indicate it does not throw exceptions.
		 */
		JitCallback& operator=(JitCallback&& other) noexcept;

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

	/**
	 * @brief Concept for types that can be stored directly in a register slot
	 */
	template <typename T>
	concept SlotStorable = std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(uint64_t);

	/**
	 * @class ParametersSpan
	 * @brief Provides access to a span of parameters stored in register-sized slots.
	 *
	 * This class allows reading and writing values of trivially copyable types
	 * to a contiguous array of register-sized slots, typically used for passing
	 * function arguments in JIT or ABI contexts.
	 */
	class ParametersSpan {
	public:
		using slot_type = uint64_t;

		/**
		 * @brief Constructs a ParametersSpan.
		 * @param data Pointer to the slot data.
		 * @param count Number of slots.
		 */
		ParametersSpan(slot_type* data, size_t count) noexcept
		    : _data(data)
		    , _count(count) {
		}

		/**
		 * @brief Get a value from the parameter span
		 * @tparam T Type to interpret the parameter as
		 * @param index Parameter index
		 * @return The value at the specified index
		 */
		template <typename T>
		    requires SlotStorable<T>
		T Get(size_t index) const noexcept {
			assert(index < _count && "Index out of bounds");
			T result{};
			std::memcpy(&result, &_data[index], sizeof(T));
			return result;
		}

		/**
		 * @brief Set a value in the parameter span
		 * @tparam T Type of the value to set
		 * @param index Parameter index
		 * @param value Value to set
		 */
		template <typename T>
		    requires SlotStorable<T>
		void Set(size_t index, const T& value) noexcept {
			assert(index < _count && "Index out of bounds");
			std::memcpy(&_data[index], &value, sizeof(T));
		}

	private:
		slot_type* _data;
		size_t _count;
	};

	/**
	 * @class ReturnSlot
	 * @brief Provides a storage area for return values in JIT/ABI contexts.
	 *
	 * This class manages a memory region for return values, allowing safe
	 * reading, writing, and in-place construction of trivially copyable types.
	 * It is typically used to handle return values from dynamically generated functions.
	 */
	class ReturnSlot {
	public:
		using type = void;

		/**
		 * @brief Construct from byte storage
		 * @param data Pointer to aligned byte storage
		 * @param size Size in bytes
		 */
		explicit ReturnSlot(type* data, size_t size) noexcept
		    : _data(data)
		    , _size(size) {
		}

		/**
		 * @brief Set the return value
		 * @tparam T Type of the return value
		 * @param value Value to return
		 */
		template <typename T>
		    requires std::is_trivially_copyable_v<T>
		void Set(const T& value) noexcept {
			assert(sizeof(T) <= _size && "Return value too large");
			std::memcpy(_data, &value, sizeof(T));
		}

		/**
		 * @brief Get the return value
		 * @tparam T Type of the return value
		 * @return The return value
		 */
		template <typename T>
		    requires std::is_trivially_copyable_v<T>
		T Get() const noexcept {
			assert(sizeof(T) <= _size && "Return type too large");
			T result{};
			std::memcpy(&result, _data, sizeof(T));
			return result;
		}

		/**
		 * @brief Construct an object in-place
		 * @tparam T Type to construct
		 * @tparam Args Constructor argument types
		 * @param args Constructor arguments
		 */
		template <typename T, typename... Args>
			// requires std::is_trivially_destructible_v<T>
		void Construct(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
			assert(sizeof(T) <= _size && "Type too large");
			std::construct_at(reinterpret_cast<T*>(_data), std::forward<Args>(args)...);
		}

	private:
		type* _data;
		size_t _size;
	};
}  // namespace plugify