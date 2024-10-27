#include <catch2/catch_all.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector capacity > size", "[strvectoring]") {

    SECTION("size") {
        plg::vector<int> a;
        REQUIRE(a.size() == 0);

        plg::vector<int> b{4, 4};
        REQUIRE(b.size() == 2);
    }

}
