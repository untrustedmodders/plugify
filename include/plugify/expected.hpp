#pragma once

namespace plg {
	template <typename E>
	class bad_expected_access : public std::exception {
	public:
		explicit bad_expected_access(E err) : _error(std::move(err)) {}
		const char *what() const noexcept override { return "bad expected access"; }
		const E &error() const & noexcept { return _error; }
		E &error() & noexcept { return _error; }

	private:
		E _error;
	};

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

		constexpr expected(const T &value) : _data(value) {}
		constexpr expected(T &&value) noexcept(std::is_nothrow_move_constructible_v<T>)
			: _data(std::move(value)) {}

		template<typename... Args>
		constexpr expected(in_place_value_t, Args &&...args)
			: _data(std::in_place_type<T>, std::forward<Args>(args)...) {}

		constexpr expected(const E &error) : _data(error) {}
		constexpr expected(E &&error) noexcept(std::is_nothrow_move_constructible_v<E>)
			: _data(std::move(error)) {}

		template<typename... Args>
		constexpr expected(in_place_error_t, Args &&...args)
			: _data(std::in_place_type<E>, std::forward<Args>(args)...) {}

		[[nodiscard]] constexpr bool has_value() const noexcept {
			return std::holds_alternative<T>(_data);
		}
		constexpr explicit operator bool() const noexcept { return has_value(); }

		constexpr T &operator*() & { return value(); }
		constexpr const T &operator*() const & { return value(); }
		constexpr T &&operator*() && { return std::move(value()); }

		constexpr T *operator->() { return &value(); }
		constexpr const T *operator->() const { return &value(); }

		constexpr T &value() & {
			if (!has_value()) throw bad_expected_access<E>(error());
			return std::get<T>(_data);
		}
		constexpr const T &value() const & {
			if (!has_value()) throw bad_expected_access<E>(error());
			return std::get<T>(_data);
		}
		constexpr T &&value() && {
			if (!has_value()) throw bad_expected_access<E>(error());
			return std::move(std::get<T>(_data));
		}

		constexpr E &error() & {
			if (has_value()) throw std::logic_error("expected has value, not error");
			return std::get<E>(_data);
		}
		constexpr const E &error() const & {
			if (has_value()) throw std::logic_error("expected has value, not error");
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

		constexpr expected(const E &error) : _hasValue(false), _error(error) {}
		constexpr expected(E &&error) noexcept(std::is_nothrow_move_constructible_v<E>)
			: _hasValue(false), _error(std::move(error)) {}

		template<typename... Args>
		constexpr expected(in_place_error_t, Args &&...args)
			: _hasValue(false), _error(std::forward<Args>(args)...) {}

		[[nodiscard]] constexpr bool has_value() const noexcept { return _hasValue; }
		constexpr explicit operator bool() const noexcept { return _hasValue; }

		constexpr void value() const {
			if (!_hasValue) throw bad_expected_access<E>(_error);
		}

		constexpr E &error() & {
			if (_hasValue) throw std::logic_error("expected has value, not error");
			return _error;
		}
		constexpr const E &error() const & {
			if (_hasValue) throw std::logic_error("expected has value, not error");
			return _error;
		}

	private:
		bool _hasValue;
		E _error{};
	};
}