#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector access > data", "[vector]") {

    SECTION("data") {
        plg::vector<char> const a{'s', 'm', 'a', 'l', 'l'};
        REQUIRE(a.data()[0] == 's');
        REQUIRE(a.data()[4] == 'l');
		
		plg::vector<int> v;
		
		CHECK(v.empty());
		CHECK(v.data() == v.begin());

		v.push_back(1);
		CHECK(v.data() == v.begin());
		CHECK(1 == *v.data());

		v.push_back(2);
		CHECK(v.data() == v.begin());
		CHECK(1 == *v.data());
    }
}
