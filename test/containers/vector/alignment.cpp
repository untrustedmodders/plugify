#include <plg/vector.hpp>

#include <array>

namespace {

template <size_t Alignment, size_t Size>
struct A {
	alignas(Alignment) std::array<std::byte, Size> m_data{};
};

} // namespace

static_assert(std::alignment_of_v<A<8, 1>> == 8);
static_assert(std::alignment_of_v<A<4, 1>> == 4);
static_assert(std::alignment_of_v<A<2, 1>> == 2);
static_assert(std::alignment_of_v<A<1, 1>> == 1);

static_assert(sizeof(A<1, 1>) == 1);
static_assert(sizeof(A<1, 2>) == 2);
static_assert(sizeof(A<1, 3>) == 3);
static_assert(sizeof(A<1, 4>) == 4);

static_assert(sizeof(A<2, 1>) == 2);
static_assert(sizeof(A<2, 2>) == 2);
static_assert(sizeof(A<2, 3>) == 4);
static_assert(sizeof(A<2, 4>) == 4);

using V16 = plg::vector<A<16, 16>>;
static_assert(sizeof(V16) == 24);
