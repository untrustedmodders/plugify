#include <catch_amalgamated.hpp>

#include <plugifyvariant.hpp>

TEST_CASE("variant monostate", "[variant]") {
	SECTION("struct monostate {}") {
		using M = plg::monostate;
		static_assert(std::is_trivially_default_constructible<M>::value);
		static_assert(std::is_trivially_copy_constructible<M>::value);
		static_assert(std::is_trivially_copy_assignable<M>::value);
		static_assert(std::is_trivially_destructible<M>::value);
		constexpr M m{};
		((void) m);
	}
}

