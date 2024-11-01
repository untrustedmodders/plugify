#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector access > front", "[vector]") {

    SECTION("front") {
        plg::vector<char> a{'s', 'm', 'a', 'l', 'l'};
        REQUIRE(a.front() == 's');
        a.front() = 't';
        REQUIRE(a.front() == 't');

		plg::vector<int> v { 2 };
		
		REQUIRE(v.front() == 2);
		REQUIRE(v.front() == v[0]);

		v.push_back(3);

		REQUIRE(v.front() == 2);
		REQUIRE(v.front() == v[0]);

		v.insert(v.begin(), 1);

		REQUIRE(v.front() == 1);
		REQUIRE(v.front() == v[0]);
    }
}
