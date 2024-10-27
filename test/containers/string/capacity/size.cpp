#include <catch2/catch_all.hpp>

#include <plugify/string.hpp>

TEST_CASE("string capacity > size", "[string]") {

    SECTION("size_type size() const noexcept") {
        plg::string a;
        REQUIRE(a.size() == 0);

        plg::string b{"toto", 4};
        REQUIRE(b.size() == 4);
    }

}
