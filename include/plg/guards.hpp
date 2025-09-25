#pragma once

#include "plg/config.hpp"

namespace plg {
#if PLUGIFY_HAS_EXCEPTIONS
	template <class Rollback>
	struct exception_guard_exceptions {
		exception_guard_exceptions() = delete;

		constexpr explicit exception_guard_exceptions(Rollback rollback)
			: _rollback(std::move(rollback))
			, _completed(false) {
		}

		constexpr exception_guard_exceptions(
			exception_guard_exceptions&& other
		) noexcept(std::is_nothrow_move_constructible_v<Rollback>)
			: _rollback(std::move(other._rollback))
			, _completed(other._completed) {
			other._completed = true;
		}

		exception_guard_exceptions(const exception_guard_exceptions&) = delete;
		exception_guard_exceptions& operator=(const exception_guard_exceptions&) = delete;
		exception_guard_exceptions& operator=(exception_guard_exceptions&&) = delete;

		constexpr void complete() noexcept {
			_completed = true;
		}

		constexpr ~exception_guard_exceptions() {
			if (!_completed) {
				_rollback();
			}
		}

	private:
		PLUGIFY_NO_UNIQUE_ADDRESS Rollback _rollback;
		bool _completed;
	};

	template <class Rollback>
	using exception_guard = exception_guard_exceptions<Rollback>;
#else
	template <class Rollback>
	struct exception_guard_noexceptions {
		exception_guard_noexceptions() = delete;

		constexpr explicit exception_guard_noexceptions(Rollback) {
		}

		constexpr exception_guard_noexceptions(
			exception_guard_noexceptions&& other
		) noexcept(std::is_nothrow_move_constructible_v<Rollback>)
			: _completed(other._completed) {
			other._completed = true;
		}

		exception_guard_noexceptions(const exception_guard_noexceptions&) = delete;
		exception_guard_noexceptions& operator=(const exception_guard_noexceptions&) = delete;
		exception_guard_noexceptions& operator=(exception_guard_noexceptions&&) = delete;

		constexpr void complete() noexcept {
			_completed = true;
		}

		constexpr ~exception_guard_noexceptions() {
			PLUGIFY_ASSERT(_completed, "exception_guard not completed with exceptions disabled");
		}

	private:
		bool _completed = false;
	};

	template <class Rollback>
	using exception_guard = exception_guard_noexceptions<Rollback>;
#endif

	template <class Rollback>
	constexpr exception_guard<Rollback> make_exception_guard(Rollback rollback) {
		return exception_guard<Rollback>(std::move(rollback));
	}

	template <class Func>
	class scope_guard {
		PLUGIFY_NO_UNIQUE_ADDRESS Func _func;

	public:
		constexpr explicit scope_guard(Func func)
			: _func(std::move(func)) {
		}

		constexpr ~scope_guard() {
			_func();
		}

		scope_guard(const scope_guard&) = delete;
		scope_guard& operator=(const scope_guard&) = delete;
		scope_guard& operator=(scope_guard&&) = delete;
		scope_guard(scope_guard&&) = delete;
	};

	template <class Func>
	constexpr scope_guard<Func> make_scope_guard(Func func) {
		return scope_guard<Func>(std::move(func));
	}
}
