#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector access > front", "[vector]") {

    SECTION("front") {
        plg::vector<char> a{'s', 'm', 'a', 'l', 'l'};
        REQUIRE(a.front() == 's');
        a.front() = 't';
        REQUIRE(a.front() == 't');
    }
}
