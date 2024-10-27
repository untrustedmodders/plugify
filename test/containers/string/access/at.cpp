#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>

TEST_CASE("string access > at", "[string]") {

    SECTION("reference at(size_type pos) noexcept") {
        plg::string a{"small", 5};
        REQUIRE(a.at(0) == 's');
        REQUIRE(a.at(4) == 'l');
        a.at(0) = 't';
        REQUIRE(a.at(0) == 't');
        a.at(4) = 't';
        REQUIRE(a.at(4) == 't');

        plg::string b{"longlonglonglonglonglonglonglong", 32};
        REQUIRE(b.at(0) == 'l');
        REQUIRE(b.at(31) == 'g');
        b.at(0) = 't';
        REQUIRE(b.at(0) == 't');
        b.at(31) = 't';
        REQUIRE(b.at(31) == 't');
    }

    SECTION("const_reference at(size_type pos) const noexcept") {
        plg::string const a{"small", 5};
        REQUIRE(a.at(0) == 's');
        REQUIRE(a.at(4) == 'l');

        plg::string const b{"longlonglonglonglonglonglonglong", 32};
        REQUIRE(b.at(0) == 'l');
        REQUIRE(b.at(31) == 'g');
    }

}
