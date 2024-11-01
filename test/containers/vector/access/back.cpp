#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector access > back", "[vector]") {

    SECTION("back") {
        plg::vector<char> a{'s', 'm', 'a', 'l', 'l'};
        REQUIRE(a.back() == 'l');
        a.back() = 't';
        REQUIRE(a.back() == 't');

		plg::vector<int> v{ 2 };

		REQUIRE(v.back () == 2);
		REQUIRE(v.back () == v[v.size () - 1]);

		v.push_back(3);

		REQUIRE(v.back () == 3);
		REQUIRE(v.back () == v[v.size () - 1]);

		v.insert(v.begin (), 1);

		REQUIRE(v.back () == 3);
		REQUIRE(v.back () == v[v.size () - 1]);
    }
}
