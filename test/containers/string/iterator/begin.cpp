#include <catch2/catch_all.hpp>

#include <plugify/string.hpp>

TEST_CASE("string iterator > begin", "[string]") {

    SECTION("iterator begin() noexcept") {
        plg::string a{"toto", 4};
        REQUIRE(*a.begin() == 't');

        plg::string b{"longlonglonglonglonglonglonglong", 32};
        REQUIRE(*b.begin() == 'l');

        plg::string c;
        REQUIRE(*c.begin() == '\0');
    }

    SECTION("const_iterator begin() const noexcept") {
        plg::string const a{"toto", 4};
        REQUIRE(*a.begin() == 't');

        plg::string const b{"longlonglonglonglonglonglonglong", 32};
        REQUIRE(*b.begin() == 'l');

        plg::string const c;
        REQUIRE(*c.begin() == '\0');
    }

    SECTION("const_iterator cbegin() const noexcept") {
        plg::string a{"toto", 4};
        REQUIRE(*a.cbegin() == 't');

        plg::string const b{"longlonglonglonglonglonglonglong", 32};
        REQUIRE(*b.cbegin() == 'l');

        plg::string c;
        REQUIRE(*c.cbegin() == '\0');
    }

}
