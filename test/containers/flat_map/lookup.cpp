#include <catch_amalgamated.hpp>

#include <string>

#include "nat.hpp"
#include "helper/flat.hpp"

TEMPLATE_PRODUCT_TEST_CASE("flat_map lookup", "[flat_map]",
                           (plg::flat::test::flatmap),
                           ((int, int))) {
    SECTION("iterator find(key_type const &key) noexcept") {
        {
            TestType a{{10, 10}, {20, 20}, {30, 30}, {40, 40}, {50, 50}, {60, 60}, {70, 70}, {80, 80}};
            int const k30{30};
            auto it = a.find(k30);
            REQUIRE(it->first == 30);
            REQUIRE(it->second == 30);
            int const k5{5};
            it = a.find(k5);
            REQUIRE(it == a.end());
            REQUIRE(it == a.cend());
        }
        {
            plg::flat::test::flatmap<char, char> a{{'a', 'a'}, {'b', 'b'}, {'c', 'c'}};
            auto it = a.find('b');
            REQUIRE(it->first == 'b');
            REQUIRE(it->second == 'b');
        }
    }

    SECTION("const_iterator find(key_type const &key) const noexcept") {
        TestType const a{{10, 10}, {20, 20}, {30, 30}, {40, 40}, {50, 50}, {60, 60}, {70, 70}, {80, 80}};
        int const k30{30};
        auto it = a.find(k30);
        REQUIRE(it->first == 30);
        REQUIRE(it->second == 30);
        int const k5{5};
        it = a.find(k5);
        REQUIRE(it == a.end());
        REQUIRE(it == a.cend());
    }

    SECTION("size_type count(key_type const& key) const") {
        TestType a{{10, 10}, {20, 20}, {30, 30}, {40, 40}, {50, 50}, {60, 60}, {70, 70}, {80, 80}};
        int const k30{30};
        REQUIRE(a.count(k30) == 1);
        int const k5{5};
        REQUIRE(a.count(k5) == 0);
    }

    SECTION("bool contains(key_type const& key) const") {
        TestType const a{{10, 10}, {20, 20}, {30, 30}, {40, 40}, {50, 50}, {60, 60}, {70, 70}, {80, 80}};
        int const k30{30};
        REQUIRE(a.contains(k30));
        int const k5{5};
        REQUIRE_FALSE(a.contains(k5));
    }

    SECTION("mapped_type& at(key_type const& key)") {
        TestType a{{1, 1}, {2, 2}, {3, 3}, {4, 4}};
        int const k1 = 1;
        a.at(k1) = 42;
        REQUIRE(a.at(k1) == 42);
        REQUIRE(a.size() == 4);
    }

    SECTION("mapped_type const& at(key_type const& key) const") {
        TestType const a{{1, 1}, {2, 2}, {3, 3}, {4, 4}};
        int const k1{1};
        REQUIRE(a.at(k1) == 1);
        REQUIRE(a.size() == 4);
    }

    SECTION("mapped_type& operator[](key_type const& key)") {
        TestType a{{1, 1}, {2, 2}, {3, 3}, {4, 4}, {1, 4}, {2, 4}};
        int const k1{1};
        REQUIRE(a.size() == 4);
        a[k1] = 42;
        REQUIRE(a.at(1) == 42);
        int const k11{11};
        a[k11] = 11;
        REQUIRE(a.size() == 5);
        REQUIRE(a.at(11) == 11);
    }

    SECTION("mapped_type& operator[](key_type&& key)") {
        TestType a{{1, 1}, {2, 2}, {3, 3}, {4, 4}, {1, 4}, {2, 4}};
        REQUIRE(a.size() == 4);
        a[1] = 42;
        REQUIRE(a.at(1) == 42);
        a[11] = 11;
        REQUIRE(a.size() == 5);
        REQUIRE(a.at(11) == 11);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("flat_map lookup", "[flat_map][nat]",
                           (plg::flat::test::flatmap),
                           ((int, nat))) {
    SECTION("iterator find(key_type const &key) noexcept") {
        reset_static_nat_counter();
        {
            TestType a{{10, nat{10}}, {20, nat{20}}, {30, nat{30}}, {40, nat{40}}, {50, nat{50}}, {60, nat{60}}, {70, nat{70}}, {80, nat{80}}};
            int const k30 = 30;
            auto it = a.find(k30);
            REQUIRE(it->first == 30);
            REQUIRE(it->second.cnt == 30);
            int const k5 = 5;
            it = a.find(k5);
            REQUIRE(it == a.end());
            REQUIRE(it == a.cend());

            REQUIRE(ctr == 8);
            REQUIRE(cpy == 8);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 15);
            REQUIRE(mv_assign == 7);
            REQUIRE(dtr == 23);
        }
        REQUIRE(ctr == 8);
        REQUIRE(cpy == 8);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 15);
        REQUIRE(mv_assign == 7);
        REQUIRE(dtr == 31);
    }

    SECTION("const_iterator find(key_type const &key) const noexcept") {
        reset_static_nat_counter();
        {
            TestType const a{{10, nat{10}}, {20, nat{20}}, {30, nat{30}}, {40, nat{40}}, {50, nat{50}}, {60, nat{60}}, {70, nat{70}}, {80, nat{80}}};
            int const k30 = 30;
            auto it = a.find(k30);
            REQUIRE(it->first == 30);
            REQUIRE(it->second.cnt == 30);
            int const k5 = 5;
            it = a.find(k5);
            REQUIRE(it == a.end());
            REQUIRE(it == a.cend());

            REQUIRE(ctr == 8);
            REQUIRE(cpy == 8);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 15);
            REQUIRE(mv_assign == 7);
            REQUIRE(dtr == 23);
        }
        REQUIRE(ctr == 8);
        REQUIRE(cpy == 8);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 15);
        REQUIRE(mv_assign == 7);
        REQUIRE(dtr == 31);
    }

    SECTION("size_type count(key_type const& key) const") {
        reset_static_nat_counter();
        {
            TestType const a{{10, nat{10}}, {20, nat{20}}, {30, nat{30}}, {40, nat{40}}, {50, nat{50}}, {60, nat{60}}, {70, nat{70}}, {80, nat{80}}};
            int const k30{30};
            REQUIRE(a.count(k30) == 1);
            int const k5{5};
            REQUIRE(a.count(k5) == 0);

            REQUIRE(ctr == 8);
            REQUIRE(cpy == 8);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 15);
            REQUIRE(mv_assign == 7);
            REQUIRE(dtr == 23);
        }
        REQUIRE(ctr == 8);
        REQUIRE(cpy == 8);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 15);
        REQUIRE(mv_assign == 7);
        REQUIRE(dtr == 31);
    }

    SECTION("bool contains(key_type const& key) const") {
        reset_static_nat_counter();
        {
            TestType const a{{10, nat{10}}, {20, nat{20}}, {30, nat{30}}, {40, nat{40}}, {50, nat{50}}, {60, nat{60}}, {70, nat{70}}, {80, nat{80}}};
            int const k30{30};
            REQUIRE(a.contains(k30));
            int const k5{5};
            REQUIRE_FALSE(a.contains(k5));

            REQUIRE(ctr == 8);
            REQUIRE(cpy == 8);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 15);
            REQUIRE(mv_assign == 7);
            REQUIRE(dtr == 23);
        }
        REQUIRE(ctr == 8);
        REQUIRE(cpy == 8);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 15);
        REQUIRE(mv_assign == 7);
        REQUIRE(dtr == 31);
    }

    SECTION("mapped_type& at(key_type const& key)") {
        reset_static_nat_counter();
        {
            TestType a{{1, nat{1}}, {2, nat{2}}, {3, nat{3}}, {4, nat{4}}};
            REQUIRE(a.size() == 4);
            a.at(1) = nat{42};
            REQUIRE(a.at(1).cnt == 42);
            REQUIRE(a.at(2).cnt == 2);

            REQUIRE(ctr == 5);
            REQUIRE(cpy == 4);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 7);
            REQUIRE(mv_assign == 4);
            REQUIRE(dtr == 12);
        }
        REQUIRE(ctr == 5);
        REQUIRE(cpy == 4);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 7);
        REQUIRE(mv_assign == 4);
        REQUIRE(dtr == 16);
    }

    SECTION("mapped_type const& at(key_type const& key) const") {
        reset_static_nat_counter();
        {
            TestType const a{{1, nat{1}}, {2, nat{2}}, {3, nat{3}}, {4, nat{4}}};
            int const k{1};
            REQUIRE(a.size() == 4);
            REQUIRE(a.at(k).cnt == 1);

            REQUIRE(ctr == 4);
            REQUIRE(cpy == 4);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 7);
            REQUIRE(mv_assign == 3);
            REQUIRE(dtr == 11);
        }

        REQUIRE(ctr == 4);
        REQUIRE(cpy == 4);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 7);
        REQUIRE(mv_assign == 3);
        REQUIRE(dtr == 15);
    }

    SECTION("mapped_type& operator[](key_type const& key)") {
        reset_static_nat_counter();
        {
            TestType a{{1, nat{1}}, {2, nat{2}}, {3, nat{3}}, {4, nat{4}}};
            REQUIRE(a.size() == 4);
            int const k1{11};
            auto& ref1 = a[k1];
            REQUIRE(a.size() == 5);
            REQUIRE(ctr == 5);
            int const k2{11};
            auto& ref2 = a[k2];
            REQUIRE(a.size() == 5);
            REQUIRE(ctr == 5);
            REQUIRE(&ref1 == &ref2);

            REQUIRE(ctr == 5);
            REQUIRE(cpy == 4);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 13);
            REQUIRE(mv_assign == 3);
            REQUIRE(dtr == 17);
        }
        REQUIRE(ctr == 5);
        REQUIRE(cpy == 4);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 13);
        REQUIRE(mv_assign == 3);
        REQUIRE(dtr == 22);
    }

    SECTION("mapped_type& operator[](key_type&& key)") {
        reset_static_nat_counter();
        {
            TestType a{{1, nat{1}}, {2, nat{2}}, {3, nat{3}}, {4, nat{4}}};
            REQUIRE(a.size() == 4);
            auto& ref1 = a[11];
            REQUIRE(a.size() == 5);
            REQUIRE(ctr == 5);
            auto& ref2 = a[11];
            REQUIRE(a.size() == 5);
            REQUIRE(ctr == 5);
            REQUIRE(&ref1 == &ref2);

            REQUIRE(ctr == 5);
            REQUIRE(cpy == 4);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 13);
            REQUIRE(mv_assign == 3);
            REQUIRE(dtr == 17);
        }
        REQUIRE(ctr == 5);
        REQUIRE(cpy == 4);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 13);
        REQUIRE(mv_assign == 3);
        REQUIRE(dtr == 22);
    }
}
