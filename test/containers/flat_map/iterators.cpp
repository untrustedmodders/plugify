#include <catch_amalgamated.hpp>

#include "helper/flat.hpp"

TEMPLATE_PRODUCT_TEST_CASE("flatmap iterators", "[flatmap]",
                           (plg::flat::test::flatmap),
                           ((int, int))) {
    SECTION("iterator begin()/end() noexcept") {
        {
            TestType a;
            REQUIRE(a.begin() == a.end());
            REQUIRE_FALSE(a.begin() != a.end());
        }
        {
            TestType a;
            for (int i = 0; i < 31; ++i) {
                a.insert({i, i});
            }
            REQUIRE(a.begin() != a.end());
            REQUIRE_FALSE(a.begin() == a.end());
        }
    }

    SECTION("const_iterator begin()/end() const noexcept") {
        TestType const a;
        REQUIRE(a.begin() == a.end());
        REQUIRE_FALSE(a.begin() != a.end());
    }
}
