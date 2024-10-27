#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>

TEST_CASE("string iterator > end", "[string]") {

    SECTION("iterator end() noexcept") {
        plg::string a{"toto", 4};
        REQUIRE(*a.end() == '\0');

        plg::string b{"longlonglonglonglonglonglonglong", 32};
        REQUIRE(*b.end() == '\0');

        plg::string c;
        REQUIRE(*c.end() == '\0');
    }

    SECTION("const_iterator end() const noexcept") {
        plg::string const a{"toto", 4};
        REQUIRE(*a.end() == '\0');

        plg::string const b{"longlonglonglonglonglonglonglong", 32};
        REQUIRE(*b.end() == '\0');

        plg::string const c;
        REQUIRE(*c.end() == '\0');
    }

    SECTION("const_iterator cend() const noexcept") {
        plg::string a{"toto", 4};
        REQUIRE(*a.cend() == '\0');

        plg::string const b{"longlonglonglonglonglonglonglong", 32};
        REQUIRE(*b.cend() == '\0');

        plg::string c;
        REQUIRE(*c.cend() == '\0');
    }

}
