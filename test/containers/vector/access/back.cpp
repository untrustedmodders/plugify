#include <catch2/catch_all.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector access > back", "[vector]") {

    SECTION("back") {
        plg::vector<char> a{'s', 'm', 'a', 'l', 'l'};
        REQUIRE(a.back() == 'l');
        a.back() = 't';
        REQUIRE(a.back() == 't');
    }
}
