#pragma once

#include <type_traits>
#include <utility>

namespace plugify {
    template<typename F>
    struct Defer {
        F f;

        Defer(F&& arg) noexcept(std::is_nothrow_move_constructible_v<F>)
            : f(std::move(arg)) {}

        ~Defer() noexcept(std::is_nothrow_invocable_v<F>) {
            f();
        }
    };

    template <typename F>
    Defer(F f) -> Defer<std::decay_t<F>>;
}

#define PLUGIFY_DEFER_TOK_CONCAT(X, Y) X ## Y
#define PLUGIFY_DEFER_TOK_PASTE(X, Y) PLUGIFY_DEFER_TOK_CONCAT(X, Y)
#define defer plugify::Defer PLUGIFY_DEFER_TOK_PASTE(__scoped_defer_obj, __COUNTER__) = [&]()