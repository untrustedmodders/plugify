#pragma once

#include <cstddef>
#include <cstdint>
#include <bit>
#include <memory>
#include <type_traits>
#include <span>
#include <array>
#include <concepts>

#include "plugify/global.h"
#include "plugify/mem_addr.hpp"
#include "plugify/method.hpp"
#include "plugify/property.hpp"
#include "plugify/signarure.hpp"
#include "plugify/value_type.hpp"

namespace plugify {
    // Concept to ensure type fits in storage and is trivially copyable
    template<typename T>
    concept StorableType = std::is_trivially_copyable_v<T> &&
                           sizeof(T) <= sizeof(uint64_t) &&
                           alignof(T) <= alignof(uint64_t);
	/**
	 * @class JitCallback
	 * @brief Class to create callback
	 * objects, that can be passed to functions as callback function pointers.
	 * In other words, a pointer to the callback object can be "called",
	 * directly. A generic callback handler invoked by this object then allows
	 * iterating dynamically over the arguments once called back.
	 */
	class PLUGIFY_API JitCallback {
		struct Impl;
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
         * @class Parameters
         * @brief Modern wrapper for function parameters stored in a stack-allocated array.
         *
         * This class provides type-safe access to parameters stored in a contiguous
         * memory region, typically created by asmjit on the stack.
         */
        class Parameters {
        public:
            // Size of each parameter slot in bytes
            static constexpr size_t SlotSize = sizeof(uint64_t);

            /**
             * @brief Set the value of an argument at the specified index.
             * @tparam T Type of the argument (must be trivially copyable and ≤ 64 bits).
             * @param idx Index of the argument slot.
             * @param val Value to set.
             */
            template<StorableType T>
            void SetArgument(size_t idx, T val) noexcept {
                auto* ptr = GetSlotAs<T>(idx);
                *ptr = val;
            }

            /**
             * @brief Set a value in an array pointed to by the argument at the specified index.
             * @tparam T Type of the array element.
             * @param idx Index of the argument containing the array pointer.
             * @param val Value to set.
             * @param array_idx Position within the array (default: 0).
             */
            template<typename T>
            void SetArgumentArrayElement(size_t idx, T val, size_t array_idx = 0) noexcept {
                auto* array_ptr = GetArgument<T*>(idx);
                if (array_ptr) {
                    array_ptr[array_idx] = val;
                }
            }

            /**
             * @brief Get the value of an argument at the specified index.
             * @tparam T Type of the argument.
             * @param idx Index of the argument.
             * @return Value of the argument.
             */
            template<StorableType T>
            [[nodiscard]] T GetArgument(size_t idx) const noexcept {
                return *GetSlotAs<T>(idx);
            }

            /**
             * @brief Get a span view of the argument slots.
             * @param count Number of slots to include in the span.
             * @return Span of argument slots as bytes.
             */
            [[nodiscard]] std::span<std::byte> GetArgumentsSpan(size_t count) noexcept {
                return std::span<std::byte>(GetRawStorage(), count * SlotSize);
            }

            /**
             * @brief Get a const span view of the argument slots.
             * @param count Number of slots to include in the span.
             * @return Const span of argument slots as bytes.
             */
            [[nodiscard]] std::span<const std::byte> GetArgumentsSpan(size_t count) const noexcept {
                return std::span<const std::byte>(GetRawStorage(), count * SlotSize);
            }

        protected:
            /**
             * @brief Get a typed pointer to the slot at the specified index.
             * @tparam T Type to interpret the slot as.
             * @param idx Index of the slot.
             * @return Pointer to the slot interpreted as type T.
             */
            template<typename T>
            [[nodiscard]] T* GetSlotAs(size_t idx) noexcept {
                auto* slot_ptr = GetRawStorage() + (idx * SlotSize);
                return std::launder(reinterpret_cast<T*>(slot_ptr));
            }

            template<typename T>
            [[nodiscard]] const T* GetSlotAs(size_t idx) const noexcept {
                auto* slot_ptr = GetRawStorage() + (idx * SlotSize);
                return std::launder(reinterpret_cast<const T*>(slot_ptr));
            }

            /**
             * @brief Get raw storage pointer.
             * @return Pointer to the beginning of the storage.
             */
            [[nodiscard]] std::byte* GetRawStorage() noexcept {
                return reinterpret_cast<std::byte*>(&_storage);
            }

            [[nodiscard]] const std::byte* GetRawStorage() const noexcept {
                return reinterpret_cast<const std::byte*>(&_storage);
            }

        private:
            // Using alignas to ensure proper alignment for any type up to uint64_t
            alignas(alignof(uint64_t)) uint64_t _storage;  // First slot of the arguments array
        };

        /**
         * @class Return
         * @brief Modern wrapper for function return values.
         *
         * This class provides type-safe access to return value storage,
         * supporting both primitive types and in-place construction.
         */
        class Return {
        public:
            /**
             * @brief Construct an object of type T in the return value storage.
             * @tparam T Type to construct (must be trivially constructible).
             * @param args Arguments to forward to the constructor.
             */
            template<typename T, typename... Args>
                requires std::is_trivially_destructible_v<T> && (sizeof(T) <= sizeof(uint64_t))
            void Construct(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
                std::construct_at(GetAs<T>(), std::forward<Args>(args)...);
            }

            /**
             * @brief Set the return value.
             * @tparam T Type of the return value.
             * @param val Value to set.
             */
            template<StorableType T>
            void Set(T val) noexcept {
                if constexpr (sizeof(T) == sizeof(uint64_t) && alignof(T) == alignof(uint64_t)) {
                    // For types that exactly match our storage, use bit_cast for optimal codegen
                    _storage = std::bit_cast<uint64_t>(val);
                } else {
                    *GetAs<T>() = val;
                }
            }

            /**
             * @brief Get the return value.
             * @tparam T Type of the return value.
             * @return The return value.
             */
            template<StorableType T>
            [[nodiscard]] T Get() const noexcept {
                if constexpr (sizeof(T) == sizeof(uint64_t) && alignof(T) == alignof(uint64_t)) {
                    return std::bit_cast<T>(_storage);
                } else {
                    return *GetAs<T>();
                }
            }

            /**
             * @brief Get a reference to the return value.
             * @tparam T Type of the return value.
             * @return Reference to the return value.
             */
            template<typename T>
            [[nodiscard]] T& GetRef() noexcept {
                return *GetAs<T>();
            }

            template<typename T>
            [[nodiscard]] const T& GetRef() const noexcept {
                return *GetAs<T>();
            }

            /**
             * @brief Get raw pointer to the return value storage.
             * @return Pointer to the storage as bytes.
             */
            [[nodiscard]] std::byte* Data() noexcept {
                return reinterpret_cast<std::byte*>(&_storage);
            }

            [[nodiscard]] const std::byte* Data() const noexcept {
                return reinterpret_cast<const std::byte*>(&_storage);
            }

        protected:
            template<typename T>
            [[nodiscard]] T* GetAs() noexcept {
                return std::launder(reinterpret_cast<T*>(&_storage));
            }

            template<typename T>
            [[nodiscard]] const T* GetAs() const noexcept {
                return std::launder(reinterpret_cast<const T*>(&_storage));
            }

        private:
            alignas(alignof(uint64_t)) uint64_t _storage{};  // Zero-initialized by default
        };

        /**
         * @class ExtendedParameters
         * @brief Extended parameter wrapper with bounds checking in debug mode.
         *
         * This version adds optional bounds checking and more utility methods.
         */
        template<size_t MaxArgs = 64>
        class ExtendedParameters : public Parameters {
        public:
            explicit ExtendedParameters(size_t argCount = 0) noexcept
                : _argCount(argCount) {}

            template<StorableType T>
            void SetArgument(size_t idx, T val) noexcept {
                #ifndef NDEBUG
                if (idx >= _argCount) [[unlikely]] {
                    // Handle error in debug mode (e.g., assert or log)
                    return;
                }
                #endif
                Parameters::SetArgument(idx, val);
            }

            template<StorableType T>
            [[nodiscard]] T GetArgument(size_t idx) const noexcept {
                #ifndef NDEBUG
                if (idx >= _argCount) [[unlikely]] {
                    return T{};  // Return default value in debug mode
                }
                #endif
                return Parameters::GetArgument<T>(idx);
            }

            /**
             * @brief Get the number of arguments.
             * @return Number of arguments.
             */
            [[nodiscard]] size_t Size() const noexcept { return _argCount; }

            /**
             * @brief Check if an index is valid.
             * @param idx Index to check.
             * @return True if the index is within bounds.
             */
            [[nodiscard]] bool IsValidIndex(size_t idx) const noexcept {
                return idx < _argCount;
            }

        private:
            size_t _argCount{};
        };

		using CallbackHandler = void(*)(const Method& method, MemAddr data, const Parameters* params, size_t count, const Return* ret);
		using HiddenParam = bool(*)(ValueType);

		/**
		 * @brief Get a dynamically created callback function based on the raw signature.
		 * @param signature Function signature.
		 * @param method Method description.
		 * @param callback Callback function.
		 * @param data User data.
		 * @param hidden If true, return will be pass as hidden argument.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(const Signature& signature, const Method& method, CallbackHandler callback, MemAddr data, bool hidden);

		/**
		 * @brief Get a dynamically created function based on the method.
		 * @param method HMethod description.
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
		MemAddr GetJitFunc(const Method& method, CallbackHandler callback, MemAddr data = nullptr, HiddenParam hidden = &ValueUtils::IsHiddenParam);

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

	PLUGIFY_ACCESS:
	    PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
} // namespace plugify