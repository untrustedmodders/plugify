#include "nat.hpp"

std::size_t ctr = 0;
std::size_t cpy = 0;
std::size_t cpy_assign = 0;
std::size_t mv = 0;
std::size_t mv_assign = 0;
std::size_t dtr = 0;

// nat

nat::nat()
: cnt{static_cast<signed long long>(++ctr)} {
}

nat::nat(signed long long c0)
: cnt{c0} {
    ++ctr;
}
nat::nat(signed long long c0, signed long long c1)
: cnt{c0 + c1} {
    ++ctr;
}
nat::nat(nat const& other) noexcept
: cnt{other.cnt} {
    ++cpy;
}
nat& nat::operator=(nat const& other) noexcept {
    cnt = other.cnt;
    ++cpy_assign;
    return *this;
}
nat::nat(nat&& other) noexcept
: cnt{other.cnt} {
    ++mv;
}
nat& nat::operator=(nat&& other) noexcept {
    cnt = other.cnt;
    ++mv_assign;
    return *this;
}
nat::~nat() {
    ++dtr;
    cnt = ~0ll;
}

// nat_no_move

nat_no_move::nat_no_move()
: cnt{++ctr} {
}
nat_no_move::nat_no_move(nat_no_move const& other) noexcept
: cnt{other.cnt} {
    ++cpy;
}
nat_no_move& nat_no_move::operator=(nat_no_move const& other) noexcept {
    cnt = other.cnt;
    ++cpy_assign;
    return *this;
}
nat_no_move::~nat_no_move() {
    ++dtr;
    cnt = ~0ul;
}

// nat_no_default_ctr

nat_no_default_ctr::nat_no_default_ctr(nat_no_default_ctr const& other) noexcept
: cnt{other.cnt} {
    ++cpy;
}
nat_no_default_ctr& nat_no_default_ctr::operator=(nat_no_default_ctr const& other) noexcept {
    cnt = other.cnt;
    ++cpy_assign;
    return *this;
}
nat_no_default_ctr::~nat_no_default_ctr() {
    ++dtr;
    cnt = ~0ul;
}

void reset_static_nat_counter() noexcept {
    ctr = 0;
    cpy = 0;
    cpy_assign = 0;
    mv = 0;
    mv_assign = 0;
    dtr = 0;
}
