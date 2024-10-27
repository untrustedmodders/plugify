#include <catch2/catch_all.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector access > data", "[vector]") {

    SECTION("data") {
        plg::vector<char> const a{'s', 'm', 'a', 'l', 'l'};
        REQUIRE(a.data()[0] == 's');
        REQUIRE(a.data()[4] == 'l');
    }
}
