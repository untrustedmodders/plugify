#include <catch_amalgamated.hpp>

#include <plg/vector.hpp>

TEST_CASE("vector capacity > empty", "[vector]") {

	SECTION("empty") {
		plg::vector<int> a;
		REQUIRE(a.empty());

		plg::vector<int> b{4, 4};
		REQUIRE_FALSE(b.empty());
	}

}
