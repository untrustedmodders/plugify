#include <catch_amalgamated.hpp>

#include "nat.hpp"
#include "helper/flat.hpp"
#include <cstddef>
#include <random>
#include <type_traits>

template <typename T, std::enable_if_t<std::is_arithmetic_v<T> && std::is_integral_v<T>>* = nullptr>
static inline T get_random(T a, T b) noexcept {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dis(a, b);
    return dis(gen);
}

TEMPLATE_PRODUCT_TEST_CASE("flatmap modifiers", "[flatmap]",
                           (plg::flat::test::flatmap),
                           ((int, int))) {
    SECTION("void clear()") {
        TestType a{{10, 10}, {20, 20}, {30, 30}, {40, 40}, {50, 50}, {60, 60}, {70, 70}, {80, 80}};
        a.clear();
        REQUIRE(a.size() == 0);
    }

    SECTION("pair<iterator, bool> insert(value_type const& value)") {
        {
            TestType a;
            std::pair<int, int> p1{1, 1};
            auto it = a.insert(p1);
            REQUIRE(it.second);
            REQUIRE(it.first->first == 1);
            REQUIRE(it.first->second == 1);
            std::pair<int, int> const p2{2, 2};
            auto jt = a.insert(p2);
            REQUIRE(jt.second);
            REQUIRE(jt.first->first == 2);
            REQUIRE(jt.first->second == 2);
        }
        {
            TestType a;
            std::pair<int, int> p1{1, 1};
            auto it = a.insert(p1);
            REQUIRE(it.second);
            REQUIRE(it.first->first == 1);
            REQUIRE(it.first->second == 1);
            std::pair<int, int> p2{1, 2};
            auto jt = a.insert(p2);
            REQUIRE_FALSE(jt.second);
            REQUIRE(jt.first->first == 1);
            REQUIRE(jt.first->second == 1);
        }
        {
            std::pair<int, int> data[1000];
            for (int i = 0; i < 1000; ++i) {
                data[i].first = i;
                data[i].second = ::get_random(-1000000, 1000000);
            }
            for (auto i = 1000 - 1; i > 0; --i) {
                std::swap(data[i], data[::get_random(0, i)]);
            }
            TestType a;
            for (auto& p : data) {
                auto it = a.insert(p);
                REQUIRE(it.first != a.end());
                REQUIRE(p.second == it.first->second);
                REQUIRE(p.first == it.first->first);
            }

            for (auto& p : data) {
                auto it = a.find(p.first);
                REQUIRE(it != a.end());
                REQUIRE(p.second == it->second);
                REQUIRE(p.first == it->first);
            }
        }
    }

    SECTION("pair<iterator, bool> insert(value_type&& value)") {
        {
            TestType a;
            auto it = a.insert({1, 1});
            REQUIRE(it.second);
            REQUIRE(it.first->first == 1);
            REQUIRE(it.first->second == 1);
            auto jt = a.insert({2, 2});
            REQUIRE(jt.second);
            REQUIRE(jt.first->first == 2);
            REQUIRE(jt.first->second == 2);
        }
        {
            TestType a;
            auto it = a.insert({1, 1});
            REQUIRE(it.second);
            REQUIRE(it.first->first == 1);
            REQUIRE(it.first->second == 1);
            auto jt = a.insert({1, 2});
            REQUIRE_FALSE(jt.second);
            REQUIRE(jt.first->first == 1);
            REQUIRE(jt.first->second == 1);
        }
    }

    SECTION("iterator insert(const_iterator, value_type const& value)") {
        {
            TestType a;
            std::pair<int, int> p1{1, 1};
            auto it = a.insert(a.end(), p1);
            REQUIRE(it->second == 1);
            REQUIRE(it->first == 1);
            REQUIRE(a.size() == 1);
        }
        {
            TestType a;
            typename TestType::const_iterator e = a.end();
            std::pair<int, int> const p1{1, 1};
            auto it = a.insert(e, p1);
            REQUIRE(it->second == 1);
            REQUIRE(it->first == 1);
            REQUIRE(a.size() == 1);
        }
    }

    SECTION("iterator insert(const_iterator, value_type&& value)") {
        {
            TestType a;
            auto it = a.insert(a.end(), {1, 1});
            REQUIRE(it->second == 1);
            REQUIRE(it->first == 1);
            REQUIRE(a.size() == 1);
        }
        {
            TestType a;
            typename TestType::const_iterator e = a.end();
            auto it = a.insert(e, {1, 1});
            REQUIRE(it->second == 1);
            REQUIRE(it->first == 1);
            REQUIRE(a.size() == 1);
        }
    }

    SECTION("void insert(It begin, It end)") {
        {
            std::pair<int, int> data[] = {{1, 1}, {2, 2}, {3, 3}};
            TestType a;
            a.insert(data, data + sizeof(data) / sizeof(data[0]));
            REQUIRE(a.size() == 3);
        }
        {
            std::pair<int, int> data[] = {
                {1, 1}, {2, 2}, {3, 3}, {1, 4}, {2, 4},
            };
            TestType a;
            a.insert(data, data + sizeof(data) / sizeof(data[0]));
            REQUIRE(a.size() == 3);
        }
        {
            std::pair<int, int> const data[] = {{1, 1}, {2, 2}, {3, 3}};
            TestType a;
            a.insert(data, data + sizeof(data) / sizeof(data[0]));
            REQUIRE(a.size() == 3);
        }
    }

    SECTION("void insert(std::initializer_list<value_type> il)") {
        TestType a;
        a.insert({{1, 1}, {2, 2}, {3, 3}});
        REQUIRE(a.size() == 3);
    }

    SECTION("pair<iterator, bool> insert_or_assign(key_type const& key, M&& m)") {
        TestType a;
        a.emplace(8, 8);
        REQUIRE(a.size() == 1);

        int const k8{8};
        auto it = a.insert_or_assign(k8, 8 * 8);
        REQUIRE(a.size() == 1);
        REQUIRE_FALSE(it.second);
        REQUIRE(it.first->first == 8);
        REQUIRE(it.first->second == 8 * 8);

        int const k1{-1};
        auto jt = a.insert_or_assign(k1, 1);
        REQUIRE(a.size() == 2);
        REQUIRE(jt.second);
        REQUIRE(jt.first->first == -1);
        REQUIRE(jt.first->second == 1);
    }

    SECTION("pair<iterator, bool> insert_or_assign(key_type&& key, M&& m)") {
        TestType a;
        a.emplace(8, 8);
        REQUIRE(a.size() == 1);

        auto it = a.insert_or_assign(8, 8 * 8);
        REQUIRE(a.size() == 1);
        REQUIRE_FALSE(it.second);
        REQUIRE(it.first->first == 8);
        REQUIRE(it.first->second == 8 * 8);

        auto jt = a.insert_or_assign(-1, 1);
        REQUIRE(a.size() == 2);
        REQUIRE(jt.second);
        REQUIRE(jt.first->first == -1);
        REQUIRE(jt.first->second == 1);
    }

    /*SECTION("iterator insert_or_assign(const_iterator, key_type const& key, M&& m)") {
        TestType a;
        a.emplace(8, 8);
        REQUIRE(a.size() == 1);

        int const k8{8};
        auto it = a.insert_or_assign(a.end(), k8, 8 * 8);
        REQUIRE(a.size() == 1);
        REQUIRE(it->first == 8);
        REQUIRE(it->second == 8 * 8);

        int const k1{-1};
        auto jt = a.insert_or_assign(a.find(8), k1, 1);
        REQUIRE(a.size() == 2);
        REQUIRE(jt->first == -1);
        REQUIRE(jt->second == 1);
    }

    SECTION("iterator insert_or_assign(const_iterator, key_type&& key, M&& m)") {
        TestType a;
        a.emplace(8, 8);
        REQUIRE(a.size() == 1);

        auto it = a.insert_or_assign(a.end(), 8, 8 * 8);
        REQUIRE(a.size() == 1);
        REQUIRE(it->first == 8);
        REQUIRE(it->second == 8 * 8);

        auto jt = a.insert_or_assign(a.find(8), -1, 1);
        REQUIRE(a.size() == 2);
        REQUIRE(jt->first == -1);
        REQUIRE(jt->second == 1);
    }

    SECTION("pair<iterator, bool> emplace(Args&&... args)") {
        TestType a;
        auto it = a.emplace(0, 42);
        REQUIRE(it.second);
        REQUIRE(it.first->first == 0);
        REQUIRE(it.first->second == 42);
        REQUIRE(a.size() == 1);
    }*/

    SECTION("iterator emplace_hint(const_iterator, Args&&... args)") {
        {
            TestType a;
            auto it = a.emplace_hint(a.end(), 1, 1);
            REQUIRE(it->second == 1);
            REQUIRE(it->first == 1);
            REQUIRE(a.size() == 1);
        }
        {
            TestType a;
            typename TestType::const_iterator e = a.end();
            auto it = a.emplace_hint(e, 1, 1);
            REQUIRE(it->second == 1);
            REQUIRE(it->first == 1);
            REQUIRE(a.size() == 1);
        }
    }

    SECTION("convertible_to_iterator erase(const_iterator pos)") {
        for (int test = 0; test < 1000; ++test) {
            std::pair<int, int> data[20];
            for (int i = 0; i < 20; ++i) {
                data[i].first = ::get_random(-1000000, 1000000);
                data[i].second = ::get_random(-1000000, 1000000);
            }
            for (auto i = 20 - 1; i > 0; --i) {
                std::swap(data[i], data[::get_random(0, i)]);
            }
            TestType a;
            for (auto& p : data) {
                a.insert(p);
            }
            using iterator = typename TestType::iterator;
            int remove_key = ::get_random(1, 18);
            iterator to_remove = a.begin();
            iterator to_remove_next = ++a.begin();
            for (int i = 1; i < remove_key; ++i) {
                ++to_remove;
                ++to_remove_next;
            }
            auto result = *to_remove_next;
            size_t const sz = a.size();
            iterator it = a.erase(to_remove);
            REQUIRE(result.first == it->first);
            REQUIRE(result.second == it->second);
            REQUIRE(a.size() == sz - 1ul);
        }
    }

    SECTION("iterator erase(const_iterator first, const_iterator last)") {
        for (int test = 0; test < 1000; ++test) {
            std::pair<int, int> data[20];
            for (int i = 0; i < 20; ++i) {
                data[i].first = ::get_random(-1000000, 1000000);
                data[i].second = ::get_random(-1000000, 1000000);
            }
            for (auto i = 20 - 1; i > 0; --i) {
                std::swap(data[i], data[::get_random(0, i)]);
            }
            TestType a;
            for (auto& p : data) {
                a.insert(p);
            }
            using iterator = typename TestType::iterator;
            int remove_first = ::get_random(0, 18);
            iterator to_remove_beg = a.begin();
            for (int i = 1; i < remove_first; ++i) {
                ++to_remove_beg;
            }
            int remove_count = ::get_random(1, 19 - remove_first);
            iterator to_remove_last = to_remove_beg;
            for (int i = 0; i < remove_count; ++i) {
                ++to_remove_last;
            }
            auto result = *to_remove_last;
            size_t const sz = a.size();
            iterator it = a.erase(to_remove_beg, to_remove_last);
            REQUIRE(result.first == it->first);
            REQUIRE(result.second == it->second);
            REQUIRE(a.size() == sz - static_cast<size_t>(remove_count));
        }
    }

    SECTION("size_type erase(key_type const& key)") {
        TestType a{{1, 1},   {2, 2},   {3, 3},   {4, 4},   {5, 5},   {6, 6},   {7, 7},   {8, 8},   {9, 9},   {10, 10},
                   {11, 11}, {12, 12}, {13, 13}, {14, 14}, {15, 15}, {16, 16}, {17, 17}, {18, 18}, {19, 19}, {20, 20}};
        int const k5{5};
        auto next = a.erase(k5);
        REQUIRE(a.size() == 19);
        REQUIRE(a.at(4) == 4);
        REQUIRE(next == 1);

        int const k42{42};
        next = a.erase(k42);
        REQUIRE(a.size() == 19);
        REQUIRE(a.at(1) == 1);
        REQUIRE(a.at(3) == 3);
        REQUIRE(a.at(4) == 4);
        REQUIRE(next == 0);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("flatmap modifiers", "[flatmap][nat]",
                           (plg::flat::test::flatmap),
                           ((int, nat))) {
    SECTION("void clear()") {
        reset_static_nat_counter();
        {
            TestType a{{10, nat{10}}, {20, nat{20}}, {30, nat{30}}, {40, nat{40}}, {50, nat{50}}, {60, nat{60}}, {70, nat{70}}, {80, nat{80}}};
            a.clear();
            REQUIRE(a.size() == 0);

            REQUIRE(ctr == 8);
            REQUIRE(cpy == 8);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 15);
            REQUIRE(mv_assign == 7);
            REQUIRE(dtr == 31);
        }
        REQUIRE(ctr == 8);
        REQUIRE(cpy == 8);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 15);
        REQUIRE(mv_assign == 7);
        REQUIRE(dtr == 31);
    }

    SECTION("pair<iterator, bool> insert(value_type const& value)") {
        reset_static_nat_counter();
        {
            TestType a;
            std::pair<int, nat> p1{1, nat{1}};
            auto it = a.insert(p1);
            REQUIRE(it.second);
            REQUIRE(it.first->first == 1);
            REQUIRE(it.first->second.cnt == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 1);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 1);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 1);

            std::pair<int, nat> const p2{2, nat{2}};
            auto jt = a.insert(p2);
            REQUIRE(jt.second);
            REQUIRE(jt.first->first == 2);
            REQUIRE(jt.first->second.cnt == 2);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 2);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 3);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 3);
        }
        REQUIRE(ctr == 2);
        REQUIRE(cpy == 2);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 3);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 7);

        reset_static_nat_counter();
        {
            TestType a;
            std::pair<int, nat> p1{1, nat{1}};
            auto it = a.insert(p1);
            REQUIRE(it.second);
            REQUIRE(it.first->first == 1);
            REQUIRE(it.first->second.cnt == 1);
            std::pair<int, nat> p2{1, nat{2}};
            auto jt = a.insert(p2);
            REQUIRE_FALSE(jt.second);
            REQUIRE(jt.first->first == 1);
            REQUIRE(jt.first->second.cnt == 1);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 1);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);
        }
        REQUIRE(ctr == 2);
        REQUIRE(cpy == 1);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 5);

        {
            std::pair<int, nat> data[1000];
            for (int i = 0; i < 1000; ++i) {
                data[i].first = i;
                data[i].second = nat{::get_random(-1000000ll, 1000000ll)};
            }
            for (auto i = 1000 - 1; i > 0; --i) {
                std::swap(data[i], data[::get_random(0, i)]);
            }
            reset_static_nat_counter();
            TestType a{1000 >> 4};
            for (auto& p : data) {
                auto it = a.insert(p);
                REQUIRE(it.first != a.end());
                REQUIRE(p.second == it.first->second);
                REQUIRE(p.first == it.first->first);
            }
            REQUIRE(ctr == 0);
            REQUIRE(cpy == 1000);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 0);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 0);

            for (auto& p : data) {
                auto it = a.find(p.first);
                REQUIRE(it != a.end());
                REQUIRE(p.second == it->second);
                REQUIRE(p.first == it->first);
            }
        }
        REQUIRE(ctr == 0);
        REQUIRE(cpy == 1000);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 0);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 2000);
    }

    SECTION("pair<iterator, bool> insert(value_type&& value)") {
        reset_static_nat_counter();
        {
            TestType a;
            auto it = a.insert({1, nat{1}});
            REQUIRE(it.second);
            REQUIRE(it.first->first == 1);
            REQUIRE(it.first->second.cnt == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);

            auto jt = a.insert({2, nat{2}});
            REQUIRE(jt.second);
            REQUIRE(jt.first->first == 2);
            REQUIRE(jt.first->second.cnt == 2);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 5);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 5);
        }
        REQUIRE(ctr == 2);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 5);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 7);

        reset_static_nat_counter();
        {
            TestType a;
            auto it = a.insert({1, nat{1}});
            REQUIRE(it.second);
            REQUIRE(it.first->first == 1);
            REQUIRE(it.first->second.cnt == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);

            auto jt = a.insert({1, nat{2}});
            REQUIRE_FALSE(jt.second);
            REQUIRE(jt.first->first == 1);
            REQUIRE(jt.first->second.cnt == 1);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 3);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 4);
        }
        REQUIRE(ctr == 2);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 3);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 5);
    }

    SECTION("iterator insert(const_iterator, value_type const& value)") {
        reset_static_nat_counter();
        {
            TestType a;
            std::pair<int, nat> p1{1, nat{1}};
            auto it = a.insert(a.end(), p1);
            REQUIRE(it->first == 1);
            REQUIRE(it->second.cnt == 1);
            REQUIRE(a.size() == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 1);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 1);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 1);
        }
        REQUIRE(ctr == 1);
        REQUIRE(cpy == 1);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 1);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 3);

        reset_static_nat_counter();
        {
            /*TestType a;
            typename TestType::const_iterator e = a.end();
            std::pair<int, nat> const p1{1, nat{1}};
            auto it = a.insert(p1);
            REQUIRE(it->first == 1);
            REQUIRE(it->second.cnt == 1);
            REQUIRE(a.size() == 1);*/

            REQUIRE(ctr == 0);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 0);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 0);
        }
        REQUIRE(ctr == 0);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 0);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 0);
    }

    SECTION("iterator insert(const_iterator, value_type&& value)") {
        reset_static_nat_counter();
        {
            TestType a;
            auto it = a.insert(a.end(), {1, nat{1}});
            REQUIRE(it->first == 1);
            REQUIRE(it->second.cnt == 1);
            REQUIRE(a.size() == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);
        }
        REQUIRE(ctr == 1);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 3);

        reset_static_nat_counter();
        {
            TestType a;
            typename TestType::const_iterator e = a.end();
            auto it = a.insert(e, {1, nat{1}});
            REQUIRE(it->first == 1);
            REQUIRE(it->second.cnt == 1);
            REQUIRE(a.size() == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);
        }
        REQUIRE(ctr == 1);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 3);
    }

    SECTION("void insert(It begin, It end)") {
        reset_static_nat_counter();
        {
            std::pair<int, nat> data[] = {{1, nat{1}}, {2, nat{2}}, {3, nat{3}}};
            TestType a;
            a.insert(data, data + sizeof(data) / sizeof(data[0]));
            REQUIRE(a.size() == 3);

            REQUIRE(ctr == 3);
            REQUIRE(cpy == 3);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 5);
            REQUIRE(mv_assign == 2);
            REQUIRE(dtr == 5);
        }
        REQUIRE(ctr == 3);
        REQUIRE(cpy == 3);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 5);
        REQUIRE(mv_assign == 2);
        REQUIRE(dtr == 11);

        reset_static_nat_counter();
        {
            std::pair<int, nat> data[] = {
                {1, nat{1}}, {2, nat{2}}, {3, nat{3}}, {1, nat{4}}, {2, nat{4}},
            };
            TestType a;
            a.insert(data, data + sizeof(data) / sizeof(data[0]));
            REQUIRE(a.size() == 3);

            REQUIRE(ctr == 5);
            REQUIRE(cpy == 5);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 9);
            REQUIRE(mv_assign == 9);
            REQUIRE(dtr == 11);
        }
        REQUIRE(ctr == 5);
        REQUIRE(cpy == 5);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 9);
        REQUIRE(mv_assign == 9);
        REQUIRE(dtr == 19);

        reset_static_nat_counter();
        {
            std::pair<int, nat> const data[] = {{1, nat{1}}, {2, nat{2}}, {3, nat{3}}};
            TestType a;
            a.insert(data, data + sizeof(data) / sizeof(data[0]));
            REQUIRE(a.size() == 3);

            REQUIRE(ctr == 3);
            REQUIRE(cpy == 3);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 5);
            REQUIRE(mv_assign == 2);
            REQUIRE(dtr == 5);
        }
        REQUIRE(ctr == 3);
        REQUIRE(cpy == 3);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 5);
        REQUIRE(mv_assign == 2);
        REQUIRE(dtr == 11);
    }

    SECTION("void insert(std::initializer_list<value_type> il)") {
        reset_static_nat_counter();
        {
            TestType a;
            a.insert({{1, nat{1}}, {2, nat{2}}, {3, nat{3}}});
            REQUIRE(a.size() == 3);

            REQUIRE(ctr == 3);
            REQUIRE(cpy == 3);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 5);
            REQUIRE(mv_assign == 2);
            REQUIRE(dtr == 8);
        }
        REQUIRE(ctr == 3);
        REQUIRE(cpy == 3);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 5);
        REQUIRE(mv_assign == 2);
        REQUIRE(dtr == 11);
    }

    SECTION("pair<iterator, bool> insert_or_assign(key_type const& key, M&& m)") {
        reset_static_nat_counter();
        {
            TestType a;
            a.emplace(8, nat{8});
            REQUIRE(a.size() == 1);

            int const k8{8};
            auto it = a.insert_or_assign(k8, nat{8 * 8});
            REQUIRE(a.size() == 1);
            REQUIRE_FALSE(it.second);
            REQUIRE(it.first->first == 8);
            REQUIRE(it.first->second.cnt == 8 * 8);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 1);
            REQUIRE(dtr == 3);

            int const k1{-1};
            auto jt = a.insert_or_assign(k1, nat{1});
            REQUIRE(a.size() == 2);
            REQUIRE(jt.second);
            REQUIRE(jt.first->first == -1);
            REQUIRE(jt.first->second.cnt == 1);

            REQUIRE(ctr == 3);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 5);
            REQUIRE(mv_assign == 1);
            REQUIRE(dtr == 6);
        }
        REQUIRE(ctr == 3);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 5);
        REQUIRE(mv_assign == 1);
        REQUIRE(dtr == 8);
    }

    SECTION("pair<iterator, bool> insert_or_assign(key_type&& key, M&& m)") {
        reset_static_nat_counter();
        {
            TestType a;
            a.emplace(8, nat{8});
            REQUIRE(a.size() == 1);

            auto it = a.insert_or_assign(8, nat{8 * 8});
            REQUIRE(a.size() == 1);
            REQUIRE_FALSE(it.second);
            REQUIRE(it.first->first == 8);
            REQUIRE(it.first->second.cnt == 8 * 8);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 1);
            REQUIRE(dtr == 3);

            auto jt = a.insert_or_assign(-1, nat{1});
            REQUIRE(a.size() == 2);
            REQUIRE(jt.second);
            REQUIRE(jt.first->first == -1);
            REQUIRE(jt.first->second.cnt == 1);

            REQUIRE(ctr == 3);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 5);
            REQUIRE(mv_assign == 1);
            REQUIRE(dtr == 6);
        }
        REQUIRE(ctr == 3);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 5);
        REQUIRE(mv_assign == 1);
        REQUIRE(dtr == 8);
    }

    /*SECTION("iterator insert_or_assign(const_iterator, key_type const& key, M&& m)") {
        reset_static_nat_counter();
        {
            TestType a;
            a.emplace(8, nat{8});
            REQUIRE(a.size() == 1);

            int const k8{8};
            auto it = a.insert_or_assign(a.end(), k8, nat{8 * 8});
            REQUIRE(a.size() == 1);
            REQUIRE(it->first == 8);
            REQUIRE(it->second.cnt == 8 * 8);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 1);
            REQUIRE(mv_assign == 1);
            REQUIRE(dtr == 2);

            int const k1{-1};
            auto jt = a.insert_or_assign(a.find(8), k1, nat{1});
            REQUIRE(a.size() == 2);
            REQUIRE(jt->first == -1);
            REQUIRE(jt->second.cnt == 1);

            REQUIRE(ctr == 3);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 1);
            REQUIRE(dtr == 3);
        }
        REQUIRE(ctr == 3);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 1);
        REQUIRE(dtr == 5);
    }

    SECTION("iterator insert_or_assign(const_iterator, key_type&& key, M&& m)") {
        reset_static_nat_counter();
        {
            TestType a;
            a.emplace(8, nat{8});
            REQUIRE(a.size() == 1);

            auto it = a.insert_or_assign(a.end(), 8, nat{8 * 8});
            REQUIRE(a.size() == 1);
            REQUIRE(it->first == 8);
            REQUIRE(it->second.cnt == 8 * 8);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 1);
            REQUIRE(mv_assign == 1);
            REQUIRE(dtr == 2);

            auto jt = a.insert_or_assign(a.find(8), -1, nat{1});
            REQUIRE(a.size() == 2);
            REQUIRE(jt->first == -1);
            REQUIRE(jt->second.cnt == 1);

            REQUIRE(ctr == 3);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 1);
            REQUIRE(dtr == 3);
        }
        REQUIRE(ctr == 3);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 1);
        REQUIRE(dtr == 5);
    }*/

    SECTION("pair<iterator, bool> emplace(Args&&... args)") {
        reset_static_nat_counter();
        {
            TestType a;
            auto it = a.emplace(0, nat{42});
            REQUIRE(it.second);
            REQUIRE(it.first->first == 0);
            REQUIRE(it.first->second.cnt == 42);
            REQUIRE(a.size() == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);
        }
        REQUIRE(ctr == 1);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 3);
    }

    SECTION("iterator emplace_hint(const_iterator, Args&&... args)") {
        reset_static_nat_counter();
        {
            TestType a;
            auto it = a.emplace_hint(a.end(), 1, nat{1});
            REQUIRE(it->first == 1);
            REQUIRE(it->second.cnt == 1);
            REQUIRE(a.size() == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);
        }
        REQUIRE(ctr == 1);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 3);

        reset_static_nat_counter();
        {
            TestType a;
            typename TestType::const_iterator e = a.end();
            auto it = a.emplace_hint(e, 1, nat{1});
            REQUIRE(it->first == 1);
            REQUIRE(it->second.cnt == 1);
            REQUIRE(a.size() == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 0);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);
        }
        REQUIRE(ctr == 1);
        REQUIRE(cpy == 0);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 3);
    }

    SECTION("convertible_to_iterator erase(const_iterator pos)") {
        for (int test = 0; test < 1000; ++test) {
            std::pair<int, nat> data[20];
            for (int i = 0; i < 20; ++i) {
                data[i].first = i;
                data[i].second = nat{::get_random(-1000000ll, 1000000ll)};
            }
            for (auto i = 20 - 1; i > 0; --i) {
                std::swap(data[i], data[::get_random(0, i)]);
            }
            reset_static_nat_counter();
            {
                TestType a{20 >> 2};
                for (auto& p : data) {
                    a.insert(p);
                }
                using iterator = typename TestType::iterator;
                int remove_key = ::get_random(1, 18);
                iterator to_remove = a.begin();
                iterator to_remove_next = ++a.begin();
                for (int i = 1; i < remove_key; ++i) {
                    ++to_remove;
                    ++to_remove_next;
                }
                auto result = *to_remove_next;
                size_t const sz = a.size();
                iterator it = a.erase(to_remove);
                REQUIRE(result.first == it->first);
                REQUIRE(result.second == it->second);
                REQUIRE(a.size() == sz - 1ul);

                REQUIRE(ctr == 5);
                REQUIRE(cpy == 20);
                REQUIRE(cpy_assign == 0);
                //REQUIRE(mv == 0);
                //REQUIRE(mv_assign == 0);
                //REQUIRE(dtr == 1);
            }
            REQUIRE(ctr == 5);
            REQUIRE(cpy == 20);
            REQUIRE(cpy_assign == 0);
            //REQUIRE(mv == 0);
            //REQUIRE(mv_assign == 0);
            //REQUIRE(dtr == 21);
        }
    }

    SECTION("iterator erase(const_iterator first, const_iterator last)") {
        for (int test = 0; test < 1000; ++test) {
            std::pair<int, nat> data[20];
            for (int i = 0; i < 20; ++i) {
                data[i].first = i;
                data[i].second = nat{::get_random(-1000000ll, 1000000ll)};
            }
            for (auto i = 20 - 1; i > 0; --i) {
                std::swap(data[i], data[::get_random(0, i)]);
            }
            reset_static_nat_counter();
            {
                TestType a{20 >> 2};
                for (auto& p : data) {
                    a.insert(p);
                }
                using iterator = typename TestType::iterator;
                std::size_t remove_first = ::get_random(0ul, 18ul);
                iterator to_remove_beg = a.begin();
                for (std::size_t i = 1; i < remove_first; ++i) {
                    ++to_remove_beg;
                }
                std::size_t remove_count = ::get_random(1ul, 19ul - static_cast<unsigned long>(remove_first));
                iterator to_remove_last = to_remove_beg;
                for (std::size_t i = 0; i < remove_count; ++i) {
                    ++to_remove_last;
                }
                auto result = *to_remove_last;
                size_t const sz = a.size();
                iterator it = a.erase(to_remove_beg, to_remove_last);
                REQUIRE(result.first == it->first);
                REQUIRE(result.second == it->second);
                REQUIRE(a.size() == sz - static_cast<size_t>(remove_count));

                REQUIRE(ctr == 5);
                REQUIRE(cpy == 20);
                REQUIRE(cpy_assign == 0);
                //REQUIRE(mv == 141);
                //REQUIRE(mv_assign == 87);
                //REQUIRE(dtr == remove_count);
            }

            REQUIRE(ctr == 5);
            REQUIRE(cpy == 20);
            REQUIRE(cpy_assign == 0);
            //REQUIRE(mv == 0);
            //REQUIRE(mv_assign == 0);
            //REQUIRE(dtr == 21);
        }
    }

    SECTION("size_type erase(key_type const& key)") {
        reset_static_nat_counter();
        {
            TestType a{{1, nat{1}}, {2, nat{2}}, {3, nat{3}}, {4, nat{4}}, {5, nat{5}}, {6, nat{6}}, {7, nat{7}}, {8, nat{8}}, {9, nat{9}}, {10, nat{10}}};
            int const k5{5};
            auto next = a.erase(k5);
            REQUIRE(a.size() == 9);
            REQUIRE(a.at(4).cnt == 4);
            REQUIRE(next == 1);

            REQUIRE(ctr == 10);
            REQUIRE(cpy == 10);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 24);
            REQUIRE(mv_assign == 19);
            REQUIRE(dtr == 35);

            int const k42{42};
            next = a.erase(k42);
            REQUIRE(a.size() == 9);
            REQUIRE(a.at(1).cnt == 1);
            REQUIRE(a.at(3).cnt == 3);
            REQUIRE(a.at(4).cnt == 4);
            REQUIRE(next == 0);

            REQUIRE(ctr == 10);
            REQUIRE(cpy == 10);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 24);
            REQUIRE(mv_assign == 19);
            REQUIRE(dtr == 35);
        }
        REQUIRE(ctr == 10);
        REQUIRE(cpy == 10);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 24);
        REQUIRE(mv_assign == 19);
        REQUIRE(dtr == 44);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("flatmap modifiers", "[flatmap][nat]",
                           (plg::flat::test::flatmap),
                           ((nat, int))) {
    SECTION("void clear()") {
        reset_static_nat_counter();
        {
            TestType a{{nat{10}, 10}, {nat{20}, 20}, {nat{30}, 30}, {nat{40}, 40}, {nat{50}, 50}, {nat{60}, 60}, {nat{70}, 70}, {nat{80}, 80}};
            a.clear();
            REQUIRE(a.size() == 0);

            REQUIRE(ctr == 8);
            REQUIRE(cpy == 8);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 15);
            REQUIRE(mv_assign == 7);
            REQUIRE(dtr == 31);
        }
        REQUIRE(ctr == 8);
        REQUIRE(cpy == 8);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 15);
        REQUIRE(mv_assign == 7);
        REQUIRE(dtr == 31);
    }

    SECTION("pair<iterator, bool> insert(value_type const& value)") {
        reset_static_nat_counter();
        {
            TestType a;
            std::pair<nat, int> p1{nat{1}, 1};
            auto it = a.insert(p1);
            REQUIRE(it.second);
            REQUIRE(it.first->first.cnt == 1);
            REQUIRE(it.first->second == 1);

            REQUIRE(ctr == 1);
            REQUIRE(cpy == 1);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 1);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 1);

            std::pair<nat, int> const p2{nat{2}, 2};
            auto jt = a.insert(p2);
            REQUIRE(jt.second);
            REQUIRE(jt.first->first.cnt == 2);
            REQUIRE(jt.first->second == 2);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 2);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 3);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 3);
        }
        REQUIRE(ctr == 2);
        REQUIRE(cpy == 2);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 3);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 7);

        reset_static_nat_counter();
        {
            TestType a;
            std::pair<nat, int> p1{nat{1}, 1};
            auto it = a.insert(p1);
            REQUIRE(it.second);
            REQUIRE(it.first->first.cnt == 1);
            REQUIRE(it.first->second == 1);
            std::pair<nat, int> p2{nat{1}, 2};
            auto jt = a.insert(p2);
            REQUIRE_FALSE(jt.second);
            REQUIRE(jt.first->first.cnt == 1);
            REQUIRE(jt.first->second == 1);

            REQUIRE(ctr == 2);
            REQUIRE(cpy == 1);
            REQUIRE(cpy_assign == 0);
            REQUIRE(mv == 2);
            REQUIRE(mv_assign == 0);
            REQUIRE(dtr == 2);
        }
        REQUIRE(ctr == 2);
        REQUIRE(cpy == 1);
        REQUIRE(cpy_assign == 0);
        REQUIRE(mv == 2);
        REQUIRE(mv_assign == 0);
        REQUIRE(dtr == 5);

        {
            std::pair<nat, int> data[1000];
            for (signed long long i = 0ll; i < 1000ll; ++i) {
                data[i].first = nat{i};
                data[i].second = ::get_random(-1000000, 1000000);
            }
            for (auto i = 1000 - 1; i > 0; --i) {
                std::swap(data[i], data[::get_random(0, i)]);
            }
            reset_static_nat_counter();
            TestType a{};
            for (const auto& p : data) {
                auto it = a.insert(p);
                REQUIRE(it.first != a.end());
                REQUIRE(p.second == it.first->second);
                REQUIRE(p.first == it.first->first);
            }
            REQUIRE(ctr == 0);
            REQUIRE(cpy == 1000);
            REQUIRE(cpy_assign == 0);
            //REQUIRE(mv == 0);
            //REQUIRE(mv_assign == 0);
            //REQUIRE(dtr == 2000);

            for (auto& p : data) {
                auto it = a.find(p.first);
                REQUIRE(it != a.end());
                REQUIRE(p.second == it->second);
                REQUIRE(p.first == it->first);
            }
        }
        REQUIRE(ctr == 0);
        REQUIRE(cpy == 1000);
        REQUIRE(cpy_assign == 0);
        //REQUIRE(mv == 0);
        //REQUIRE(mv_assign == 0);
        //REQUIRE(dtr == 2000);
    }

    
}
