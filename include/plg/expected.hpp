#pragma once

#include <type_traits>
#include <exception>
#include <variant>

#include <plg/macro.hpp>

namespace plg {
#if PLUGIFY_EXCEPTIONS
	class bad_expected_access final : public std::exception {
		const char* message = ""; // llvm test requires a well formed what() on default init
	public :
	explicit bad_expected_access(const char* str) noexcept : message{str} {}
		bad_expected_access() noexcept = default;
		bad_expected_access(const bad_expected_access&) noexcept = default;
		bad_expected_access& operator=(const bad_expected_access&) noexcept = default;
		const char* what() const noexcept override { return message; }
	};
#endif // PLUGIFY_EXCEPTIONS

	// ==================== Generic expected<T, E> ====================
	template<typename T, typename E>
	class expected {
	public:
		struct in_place_value_t {
			explicit in_place_value_t() = default;
		};
		struct in_place_error_t {
			explicit in_place_error_t() = default;
		};
		static constexpr in_place_value_t in_place_value{};
		static constexpr in_place_error_t in_place_error{};

		constexpr expected()
			requires std::is_default_constructible_v<T>
			: _data(T{}) {}

		constexpr expected(const T& value) : _data(value) {}
		constexpr expected(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
			: _data(std::move(value)) {}

		template<typename... Args>
		constexpr expected(in_place_value_t, Args &&...args)
			: _data(std::in_place_type<T>, std::forward<Args>(args)...) {}

		constexpr expected(const E& error) : _data(error) {}
		constexpr expected(E&& error) noexcept(std::is_nothrow_move_constructible_v<E>)
			: _data(std::move(error)) {}

		template<typename... Args>
		constexpr expected(in_place_error_t, Args &&...args)
			: _data(std::in_place_type<E>, std::forward<Args>(args)...) {}

		[[nodiscard]] constexpr bool has_value() const noexcept {
			return std::holds_alternative<T>(_data);
		}
		constexpr explicit operator bool() const noexcept { return has_value(); }

		constexpr T& operator*() & { return value(); }
		constexpr const T& operator*() const& { return value(); }
		constexpr T&& operator*() && { return std::move(value()); }

		constexpr T *operator->() { return &value(); }
		constexpr const T *operator->() const { return &value(); }

		constexpr T& value() & {
			PLUGIFY_ASSERT(has_value(), "plg::expected:value(): Bad expected access.", bad_expected_access);
			return std::get<T>(_data);
		}
		constexpr const T& value() const& {
			PLUGIFY_ASSERT(has_value(), "plg::expected:value(): Bad expected access.", bad_expected_access);
			return std::get<T>(_data);
		}
		constexpr T&& value() && {
			PLUGIFY_ASSERT(has_value(), "plg::expected:value(): Bad expected access.", bad_expected_access);
			return std::move(std::get<T>(_data));
		}

		constexpr E& error() & {
			PLUGIFY_ASSERT(!has_value(), "plg::expected:error(): Bad expected access.", bad_expected_access);
			return std::get<E>(_data);
		}
		constexpr const E& error() const& {
			PLUGIFY_ASSERT(!has_value(), "plg::expected:error(): Bad expected access.", bad_expected_access);
			return std::get<E>(_data);
		}

	private:
		std::variant<T, E> _data;
	};

	// ==================== Specialization for expected<void, E> ====================
	template<typename E>
	class expected<void, E> {
	public:
		struct in_place_value_t {
			explicit in_place_value_t() = default;
		};
		struct in_place_error_t {
			explicit in_place_error_t() = default;
		};
		static constexpr in_place_value_t in_place_value{};
		static constexpr in_place_error_t in_place_error{};

		constexpr expected() noexcept : _hasValue(true) {}
		constexpr expected(in_place_value_t) noexcept : _hasValue(true) {}

		constexpr expected(const E& error) : _hasValue(false), _error(error) {}
		constexpr expected(E&& error) noexcept(std::is_nothrow_move_constructible_v<E>)
			: _hasValue(false), _error(std::move(error)) {}

		template<typename... Args>
		constexpr expected(in_place_error_t, Args &&...args)
			: _hasValue(false), _error(std::forward<Args>(args)...) {}

		[[nodiscard]] constexpr bool has_value() const noexcept { return _hasValue; }
		constexpr explicit operator bool() const noexcept { return _hasValue; }

		constexpr void value() const {
			PLUGIFY_ASSERT(has_value(), "plg::expected:value(): Bad expected access.", bad_expected_access);
		}

		constexpr E& error() & {
			PLUGIFY_ASSERT(!has_value(), "plg::expected:error(): Bad expected access.", bad_expected_access);
			return _error;
		}
		constexpr const E& error() const& {
			PLUGIFY_ASSERT(!has_value(), "plg::expected:error(): Bad expected access.", bad_expected_access);
			return _error;
		}

	private:
		bool _hasValue;
		E _error{};
	};
} // namespace plg
