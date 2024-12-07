#include <catch_amalgamated.hpp>

#include "app/variant_tester.hpp"

#include <plugifyvariant.hpp>

TEST_CASE("variant > holds_alternative", "[variant]") {
	SECTION("bool holds_alternative(const variant<Types...>& v) noexcept") {
		{
			using V = plg::variant<int>;
			constexpr V v;
			static_assert(plg::holds_alternative<int>(v));
		}
		{
			using V = plg::variant<int, long>;
			constexpr V v;
			static_assert(plg::holds_alternative<int>(v));
			static_assert(!plg::holds_alternative<long>(v));
		}
		{// noexcept test
			using V = plg::variant<int>;
			const V v;
			REQUIRE(plg::holds_alternative<int>(v));
		}
	}
}