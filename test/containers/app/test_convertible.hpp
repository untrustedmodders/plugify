//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <type_traits>

namespace detail {
    template <class Tp> void eat_type(Tp);

    template <class Tp, class ...Args>
    constexpr auto test_convertible_imp(int)
        -> decltype(eat_type<Tp>({std::declval<Args>()...}), true)
    { return true; }

    template <class Tp, class ...Args>
    constexpr auto test_convertible_imp(long) -> bool { return false; }
}

template <class Tp, class ...Args>
constexpr bool test_convertible()
{ return detail::test_convertible_imp<Tp, Args...>(0); }