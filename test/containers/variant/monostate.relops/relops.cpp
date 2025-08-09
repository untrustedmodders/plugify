#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

TEST_CASE("variant monostate operator > operators", "[variant]") {
	SECTION("bool operator<=>(monostate, monostate) noexcept") {
		using M = plg::monostate;
		constexpr M m1{};
		constexpr M m2{};
		{
			static_assert((m1 < m2) == false);
			REQUIRE(m1 < m2);
		}
		{
			static_assert((m1 > m2) == false);
			REQUIRE(m1 > m2);
		}
		{
			static_assert((m1 <= m2) == true);
			REQUIRE(m1 <= m2);
		}
		{
			static_assert((m1 >= m2) == true);
			REQUIRE(m1 >= m2);
		}
		{
			static_assert((m1 == m2) == true);
			REQUIRE(m1 == m2);
		}
		{
			static_assert((m1 != m2) == false);
			REQUIRE(m1 != m2);
		}
	}
}

