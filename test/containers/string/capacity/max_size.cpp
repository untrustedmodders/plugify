#include <catch2/catch_all.hpp>

#include <cstddef>

#include <plugify/string.hpp>

TEST_CASE("string capacity > max_size", "[string]") {

    SECTION("size_type max_size() const noexcept") {
        plg::string a;
        REQUIRE(a.max_size() == 0x7fffffffffffffff);
    }

}
