#pragma once

#include "plg/config.hpp"

namespace plg {
#if PLUGIFY_HAS_EXCEPTIONS
	template <class Rollback>
	struct exception_guard_exceptions {
		exception_guard_exceptions() = delete;

		constexpr explicit exception_guard_exceptions(Rollback rollback)
			: rollback_(std::move(rollback))
			, completed_(false) {
		}

		constexpr exception_guard_exceptions(
			exception_guard_exceptions&& other
		) noexcept(std::is_nothrow_move_constructible_v<Rollback>)
			: rollback_(std::move(other.rollback_))
			, completed_(other.completed_) {
			other.completed_ = true;
		}

		exception_guard_exceptions(const exception_guard_exceptions&) = delete;
		exception_guard_exceptions& operator=(const exception_guard_exceptions&) = delete;
		exception_guard_exceptions& operator=(exception_guard_exceptions&&) = delete;

		constexpr void complete() noexcept {
			completed_ = true;
		}

		constexpr ~exception_guard_exceptions() {
			if (!completed_) {
				rollback_();
			}
		}

	private:
		PLUGIFY_NO_UNIQUE_ADDRESS Rollback rollback_;
		bool completed_;
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
			: completed_(other.completed_) {
			other.completed_ = true;
		}

		exception_guard_noexceptions(const exception_guard_noexceptions&) = delete;
		exception_guard_noexceptions& operator=(const exception_guard_noexceptions&) = delete;
		exception_guard_noexceptions& operator=(exception_guard_noexceptions&&) = delete;

		constexpr void complete() noexcept {
			completed_ = true;
		}

		constexpr ~exception_guard_noexceptions() {
			PLUGIFY_ASSERT(completed_, "exception_guard not completed with exceptions disabled");
		}

	private:
		bool completed_ = false;
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
		PLUGIFY_NO_UNIQUE_ADDRESS Func func_;

	public:
		constexpr explicit scope_guard(Func func)
			: func_(std::move(func)) {
		}

		constexpr ~scope_guard() {
			func_();
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
