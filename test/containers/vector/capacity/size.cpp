#include <catch_amalgamated.hpp>

#include <plugifyvector.hpp>

TEST_CASE("vector capacity > size", "[strvectoring]") {

	SECTION("size") {
		plg::vector<int> a;
		REQUIRE(a.size() == 0);

		plg::vector<int> b{4, 4};
		REQUIRE(b.size() == 2);
		
		plg::vector<int> c;
		REQUIRE(0 == c.size());

		c.push_back(1);
		REQUIRE(1 == c.size());

		c.push_back(2);
		REQUIRE(2 == c.size());

		c.clear();
		REQUIRE(0 == c.size());

		c.push_back(1);
		c.push_back(2);
		c.push_back(3);
		c.push_back(4);
		c.push_back(5);
		REQUIRE(5 == c.size());
	}

}
