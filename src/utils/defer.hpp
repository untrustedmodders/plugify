#pragma once

#include <type_traits>
#include <utility>

namespace plugify {
	template<typename F>
	struct defer_t {
		F f;

		defer_t(F&& arg) noexcept(std::is_nothrow_move_constructible_v<F>)
			: f(std::move(arg)) {}

		~defer_t() noexcept(std::is_nothrow_invocable_v<F>) {
			f();
		}
	};

	template <typename F>
	defer_t(F f) -> defer_t<std::decay_t<F>>;
}

#define PLUGIFY_DEFER_TOK_CONCAT(X, Y) X ## Y
#define PLUGIFY_DEFER_TOK_PASTE(X, Y) PLUGIFY_DEFER_TOK_CONCAT(X, Y)
#define defer plugify::defer_t PLUGIFY_DEFER_TOK_PASTE(__scoped_defer_obj, __COUNTER__) = [&]()
