#include <catch2/catch_all.hpp>

#include <plugify/string.h>

TEST_CASE("string capacity > capacity", "[string]") {

    SECTION("size_type capacity() const noexcept") {
        plg::string a;
        REQUIRE(a.size() == 0);

        plg::string b{"toto", 4};
        REQUIRE(b.size() == 4);
    }

}
