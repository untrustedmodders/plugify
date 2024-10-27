#include <catch2/catch_all.hpp>

#include <plugify/string.h>

TEST_CASE("string capacity > empty", "[string]") {

    SECTION("[[nodiscard]] bool empty() const noexcept") {
        plg::string a;
        REQUIRE(a.empty());

        plg::string b{"toto", 4};
        REQUIRE_FALSE(b.empty());
    }

}
