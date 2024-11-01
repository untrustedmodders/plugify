#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector capacity > size", "[strvectoring]") {

    SECTION("size") {
        plg::vector<int> a;
        REQUIRE(a.size() == 0);

        plg::vector<int> b{4, 4};
        REQUIRE(b.size() == 2);
		
		plg::vector<int> v;
		REQUIRE(0 == v.size());

		v.push_back(1);
		REQUIRE(1 == v.size());

		v.push_back(2);
		REQUIRE(2 == v.size());

		v.clear();
		REQUIRE(0 == v.size());
    }

}
