#pragma once

#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>

#include "plugify/global.h"
#include "plugify/mem_addr.hpp"
#include "plugify/method.hpp"
#include "plugify/property.hpp"
#include "plugify/signarure.hpp"
#include "plugify/value_type.hpp"
#include "plg/inplace_vector.hpp"

namespace plugify {
	class Return;

	/**
	 * @class JitCall
	 * @brief Class encapsulates architecture-, OS- and compiler-specific
	 * function call semantics in a virtual "bind argument parameters from left
	 * to right and then call" interface allowing programmers to call C
	 * functions in a completely dynamic manner.
	 * In other words, instead of calling a function directly, class
	 * provides a mechanism to push the function parameters manually and to issue the call
	 * afterwards.
	 */
	class PLUGIFY_API JitCall {
	public:
		/**
		 * @brief Constructor.
		 */
		JitCall();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of JitCall.
		 */
		JitCall(const JitCall& other) = delete;

		/**
		 * @brief Move constructor.
		 * @param other Another instance of JitCall.
		 */
		JitCall(JitCall&& other) noexcept;

		/**
		 * @brief Destructor.
		 */
		~JitCall();

		enum class WaitType { None, Breakpoint, Wait_Keypress };


		using CallingFunc = void (*)(const uint64_t* params, const Return*);  // Return can be null

		/**
		 * @brief HiddenParam is a predicate function pointer to determine if a ValueType should be
		 * passed as a hidden parameter. Use for return structs on x86 and arm arch.
		 */
		using HiddenParam = bool (*)(ValueType);

		/**
		 * @brief Get a dynamically created function based on the raw signature.
		 * @param sig Function signature.
		 * @param target Target function to call.
		 * @param waitType Optionally insert a breakpoint before the call.
		 * @param hidden If true, return will be pass as hidden argument.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(const Signature& sig, MemAddr target, WaitType waitType, bool hidden);

		/**
		 * @brief Get a dynamically created function based on the method reference.
		 * @param method Reference to the method.
		 * @param target Target function to call.
		 * @param waitType Optionally insert a breakpoint before the call.
		 * @param hidden If true, return will be pass as hidden argument.
		 * @return Pointer to the generated function.
		 */
		MemAddr GetJitFunc(
		    const Method& method,
		    MemAddr target,
		    WaitType waitType = WaitType::None,
		    HiddenParam hidden = &ValueUtils::IsHiddenParam
		);

		/**
		 * @brief Get a dynamically created function.
		 * @return Pointer to the already generated function.
		 * @note The returned pointer can be nullptr if function is not generate.
		 */
		MemAddr GetFunction() const noexcept;

		/**
		 * @brief Get the target associated with the object.
		 * @details This function returns a pointer to the target function associated with the
		 * object.
		 * @return A void pointer to the target function.
		 * @note The returned pointer can be nullptr if no target is set.
		 */
		MemAddr GetTargetFunc() const noexcept;

		/**
		 * @brief Get the error message, if any.
		 * @return Error message.
		 */
		std::string_view GetError() noexcept;

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

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

	// Concept for types that can be stored in parameter slots
	template <typename T>
	concept ParameterType = std::is_trivially_copyable_v<T>
	                        && (std::is_arithmetic_v<T> || std::is_pointer_v<T> || std::is_enum_v<T>);

	// Concept for types that fit in a single slot
	template <typename T>
	concept SingleSlotType = ParameterType<T> && sizeof(T) <= sizeof(uint64_t);

	// Concept for types that need multiple slots
	template <typename T>
	concept MultiSlotType = ParameterType<T> && sizeof(T) > sizeof(uint64_t);

	/**
	 * @class Parameters
	 * @brief Builder for function parameters to be passed to JIT code.
	 *
	 * Supports various parameter types with automatic slot management.
	 */
	class Parameters {
	public:
		using SlotType = uint64_t;
		static constexpr size_t SlotSize = sizeof(SlotType);

		/**
		 * @brief Constructor.
		 * @param slotCount Number of parameter slots to allocate.
		 */
		explicit Parameters(size_t slotCount) {
			_storage.reserve(slotCount);
		}

		/**
		 * @brief Add an argument at the next available position.
		 * @tparam T Type of the argument.
		 * @param value Value to add.
		 * @return Reference to this builder for chaining.
		 */
		template <SingleSlotType T>
		Parameters& Add(T value) {
			SlotType& slot = _storage.emplace_back();
			std::memcpy(&slot, &value, sizeof(T));
			return *this;
		}

		/**
		 * @brief Add a large argument that spans multiple slots.
		 * @tparam T Type of the argument (must be trivially copyable).
		 * @param value Value to add.
		 * @return Reference to this builder for chaining.
		 */
		template <MultiSlotType T>
		Parameters& AddLarge(const T& value) {
			constexpr size_t slotsNeeded = (sizeof(T) + SlotSize - 1) / SlotSize;
			size_t oldSize = _storage.size();
			_storage.resize(oldSize + slotsNeeded);
			std::memcpy(&_storage[oldSize], &value, sizeof(T));
			return *this;
		}

		/**
		 * @brief Add multiple arguments at once.
		 * @tparam Args Types of the arguments.
		 * @param args Arguments to add.
		 * @return Reference to this builder for chaining.
		 */
		template <typename... Args>
		    requires(ParameterType<Args> && ...)
		Parameters& AddMultiple(Args... args) {
			(Add(args), ...);
			return *this;
		}

		/**
		 * @brief Set argument at specific position (overwrites if exists).
		 * @tparam T Type of the argument.
		 * @param index Slot index.
		 * @param value Value to set.
		 * @return Reference to this builder for chaining.
		 */
		template <SingleSlotType T>
		Parameters& SetAt(size_t index, T value) {
			SlotType& slot = _storage.at(index);
			slot = {};
			std::memcpy(&slot, &value, sizeof(T));
			return *this;
		}

		/**
		 * @brief Get pointer to the parameter data.
		 * @return Const pointer to the parameter storage.
		 */
		[[nodiscard]] const SlotType* Get() const noexcept {
			return _storage.data();
		}

		/**
		 * @brief Get a span view of the parameters.
		 * @return Span of parameter slots.
		 */
		[[nodiscard]] std::span<const SlotType> GetSpan() const noexcept {
			return { _storage.data(), _storage.size() };
		}

		/**
		 * @brief Get the number of slots used.
		 * @return Number of slots currently in use.
		 */
		[[nodiscard]] size_t GetUsedSlots() const noexcept {
			return _storage.size();
		}

		/**
		 * @brief Get the total capacity.
		 * @return Total number of slots allocated.
		 */
		[[nodiscard]] size_t GetCapacity() const noexcept {
			return _storage.capacity();
		}

		/**
		 * @brief Reset the builder to reuse it.
		 */
		void Reset() noexcept {
			_storage.clear();
		}

		/**
		 * @brief Check if there's room for more arguments.
		 * @param slotsNeeded Number of slots needed (default: 1).
		 * @return True if there's enough space.
		 */
		[[nodiscard]] bool HasSpace(size_t slotsNeeded = 1) const noexcept {
			return _storage.size() + slotsNeeded < _storage.capacity();
		}

	private:
		std::inplace_vector<SlotType, Signature::kMaxFuncArgs> _storage{};  ///< Fixed-capacity vector storage
	};

	/**
	 * @class Return
	 * @brief Wrapper for function return values.
	 *
	 * Supports return values up to 128 bits (2 slots).
	 */
	class Return {
	public:
		using SlotType = uint64_t;
		static constexpr size_t MaxSize = 2 * sizeof(SlotType);

		/**
		 * @brief Construct an object of type T in the return storage.
		 * @tparam T Type to construct.
		 * @param args Arguments to forward to the constructor.
		 */
		template <typename T, typename... Args>
		    requires(sizeof(T) <= MaxSize && std::is_trivially_destructible_v<T>)
		void Construct(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
			std::construct_at(reinterpret_cast<T*>(_storage.data()), std::forward<Args>(args)...);
		}

		/**
		 * @brief Set the return value.
		 * @tparam T Type of the return value.
		 * @param value Value to set.
		 */
		template <typename T>
		    requires(sizeof(T) <= MaxSize && std::is_trivially_copyable_v<T>)
		void Set(T value) noexcept {
			_storage = {};
			std::memcpy(_storage.data(), &value, sizeof(T));
		}

		/**
		 * @brief Get the return value.
		 * @tparam T Type of the return value.
		 * @return The return value.
		 */
		template <typename T>
		    requires(sizeof(T) <= MaxSize && std::is_trivially_copyable_v<T>)
		[[nodiscard]] T Get() const noexcept {
			T result;
			std::memcpy(&result, _storage.data(), sizeof(T));
			return result;
		}

		/**
		 * @brief Clear the return value storage.
		 */
		void Clear() noexcept {
			_storage = {};
		}

	private:
		alignas(alignof(std::max_align_t))
		std::array<std::byte, MaxSize> _storage{};  ///< 128-bit storage
	};
}  // namespace plugify